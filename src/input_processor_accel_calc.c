// input_processor_accel_calc.c - Level-specific calculation functions
// Separated for better code organization and maintainability
//
// Copyright (c) 2024 The ZMK Contributors
// Modifications (c) 2025 NUOVOTAKA
// SPDX-License-Identifier: MIT

#include <zephyr/logging/log.h>
#include <zephyr/input/input.h>
#include <stdlib.h>
#include "../include/drivers/input_processor_accel.h"

LOG_MODULE_DECLARE(input_processor_accel);

// =============================================================================
// LEVEL-SPECIFIC CALCULATION FUNCTIONS
// =============================================================================

// Simple acceleration: Just apply sensitivity and basic curve with safety
int32_t accel_simple_calculate(const struct accel_config *cfg, int32_t input_value, uint16_t code) {
#if !defined(CONFIG_INPUT_PROCESSOR_ACCEL_LEVEL_SIMPLE)
    // If Simple level is not enabled, return minimal processing
    // Remove division to preserve precision
    int64_t result = (int64_t)input_value * 1200;
    if (result > 1000) result = result / 1000;
    return (int32_t)ACCEL_CLAMP(result, INT16_MIN, INT16_MAX);
#else
    // Calculate DPI-adjusted sensitivity
    uint32_t dpi_adjusted_sensitivity = cfg->sensor_dpi > 0 ? 
        (cfg->sensitivity * 800) / cfg->sensor_dpi : cfg->sensitivity;
    dpi_adjusted_sensitivity = ACCEL_CLAMP(dpi_adjusted_sensitivity, MIN_SAFE_SENSITIVITY, MAX_SAFE_SENSITIVITY);
    
    // Basic calculation with proper scaling
    int64_t result = (int64_t)input_value * (int64_t)dpi_adjusted_sensitivity;
    
    // Always normalize to prevent overflow (but preserve precision for small values)
    if (dpi_adjusted_sensitivity >= 1000) {
        result = result / 1000;
    } else {
        // For sensitivity < 1000, scale down less aggressively
        result = result / 100;
    }
    
    // Early overflow check
    if (result > INT32_MAX) result = INT32_MAX;
    if (result < INT32_MIN) result = INT32_MIN;
    
    // Apply simple curve based on input magnitude
    int32_t abs_input = abs(input_value);
    if (abs_input > 1 && abs_input <= MAX_SAFE_INPUT_VALUE) {
        uint32_t curve_factor = 1000;
        
        switch (cfg->curve_type) {
            case 0: // Linear - safe multiplication
                {
                    uint32_t linear_add = (uint32_t)abs_input * 100;
                    if (linear_add > cfg->max_factor - 1000) {
                        linear_add = cfg->max_factor - 1000;
                    }
                    curve_factor = 1000 + linear_add;
                }
                break;
            case 1: // Mild - safe quadratic
                curve_factor = accel_safe_quadratic_curve(abs_input, 10);
                break;
            case 2: // Strong - safe quadratic
                curve_factor = accel_safe_quadratic_curve(abs_input, 20);
                break;
            default:
                curve_factor = 1000 + ACCEL_CLAMP((uint32_t)abs_input * 100, 0, cfg->max_factor - 1000);
                break;
        }
        
        curve_factor = ACCEL_CLAMP(curve_factor, 1000, cfg->max_factor);
        
        // Apply curve (divide only when necessary)
        if (curve_factor > 1000) {
            int64_t temp_result = (result * (int64_t)curve_factor) / 1000LL;
            if (temp_result > INT32_MAX) temp_result = INT32_MAX;
            if (temp_result < INT32_MIN) temp_result = INT32_MIN;
            result = temp_result;
        }
    }
    
    // Minimum movement guarantee
    if (input_value != 0 && result == 0) {
        result = (input_value > 0) ? 1 : -1;
    }
    
    // Clamp result to safe range
    return (int32_t)ACCEL_CLAMP(result, INT16_MIN, INT16_MAX);
#endif
}

// Standard acceleration: Speed-based with Y-axis boost and enhanced timing
int32_t accel_standard_calculate(const struct accel_config *cfg, struct accel_data *data, 
                                int32_t input_value, uint16_t code) {
#if !defined(CONFIG_INPUT_PROCESSOR_ACCEL_LEVEL_STANDARD)
    // If Standard level is not enabled, fallback to simple calculation
    LOG_DBG("* Accel processing: Level2 disabled, fallback to Level1");
    return accel_simple_calculate(cfg, input_value, code);
#else
    uint32_t speed = accel_calculate_speed(data, input_value);
    
    // Calculate DPI-adjusted sensitivity
    uint32_t dpi_adjusted_sensitivity = cfg->sensor_dpi > 0 ? 
        (cfg->sensitivity * 800) / cfg->sensor_dpi : cfg->sensitivity;
    dpi_adjusted_sensitivity = ACCEL_CLAMP(dpi_adjusted_sensitivity, MIN_SAFE_SENSITIVITY, MAX_SAFE_SENSITIVITY);
    
    // Basic calculation with proper scaling
    int64_t result = (int64_t)input_value * dpi_adjusted_sensitivity;
    
    // Always apply base scaling to prevent overflow
    result = result / 100;  // Scale down by 100 instead of 1000 to preserve precision
    
    // Speed-based acceleration
    uint32_t factor = cfg->min_factor;
    if (speed > cfg->speed_threshold) {
        if (speed >= cfg->speed_max) {
            factor = cfg->max_factor;
        } else {
            uint32_t speed_range = cfg->speed_max - cfg->speed_threshold;
            if (speed_range == 0) {
                factor = cfg->max_factor; // Fallback if range is zero
            } else {
                uint32_t speed_offset = speed - cfg->speed_threshold;
                uint32_t t = (speed_offset * 1000) / speed_range;
            
                // Exponential curve calculation (simplified)
                uint32_t curve;
                t = (t > 1000) ? 1000 : t; // Clamp input to prevent overflow
                
                switch (cfg->acceleration_exponent) {
                    case 1: // Linear
                        curve = t;
                        break;
                    case 2: // Mild exponential - safe calculation
                        {
                            uint64_t t_squared = (uint64_t)t * t;
                            curve = t + (uint32_t)(t_squared / 2000ULL);
                        }
                        break;
                    case 3: // Moderate exponential - safe calculation
                        {
                            uint64_t t_squared = (uint64_t)t * t;
                            uint64_t t_cubed = t_squared * t;
                            curve = t + (uint32_t)(t_squared / 1000ULL) + (uint32_t)(t_cubed / 3000000ULL);
                        }
                        break;
                    case 4: // Strong exponential - safe calculation
                        {
                            uint64_t t_squared = (uint64_t)t * t;
                            uint64_t t_cubed = t_squared * t;
                            curve = t + (uint32_t)(t_squared / 800ULL) + (uint32_t)(t_cubed / 2000000ULL);
                        }
                        break;
                    case 5: // Aggressive exponential - safe calculation
                        {
                            uint64_t t_squared = (uint64_t)t * t;
                            uint64_t t_cubed = t_squared * t;
                            curve = t + (uint32_t)(t_squared / 600ULL) + (uint32_t)(t_cubed / 1500000ULL);
                        }
                        break;
                    default:
                        {
                            uint64_t t_squared = (uint64_t)t * t;
                            curve = (uint32_t)(t_squared / 1000ULL); // Fallback quadratic
                        }
                        break;
                }
                
                // Final clamp to prevent overflow
                curve = (curve > 1000) ? 1000 : curve;
            
                curve = ACCEL_CLAMP(curve, 0, 1000);
                factor = cfg->min_factor + (((cfg->max_factor - cfg->min_factor) * curve) / 1000);
            }
        }
        
        factor = ACCEL_CLAMP(factor, cfg->min_factor, cfg->max_factor);
        
        // Apply acceleration with proper scaling
        int64_t temp_result = (result * (int64_t)factor) / 1000LL;
        if (temp_result > INT32_MAX) temp_result = INT32_MAX;
        if (temp_result < INT32_MIN) temp_result = INT32_MIN;
        result = temp_result;
    }
    
    // Y-axis boost with proper scaling
    if (code == 0x01) {
        int64_t temp_result = (result * (int64_t)cfg->y_boost) / 1000LL;
        if (temp_result > INT32_MAX) temp_result = INT32_MAX;
        if (temp_result < INT32_MIN) temp_result = INT32_MIN;
        result = temp_result;
    }
    
    // Calculate final accelerated value
    int32_t accelerated_value = (int32_t)result;
    
    // Optional remainder processing (can be disabled for MCU efficiency)
#ifdef CONFIG_INPUT_PROCESSOR_ACCEL_TRACK_REMAINDERS
    if (cfg->track_remainders && (code == INPUT_REL_X || code == INPUT_REL_Y)) {
        int32_t remainder = (int32_t)(result % 1000LL);
        
        // Select appropriate remainder storage
        atomic_t *remainder_ptr = (code == INPUT_REL_X) ? &data->remainder_x : &data->remainder_y;
        
        int32_t current_remainder = atomic_get(remainder_ptr);
        int32_t new_remainder = current_remainder + remainder;
        
        if (abs(new_remainder) >= 1000) {
            int32_t carry = new_remainder / 1000;
            
            // Overflow-safe addition
            int64_t temp_value = (int64_t)accelerated_value + carry;
            if (temp_value > INT16_MAX) temp_value = INT16_MAX;
            if (temp_value < INT16_MIN) temp_value = INT16_MIN;
            accelerated_value = (int32_t)temp_value;
            
            new_remainder = new_remainder % 1000;
        }
        
        atomic_set(remainder_ptr, new_remainder);
    }
#endif
    
    // Minimum movement guarantee
    if (input_value != 0 && accelerated_value == 0) {
        accelerated_value = (input_value > 0) ? 1 : -1;
    }
    
    // Clamp result to safe range
    return (int32_t)ACCEL_CLAMP(accelerated_value, INT16_MIN, INT16_MAX);
#endif
}


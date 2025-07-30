// input_processor_accel_calc.c - Level-specific calculation functions
// Separated for better code organization and maintainability
//
// Copyright (c) 2024 The ZMK Contributors
// Modifications (c) 2025 NUOVOTAKA
// SPDX-License-Identifier: MIT

#include <zephyr/logging/log.h>
#include <zephyr/input/input.h>
#include <stdlib.h>
#include <drivers/input_processor_accel.h>

LOG_MODULE_DECLARE(input_processor_accel_main);

// =============================================================================
// LEVEL-SPECIFIC CALCULATION FUNCTIONS
// =============================================================================

// Simple acceleration: Just apply sensitivity and basic curve with safety
int32_t accel_simple_calculate(const struct accel_config *cfg, int32_t input_value, uint16_t code) {
#if !defined(CONFIG_INPUT_PROCESSOR_ACCEL_LEVEL_SIMPLE)
    // If Simple level is not enabled, return minimal processing
    return (int32_t)ACCEL_CLAMP(((int64_t)input_value * 1200) / 1000, INT16_MIN, INT16_MAX);
#else
    // Apply DPI-adjusted sensitivity with overflow protection
    // Standard reference DPI is 800, adjust sensitivity based on actual sensor DPI
    uint32_t dpi_adjusted_sensitivity = cfg->sensor_dpi > 0 ? 
        (cfg->sensitivity * 800) / cfg->sensor_dpi : cfg->sensitivity;
    dpi_adjusted_sensitivity = ACCEL_CLAMP(dpi_adjusted_sensitivity, MIN_SAFE_SENSITIVITY, MAX_SAFE_SENSITIVITY);
    
    int64_t result = ((int64_t)input_value * dpi_adjusted_sensitivity) / 1000;
    
    // Apply simple curve based on input magnitude
    int32_t abs_input = abs(input_value);
    if (abs_input > 1) {
        uint32_t curve_factor = 1000;
        
        switch (cfg->curve_type) {
            case 0: // Linear
                curve_factor = 1000 + ACCEL_CLAMP(abs_input * 100, 0, cfg->max_factor - 1000);
                break;
            case 1: // Mild
                curve_factor = accel_safe_quadratic_curve(abs_input, 10);
                break;
            case 2: // Strong
                curve_factor = accel_safe_quadratic_curve(abs_input, 20);
                break;
            default:
                LOG_WRN("Invalid curve_type: %u, using linear", cfg->curve_type);
                curve_factor = 1000 + ACCEL_CLAMP(abs_input * 100, 0, cfg->max_factor - 1000);
                break;
        }
        
        curve_factor = ACCEL_CLAMP(curve_factor, 1000, cfg->max_factor);
        result = (result * curve_factor) / 1000;
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
    return accel_simple_calculate(cfg, input_value, code);
#else
    // Use enhanced speed calculation
    uint32_t speed = accel_calculate_enhanced_speed(&data->timing, input_value);
    
    // Apply DPI-adjusted sensitivity with overflow protection
    // Standard reference DPI is 800, adjust sensitivity based on actual sensor DPI
    uint32_t dpi_adjusted_sensitivity = cfg->sensor_dpi > 0 ? 
        (cfg->sensitivity * 800) / cfg->sensor_dpi : cfg->sensitivity;
    dpi_adjusted_sensitivity = ACCEL_CLAMP(dpi_adjusted_sensitivity, MIN_SAFE_SENSITIVITY, MAX_SAFE_SENSITIVITY);
    
    int64_t result = ((int64_t)input_value * dpi_adjusted_sensitivity) / 1000;
    
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
            
                // Efficient exponential curve approximation for MCU
                uint32_t curve;
                t = (t > 1000) ? 1000 : t; // Clamp to prevent overflow
                
                switch (cfg->acceleration_exponent) {
                    case 1: // Linear
                        curve = t;
                        break;
                    case 2: // Mild exponential (e^(2t) - 1 approximation)
                        curve = t + (t * t) / 2000;
                        break;
                    case 3: // Moderate exponential
                        curve = t + (t * t) / 1000 + (t * t * t) / 3000000;
                        break;
                    case 4: // Strong exponential
                        curve = t + (t * t) / 800 + (t * t * t) / 2000000;
                        break;
                    case 5: // Aggressive exponential
                        curve = t + (t * t) / 600 + (t * t * t) / 1500000;
                        break;
                    default:
                        curve = (t * t) / 1000; // Fallback quadratic
                        break;
                }
                
                curve = (curve > 1000) ? 1000 : curve;
            
                curve = ACCEL_CLAMP(curve, 0, 1000);
                factor = cfg->min_factor + (((cfg->max_factor - cfg->min_factor) * curve) / 1000);
            }
        }
        
        factor = ACCEL_CLAMP(factor, cfg->min_factor, cfg->max_factor);
        result = (result * factor) / 1000;
    }
    
    // Y-axis boost with overflow protection (code 0x01 is Y-axis)
    if (code == 0x01) {
        result = (result * cfg->y_boost) / 1000;
    }
    
    // Calculate final accelerated value
    int32_t accelerated_value = (int32_t)(result / 1000LL);
    
    // Optional remainder processing (can be disabled for MCU efficiency)
#ifdef CONFIG_INPUT_PROCESSOR_ACCEL_TRACK_REMAINDERS
    if (cfg->track_remainders && (code == INPUT_REL_X || code == INPUT_REL_Y)) {
        uint8_t remainder_idx = (code == INPUT_REL_X) ? 0 : 1;
        int32_t remainder = (int32_t)(result % 1000LL);
        
        int32_t current_remainder = atomic_get(&data->remainders[remainder_idx]);
        int32_t new_remainder = current_remainder + remainder;
        
        if (abs(new_remainder) >= 1000) {
            int32_t carry = new_remainder / 1000;
            accelerated_value += carry;
            new_remainder = new_remainder % 1000;
        }
        
        atomic_set(&data->remainders[remainder_idx], new_remainder);
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


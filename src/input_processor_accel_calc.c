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
// OVERFLOW-SAFE HELPER FUNCTIONS
// =============================================================================

static inline int64_t safe_multiply_64(int64_t a, int64_t b, int64_t max_result) {
    if (a == 0 || b == 0) return 0;
    
    if (a > 0 && b > 0) {
        if (a > max_result / b) return max_result;
    } else if (a < 0 && b < 0) {
        if (a < max_result / b) return max_result;
    } else if (a < 0 && b > 0) {
        if (a < (-max_result) / b) return -max_result;
    } else if (a > 0 && b < 0) {
        if (b < (-max_result) / a) return -max_result;
    }
    
    return a * b;
}

static inline int32_t safe_int64_to_int32(int64_t value) {
    if (value > INT32_MAX) return INT32_MAX;
    if (value < INT32_MIN) return INT32_MIN;
    return (int32_t)value;
}

static inline int16_t safe_int32_to_int16(int32_t value) {
    if (value > INT16_MAX) return INT16_MAX;
    if (value < INT16_MIN) return INT16_MIN;
    return (int16_t)value;
}

static uint32_t calculate_dpi_adjusted_sensitivity(const struct accel_config *cfg) {
    uint32_t dpi_adjusted_sensitivity;
    
    if (cfg->sensor_dpi > 0 && cfg->sensor_dpi <= MAX_SENSOR_DPI) {
        uint64_t temp = (uint64_t)cfg->sensitivity * STANDARD_DPI_REFERENCE;
        uint64_t calculated = temp / cfg->sensor_dpi;
        
        if (calculated > MAX_SAFE_SENSITIVITY) {
            dpi_adjusted_sensitivity = MAX_SAFE_SENSITIVITY;
        } else {
            dpi_adjusted_sensitivity = (uint32_t)calculated;
        }
    } else {
        dpi_adjusted_sensitivity = cfg->sensitivity;
    }
    
    dpi_adjusted_sensitivity = ACCEL_CLAMP(dpi_adjusted_sensitivity, MIN_SAFE_SENSITIVITY, MAX_SAFE_SENSITIVITY);
    return dpi_adjusted_sensitivity;
}

static uint32_t calculate_exponential_curve(uint32_t t, uint8_t exponent) {
    switch (exponent) {
        case 1: // Linear
            return t;
            
        case 2: // Mild exponential
            {
                uint64_t t_sq = (uint64_t)t * t;
                uint32_t quad = (t_sq > CURVE_MILD_DIVISOR * UINT32_MAX) ? 
                    UINT32_MAX : (uint32_t)(t_sq / CURVE_MILD_DIVISOR);
                uint32_t result = (t > UINT32_MAX - quad) ? UINT32_MAX : t + quad;
                return result;
            }
            
        case 3: // Moderate exponential
            {
                uint64_t t_sq = (uint64_t)t * t;
                uint32_t quad = (t_sq > CURVE_MODERATE_QUAD_DIV * UINT32_MAX) ? 
                    UINT32_MAX : (uint32_t)(t_sq / CURVE_MODERATE_QUAD_DIV);
                uint64_t t_cb = (t_sq > UINT64_MAX / t) ? UINT64_MAX : t_sq * t;
                uint32_t cubic = (t_cb > CURVE_MODERATE_CUBIC_DIV * UINT32_MAX) ? 
                    UINT32_MAX : (uint32_t)(t_cb / CURVE_MODERATE_CUBIC_DIV);
                uint64_t temp_result = (uint64_t)t + quad + cubic;
                uint32_t result = (temp_result > UINT32_MAX) ? UINT32_MAX : (uint32_t)temp_result;
                return result;
            }
            
        case 4: // Strong exponential
            {
                uint64_t t_sq = (uint64_t)t * t;
                uint32_t quad = (t_sq > CURVE_STRONG_QUAD_DIV * UINT32_MAX) ? 
                    UINT32_MAX : (uint32_t)(t_sq / CURVE_STRONG_QUAD_DIV);
                uint64_t t_cb = (t_sq > UINT64_MAX / t) ? UINT64_MAX : t_sq * t;
                uint32_t cubic = (t_cb > CURVE_STRONG_CUBIC_DIV * UINT32_MAX) ? 
                    UINT32_MAX : (uint32_t)(t_cb / CURVE_STRONG_CUBIC_DIV);
                uint64_t temp_result = (uint64_t)t + quad + cubic;
                uint32_t result = (temp_result > UINT32_MAX) ? UINT32_MAX : (uint32_t)temp_result;
                return result;
            }
            
        case 5: // Aggressive exponential
            {
                uint64_t t_sq = (uint64_t)t * t;
                uint32_t quad = (t_sq > CURVE_AGGRESSIVE_QUAD_DIV * UINT32_MAX) ? 
                    UINT32_MAX : (uint32_t)(t_sq / CURVE_AGGRESSIVE_QUAD_DIV);
                uint64_t t_cb = (t_sq > UINT64_MAX / t) ? UINT64_MAX : t_sq * t;
                uint32_t cubic = (t_cb > CURVE_AGGRESSIVE_CUBIC_DIV * UINT32_MAX) ? 
                    UINT32_MAX : (uint32_t)(t_cb / CURVE_AGGRESSIVE_CUBIC_DIV);
                uint64_t temp_result = (uint64_t)t + quad + cubic;
                uint32_t result = (temp_result > UINT32_MAX) ? UINT32_MAX : (uint32_t)temp_result;
                return result;
            }
            
        default: // Fallback quadratic
            {
                uint64_t t_sq = (uint64_t)t * t;
                uint32_t result = (t_sq > CURVE_DEFAULT_DIVISOR * UINT32_MAX) ? 
                    UINT32_MAX : (uint32_t)(t_sq / CURVE_DEFAULT_DIVISOR);
                return result;
            }
    }
}

// =============================================================================
// LEVEL-SPECIFIC CALCULATION FUNCTIONS
// =============================================================================

int32_t accel_simple_calculate(const struct accel_config *cfg, int32_t input_value, uint16_t code) {
    if (!cfg) {
        LOG_ERR("Configuration pointer is NULL in simple calculation");
        return input_value;
    }

#if !defined(CONFIG_INPUT_PROCESSOR_ACCEL_LEVEL_SIMPLE)
    LOG_DBG("Simple level not enabled, using fallback calculation for input: %d", input_value);
    
    // Improved fallback calculation with proper scaling
    int64_t result = (int64_t)input_value;
    
    // Apply basic sensitivity (1.0x by default)
    result = safe_multiply_64(result, 1000LL, INT32_MAX);
    
    // Apply mild acceleration for larger movements
    int32_t abs_input = abs(input_value);
    if (abs_input > 5) {
        uint32_t accel_factor = 1000 + (abs_input * 5); // Mild acceleration
        accel_factor = ACCEL_CLAMP(accel_factor, 1000, 2000); // Max 2x
        result = safe_multiply_64(result, (int64_t)accel_factor, INT32_MAX * 1000LL);
        result = result / 1000; // Scale back
    }
    
    // Final scaling
    result = result / 1000;
    
    LOG_DBG("Fallback calculation: %d -> sensitivity=%lld -> final=%lld", 
            input_value, (int64_t)input_value * 1000, result);
    
    int32_t safe_result = safe_int64_to_int32(result);
    return safe_int32_to_int16(safe_result);
#else
    if (abs(input_value) > MAX_SAFE_INPUT_VALUE) {
        LOG_WRN("Input value %d exceeds safe limit, clamping", input_value);
        input_value = (input_value > 0) ? MAX_SAFE_INPUT_VALUE : -MAX_SAFE_INPUT_VALUE;
    }
    
    uint32_t dpi_adjusted_sensitivity = calculate_dpi_adjusted_sensitivity(cfg);
    int64_t result = safe_multiply_64((int64_t)input_value, (int64_t)dpi_adjusted_sensitivity, 
                                     (int64_t)INT32_MAX * SENSITIVITY_SCALE);
    
    LOG_DBG("Level1 Debug: Input=%d, Sensitivity=%u, Raw result=%lld", 
            input_value, dpi_adjusted_sensitivity, result);
    
    if (abs(result) >= SENSITIVITY_SCALE) {
        result = result / SENSITIVITY_SCALE;
        LOG_DBG("Level1 Debug: After sensitivity scaling: %lld", result);
    } else {
        LOG_DBG("Level1 Debug: No sensitivity scaling needed (result=%lld < %d)", result, SENSITIVITY_SCALE);
    }
    
    int32_t abs_input = abs(input_value);
    if (abs_input > 1 && abs_input <= MAX_SAFE_INPUT_VALUE) {
        uint32_t curve_factor = 1000;
        
        switch (cfg->curve_type) {
            case 0: // Linear
                {
                    uint64_t linear_add = (uint64_t)abs_input * LINEAR_CURVE_MULTIPLIER;
                    uint32_t max_add = (cfg->max_factor > SENSITIVITY_SCALE) ? 
                        cfg->max_factor - SENSITIVITY_SCALE : 0;
                    if (linear_add > max_add) {
                        linear_add = max_add;
                    }
                    curve_factor = SENSITIVITY_SCALE + (uint32_t)linear_add;
                }
                break;
            case 1: // Mild
                curve_factor = accel_safe_quadratic_curve(abs_input, 10);
                break;
            case 2: // Strong
                curve_factor = accel_safe_quadratic_curve(abs_input, 20);
                break;
            default:
                {
                    uint64_t default_add = (uint64_t)abs_input * LINEAR_CURVE_MULTIPLIER;
                    uint32_t max_add = (cfg->max_factor > SENSITIVITY_SCALE) ? 
                        cfg->max_factor - SENSITIVITY_SCALE : 0;
                    curve_factor = SENSITIVITY_SCALE + ACCEL_CLAMP((uint32_t)default_add, 0, max_add);
                }
                break;
        }
        
        curve_factor = ACCEL_CLAMP(curve_factor, SENSITIVITY_SCALE, cfg->max_factor);
        
        LOG_DBG("Level1 Debug: abs_input=%d, curve_type=%u, curve_factor=%u, max_factor=%u", 
                abs_input, cfg->curve_type, curve_factor, cfg->max_factor);
        
        if (curve_factor > SENSITIVITY_SCALE) {
            int64_t temp_result = safe_multiply_64(result, (int64_t)curve_factor, (int64_t)INT32_MAX * SENSITIVITY_SCALE);
            result = temp_result / SENSITIVITY_SCALE;
            LOG_DBG("Level1 Debug: After acceleration: temp=%lld, final=%lld", temp_result, result);
        }
    }
    
    if (input_value != 0 && result == 0) {
        result = (input_value > 0) ? 1 : -1;
        LOG_DBG("Level1 Debug: Zero result forced to: %lld", result);
    }
    
    int32_t safe_result = safe_int64_to_int32(result);
    int16_t final_result = safe_int32_to_int16(safe_result);
    
    LOG_DBG("Level1 Debug: Final conversion: %lld -> %d -> %d (input was %d)", 
            result, safe_result, final_result, input_value);
    
    return final_result;
#endif
}

int32_t accel_standard_calculate(const struct accel_config *cfg, struct accel_data *data, 
                                int32_t input_value, uint16_t code) {
    if (!cfg) {
        LOG_ERR("Configuration pointer is NULL in standard calculation");
        return input_value;
    }
    if (!data) {
        LOG_ERR("Data pointer is NULL in standard calculation");
        return input_value;
    }

#if !defined(CONFIG_INPUT_PROCESSOR_ACCEL_LEVEL_STANDARD)
    LOG_DBG("Standard level not enabled, fallback to simple calculation");
    return accel_simple_calculate(cfg, input_value, code);
#else
    if (abs(input_value) > MAX_SAFE_INPUT_VALUE) {
        LOG_WRN("Input value %d exceeds safe limit, clamping", input_value);
        input_value = (input_value > 0) ? MAX_SAFE_INPUT_VALUE : -MAX_SAFE_INPUT_VALUE;
    }
    
    uint32_t speed = accel_calculate_speed(data, input_value);
    
    uint32_t dpi_adjusted_sensitivity = calculate_dpi_adjusted_sensitivity(cfg);
    
    int64_t result = safe_multiply_64((int64_t)input_value, (int64_t)dpi_adjusted_sensitivity, 
                                     (int64_t)INT32_MAX * SENSITIVITY_SCALE);
    
    if (abs(result) >= SENSITIVITY_SCALE) {
        result = result / SENSITIVITY_SCALE;
    }
    
    // Speed-based acceleration with overflow protection
    uint32_t factor = cfg->min_factor;
    if (speed > cfg->speed_threshold && cfg->speed_max > cfg->speed_threshold) {
        if (speed >= cfg->speed_max) {
            factor = cfg->max_factor;
        } else {
            uint32_t speed_range = cfg->speed_max - cfg->speed_threshold;
            uint32_t speed_offset = speed - cfg->speed_threshold;
            
            // Safe calculation of normalized speed (0-1000)
            uint64_t t_temp = ((uint64_t)speed_offset * SPEED_NORMALIZATION) / speed_range;
            uint32_t t = (t_temp > SPEED_NORMALIZATION) ? SPEED_NORMALIZATION : (uint32_t)t_temp;
            
            uint32_t curve = calculate_exponential_curve(t, cfg->acceleration_exponent);
            
            // Final clamp and factor calculation
            curve = ACCEL_CLAMP(curve, 0, SPEED_NORMALIZATION);
            
            // Safe factor calculation with overflow protection
            if (cfg->max_factor >= cfg->min_factor) {
                uint64_t factor_range = cfg->max_factor - cfg->min_factor;
                uint64_t factor_add = (factor_range * curve) / SPEED_NORMALIZATION;
                factor = cfg->min_factor + (uint32_t)ACCEL_CLAMP(factor_add, 0, factor_range);
            } else {
                factor = cfg->min_factor;
            }
        }
        
        factor = ACCEL_CLAMP(factor, cfg->min_factor, cfg->max_factor);
        
        // Apply acceleration with overflow protection
        if (factor > SENSITIVITY_SCALE) {
            int64_t temp_result = safe_multiply_64(result, (int64_t)factor, (int64_t)INT32_MAX * SENSITIVITY_SCALE);
            result = temp_result / SENSITIVITY_SCALE;
        }
    }
    
    // Y-axis boost with overflow protection
    if (code == INPUT_REL_Y && cfg->y_boost != SENSITIVITY_SCALE) {
        int64_t temp_result = safe_multiply_64(result, (int64_t)cfg->y_boost, (int64_t)INT32_MAX * SENSITIVITY_SCALE);
        result = temp_result / SENSITIVITY_SCALE;
    }
    
    // Calculate final accelerated value with safe conversion
    int32_t accelerated_value = safe_int64_to_int32(result);
    
    // Optional remainder processing with overflow protection
#ifdef CONFIG_INPUT_PROCESSOR_ACCEL_TRACK_REMAINDERS
    if (cfg->track_remainders && (code == INPUT_REL_X || code == INPUT_REL_Y)) {
        // Calculate remainder safely
        int64_t full_result = safe_multiply_64(result, SENSITIVITY_SCALE, INT64_MAX);
        int32_t remainder = (int32_t)(full_result % SENSITIVITY_SCALE);
        
        // Select appropriate remainder storage (single-threaded optimization)
        int32_t *remainder_ptr = (code == INPUT_REL_X) ? &data->remainder_x : &data->remainder_y;
        
        int32_t current_remainder = *remainder_ptr;
        
        // Safe remainder addition with overflow check
        int64_t new_remainder_64 = (int64_t)current_remainder + remainder;
        int32_t new_remainder = safe_int64_to_int32(new_remainder_64);
        
        if (abs(new_remainder) >= SENSITIVITY_SCALE) {
            int32_t carry = new_remainder / SENSITIVITY_SCALE;
            
            // Overflow-safe addition
            int64_t temp_value = (int64_t)accelerated_value + carry;
            accelerated_value = safe_int64_to_int32(temp_value);
            
            new_remainder = new_remainder % SENSITIVITY_SCALE;
        }
        
        // Direct assignment (no atomic operation needed in single-threaded context)
        *remainder_ptr = new_remainder;
    }
#endif
    
    // Minimum movement guarantee
    if (input_value != 0 && accelerated_value == 0) {
        accelerated_value = (input_value > 0) ? 1 : -1;
    }
    
    // Final safe conversion to output range
    return safe_int32_to_int16(accelerated_value);
#endif
}


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

#if defined(CONFIG_INPUT_PROCESSOR_ACCEL_LEVEL_STANDARD)
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
#endif // CONFIG_INPUT_PROCESSOR_ACCEL_LEVEL_STANDARD

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
    
    // Enhanced safety: Input value validation
    input_value = accel_clamp_input_value(input_value);
    
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
    // Enhanced safety: Input value validation with detailed logging
    if (abs(input_value) > MAX_SAFE_INPUT_VALUE) {
        LOG_WRN("Level1: Input value %d exceeds safe limit %d, clamping", 
                input_value, MAX_SAFE_INPUT_VALUE);
        input_value = accel_clamp_input_value(input_value);
    }
    
    // Enhanced safety: Configuration validation
    if (cfg->sensitivity == 0 || cfg->sensitivity > MAX_SAFE_SENSITIVITY) {
        LOG_ERR("Level1: Invalid sensitivity %u, using default", cfg->sensitivity);
        return input_value; // Safe fallback
    }
    
    uint32_t dpi_adjusted_sensitivity = calculate_dpi_adjusted_sensitivity(cfg);
    
    // Enhanced safety: Check sensitivity bounds
    if (dpi_adjusted_sensitivity == 0 || dpi_adjusted_sensitivity > MAX_SAFE_SENSITIVITY) {
        LOG_ERR("Level1: Invalid DPI-adjusted sensitivity %u, using passthrough", 
                dpi_adjusted_sensitivity);
        return input_value;
    }
    
    int64_t result = safe_multiply_64((int64_t)input_value, (int64_t)dpi_adjusted_sensitivity, 
                                     (int64_t)INT32_MAX * SENSITIVITY_SCALE);
    
    // Minimal logging for Level 1
    static uint32_t level1_log_counter = 0;
    if ((level1_log_counter++ % 100) == 0) { // Reduced logging frequency
        LOG_DBG("Level1: Input=%d, Sensitivity=%u, Raw result=%lld", 
                input_value, dpi_adjusted_sensitivity, result);
    }
    
    // Enhanced safety: Check intermediate result
    if (abs(result) > (int64_t)INT32_MAX * SENSITIVITY_SCALE / 2) {
        LOG_WRN("Level1: Intermediate result %lld too large, scaling down", result);
        result = result / 2; // Emergency scaling
    }
    
    if (abs(result) >= SENSITIVITY_SCALE) {
        result = result / SENSITIVITY_SCALE;
    }
    
    int32_t abs_input = abs(input_value);
    if (abs_input > 1 && abs_input <= MAX_SAFE_INPUT_VALUE) {
        uint32_t curve_factor = SENSITIVITY_SCALE; // Start with 1.0x
        
        // Enhanced safety: Validate max_factor before use
        uint32_t safe_max_factor = ACCEL_CLAMP(cfg->max_factor, SENSITIVITY_SCALE, MAX_SAFE_FACTOR);
        
        switch (cfg->curve_type) {
            case 0: // Linear - Enhanced safety
                {
                    uint64_t linear_add = safe_multiply_64((int64_t)abs_input, 
                                                         (int64_t)LINEAR_CURVE_MULTIPLIER, 
                                                         (int64_t)safe_max_factor);
                    uint32_t max_add = (safe_max_factor > SENSITIVITY_SCALE) ? 
                        safe_max_factor - SENSITIVITY_SCALE : 0;
                    if (linear_add > max_add) {
                        linear_add = max_add;
                    }
                    curve_factor = SENSITIVITY_SCALE + (uint32_t)linear_add;
                }
                break;
            case 1: // Mild - Safe quadratic approximation
                {
                    uint64_t quad_add = safe_multiply_64((int64_t)abs_input * abs_input, 
                                                       10LL, (int64_t)safe_max_factor);
                    quad_add = quad_add / 100; // Scale down
                    uint32_t max_add = (safe_max_factor > SENSITIVITY_SCALE) ? 
                        safe_max_factor - SENSITIVITY_SCALE : 0;
                    curve_factor = SENSITIVITY_SCALE + ACCEL_CLAMP((uint32_t)quad_add, 0, max_add);
                }
                break;
            case 2: // Strong - Safe quadratic approximation
                {
                    uint64_t quad_add = safe_multiply_64((int64_t)abs_input * abs_input, 
                                                       20LL, (int64_t)safe_max_factor);
                    quad_add = quad_add / 100; // Scale down
                    uint32_t max_add = (safe_max_factor > SENSITIVITY_SCALE) ? 
                        safe_max_factor - SENSITIVITY_SCALE : 0;
                    curve_factor = SENSITIVITY_SCALE + ACCEL_CLAMP((uint32_t)quad_add, 0, max_add);
                }
                break;
            default: // Safe fallback
                {
                    uint64_t default_add = safe_multiply_64((int64_t)abs_input, 
                                                          (int64_t)LINEAR_CURVE_MULTIPLIER, 
                                                          (int64_t)safe_max_factor);
                    uint32_t max_add = (safe_max_factor > SENSITIVITY_SCALE) ? 
                        safe_max_factor - SENSITIVITY_SCALE : 0;
                    curve_factor = SENSITIVITY_SCALE + ACCEL_CLAMP((uint32_t)default_add, 0, max_add);
                }
                break;
        }
        
        // Enhanced safety: Double-check curve factor bounds
        curve_factor = ACCEL_CLAMP(curve_factor, SENSITIVITY_SCALE, safe_max_factor);
        
        if (curve_factor > SENSITIVITY_SCALE) {
            int64_t temp_result = safe_multiply_64(result, (int64_t)curve_factor, 
                                                 (int64_t)INT16_MAX * SENSITIVITY_SCALE);
            result = temp_result / SENSITIVITY_SCALE;
            
            // Enhanced safety: Multiple range checks for Level 1 result
            if (abs(result) > INT16_MAX) {
                LOG_WRN("Level1: Result %lld exceeds int16 range, clamping to %d", 
                        result, INT16_MAX);
                result = (result > 0) ? INT16_MAX : INT16_MIN;
            }
        }
    }
    
    // Enhanced safety: Minimum movement guarantee with bounds check
    if (input_value != 0 && result == 0) {
        result = (input_value > 0) ? 1 : -1;
    }
    
    // Enhanced safety: Final result validation
    int32_t safe_result = safe_int64_to_int32(result);
    int16_t final_result = safe_int32_to_int16(safe_result);
    
    // Enhanced safety: Final bounds check with logging
    if (abs(final_result) > INT16_MAX) {
        LOG_ERR("Level1: Final result %d exceeds int16 bounds, emergency clamp", final_result);
        final_result = (final_result > 0) ? INT16_MAX : INT16_MIN;
    }
    
    // Enhanced safety: Sanity check - if input was reasonable, output should be too
    if (abs(input_value) <= 100 && abs(final_result) > 1000) {
        LOG_WRN("Level1: Suspicious result %d for input %d, using conservative value", 
                final_result, input_value);
        final_result = input_value * 2; // Conservative fallback
        final_result = safe_int32_to_int16(final_result);
    }
    
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
    // Enhanced safety: Input value validation with detailed logging
    if (abs(input_value) > MAX_SAFE_INPUT_VALUE) {
        LOG_WRN("Level2: Input value %d exceeds safe limit %d, clamping", 
                input_value, MAX_SAFE_INPUT_VALUE);
        input_value = accel_clamp_input_value(input_value);
    }
    
    // Enhanced safety: Data structure validation (simplified)
    if (data->recent_speed > UINT16_MAX / 2) {
        LOG_WRN("Level2: Invalid recent_speed %u, resetting data", data->recent_speed);
        data->recent_speed = 0;
        data->last_time_ms = 0;
        data->speed_samples = 0;
    }
    
    uint32_t speed = accel_calculate_simple_speed(data, input_value);
    
    // Enhanced safety: Speed validation
    if (speed > MAX_REASONABLE_SPEED) {
        LOG_ERR("Level2: Calculated speed %u exceeds maximum %u, using fallback", 
                speed, MAX_REASONABLE_SPEED);
        return accel_safe_fallback_calculate(input_value, cfg->max_factor);
    }
    
    uint32_t dpi_adjusted_sensitivity = calculate_dpi_adjusted_sensitivity(cfg);
    
    // Enhanced safety: Sensitivity validation
    if (dpi_adjusted_sensitivity == 0 || dpi_adjusted_sensitivity > MAX_SAFE_SENSITIVITY) {
        LOG_ERR("Level2: Invalid DPI-adjusted sensitivity %u, using fallback", 
                dpi_adjusted_sensitivity);
        return accel_safe_fallback_calculate(input_value, cfg->max_factor);
    }
    
    int64_t result = safe_multiply_64((int64_t)input_value, (int64_t)dpi_adjusted_sensitivity, 
                                     (int64_t)INT32_MAX * SENSITIVITY_SCALE);
    
    // Enhanced safety: Check intermediate result
    if (abs(result) > (int64_t)INT32_MAX * SENSITIVITY_SCALE / 4) {
        LOG_WRN("Level2: Intermediate result %lld too large, using fallback", result);
        return accel_safe_fallback_calculate(input_value, cfg->max_factor);
    }
    
    if (abs(result) >= SENSITIVITY_SCALE) {
        result = result / SENSITIVITY_SCALE;
    }
    
    // Enhanced safety: Speed-based acceleration with comprehensive protection
    uint32_t factor = cfg->min_factor;
    
    // Enhanced safety: Validate speed thresholds
    if (cfg->speed_threshold >= cfg->speed_max) {
        LOG_ERR("Level2: Invalid speed configuration (threshold=%u >= max=%u), using linear", 
                cfg->speed_threshold, cfg->speed_max);
        factor = cfg->min_factor;
    } else if (speed > cfg->speed_threshold && cfg->speed_max > cfg->speed_threshold) {
        if (speed >= cfg->speed_max) {
            factor = cfg->max_factor;
        } else {
            uint32_t speed_range = cfg->speed_max - cfg->speed_threshold;
            uint32_t speed_offset = speed - cfg->speed_threshold;
            
            // Enhanced safety: Validate speed range
            if (speed_range == 0) {
                LOG_ERR("Level2: Zero speed range, using min factor");
                factor = cfg->min_factor;
            } else {
                // Safe calculation of normalized speed (0-1000)
                uint64_t t_temp = ((uint64_t)speed_offset * SPEED_NORMALIZATION) / speed_range;
                uint32_t t = (t_temp > SPEED_NORMALIZATION) ? SPEED_NORMALIZATION : (uint32_t)t_temp;
                
                // Enhanced safety: Validate acceleration exponent
                uint8_t safe_exponent = ACCEL_CLAMP(cfg->acceleration_exponent, 1, 5);
                if (safe_exponent != cfg->acceleration_exponent) {
                    LOG_WRN("Level2: Clamping acceleration exponent from %u to %u", 
                            cfg->acceleration_exponent, safe_exponent);
                }
                
                uint32_t curve;
                // Enhanced safety: Try exponential curve with fallback
                #if defined(CONFIG_INPUT_PROCESSOR_ACCEL_LEVEL_STANDARD)
                curve = calculate_exponential_curve(t, safe_exponent);
                
                // Enhanced safety: Validate curve result
                if (curve > SPEED_NORMALIZATION * 10) {
                    LOG_WRN("Level2: Exponential curve result %u too large, using linear", curve);
                    curve = t; // Linear fallback
                }
                #else
                curve = t; // Linear fallback if exponential not available
                #endif
                
                // Final clamp and factor calculation
                curve = ACCEL_CLAMP(curve, 0, SPEED_NORMALIZATION);
                
                // Enhanced safety: Factor calculation with overflow protection
                if (cfg->max_factor >= cfg->min_factor) {
                    uint64_t factor_range = cfg->max_factor - cfg->min_factor;
                    
                    // Enhanced safety: Check for potential overflow
                    if (factor_range > UINT32_MAX / SPEED_NORMALIZATION) {
                        LOG_WRN("Level2: Factor range too large, using conservative calculation");
                        factor = cfg->min_factor + (factor_range * curve) / (SPEED_NORMALIZATION * 2);
                    } else {
                        uint64_t factor_add = (factor_range * curve) / SPEED_NORMALIZATION;
                        factor = cfg->min_factor + (uint32_t)ACCEL_CLAMP(factor_add, 0, factor_range);
                    }
                } else {
                    LOG_WRN("Level2: max_factor < min_factor, using min_factor");
                    factor = cfg->min_factor;
                }
            }
        }
        
        // Enhanced safety: Final factor validation
        factor = ACCEL_CLAMP(factor, cfg->min_factor, 
                           ACCEL_CLAMP(cfg->max_factor, SENSITIVITY_SCALE, MAX_SAFE_FACTOR));
        
        // Enhanced safety: Apply acceleration with comprehensive overflow protection
        if (factor > SENSITIVITY_SCALE) {
            // Check if multiplication would overflow
            if (abs(result) > (int64_t)INT16_MAX * SENSITIVITY_SCALE / factor) {
                LOG_WRN("Level2: Acceleration would cause overflow, using fallback");
                return accel_safe_fallback_calculate(input_value, factor);
            }
            
            int64_t temp_result = safe_multiply_64(result, (int64_t)factor, 
                                                 (int64_t)INT16_MAX * SENSITIVITY_SCALE);
            result = temp_result / SENSITIVITY_SCALE;
            
            // Enhanced safety: Check result after acceleration
            if (abs(result) > INT16_MAX) {
                LOG_WRN("Level2: Accelerated result %lld exceeds int16 range, clamping", result);
                result = (result > 0) ? INT16_MAX : INT16_MIN;
            }
        }
    }
    
    // Enhanced safety: Y-axis boost with comprehensive overflow protection
    if (code == INPUT_REL_Y && cfg->y_boost != SENSITIVITY_SCALE) {
        // Enhanced safety: Validate y_boost value
        uint32_t safe_y_boost = ACCEL_CLAMP(cfg->y_boost, 500, 3000);
        if (safe_y_boost != cfg->y_boost) {
            LOG_WRN("Level2: Clamping y_boost from %u to %u", cfg->y_boost, safe_y_boost);
        }
        
        // Enhanced safety: Check if Y-boost would cause overflow
        if (abs(result) > (int64_t)INT16_MAX * SENSITIVITY_SCALE / safe_y_boost) {
            LOG_WRN("Level2: Y-boost would cause overflow, using conservative boost");
            safe_y_boost = SENSITIVITY_SCALE + (safe_y_boost - SENSITIVITY_SCALE) / 2;
        }
        
        int64_t temp_result = safe_multiply_64(result, (int64_t)safe_y_boost, 
                                             (int64_t)INT16_MAX * SENSITIVITY_SCALE);
        result = temp_result / SENSITIVITY_SCALE;
        
        // Enhanced safety: Check result after Y-boost
        if (abs(result) > INT16_MAX) {
            LOG_WRN("Level2: Y-boosted result %lld exceeds int16 range, clamping", result);
            result = (result > 0) ? INT16_MAX : INT16_MIN;
        }
    }
    
    // Enhanced safety: Calculate final accelerated value with comprehensive validation
    int32_t accelerated_value = safe_int64_to_int32(result);
    
    // Enhanced safety: Multiple range checks for Level 2
    if (abs(accelerated_value) > INT16_MAX) {
        LOG_ERR("Level2: Accelerated value %d exceeds int16 range, clamping", accelerated_value);
        accelerated_value = (accelerated_value > 0) ? INT16_MAX : INT16_MIN;
    }
    
    // Enhanced safety: Sanity check for Level 2 - detect unreasonable results
    if (abs(input_value) <= 50 && abs(accelerated_value) > 2000) {
        LOG_WRN("Level2: Suspicious result %d for input %d, using conservative fallback", 
                accelerated_value, input_value);
        return accel_safe_fallback_calculate(input_value, cfg->max_factor);
    }
    
    // Remainder processing removed for safety and simplicity
    // The precision loss (1/1000) is negligible for practical mouse usage
    
    // Enhanced safety: Minimum movement guarantee with bounds check
    if (input_value != 0 && accelerated_value == 0) {
        accelerated_value = (input_value > 0) ? 1 : -1;
    }
    
    // Enhanced safety: Final comprehensive validation
    int16_t final_result = safe_int32_to_int16(accelerated_value);
    
    // Enhanced safety: Ultimate bounds check
    if (abs(final_result) > INT16_MAX) {
        LOG_ERR("Level2: Final result %d exceeds int16 bounds, emergency clamp", final_result);
        final_result = (final_result > 0) ? INT16_MAX : INT16_MIN;
    }
    
    // Enhanced safety: Log suspicious Level 2 results
    static uint32_t level2_log_counter = 0;
    if ((level2_log_counter++ % 200) == 0 || abs(final_result) > abs(input_value) * 10) {
        LOG_DBG("Level2: Input=%d, Speed=%u, Factor=%u, Final=%d", 
                input_value, speed, factor, final_result);
    }
    
    return final_result;
#endif
}


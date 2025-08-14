// input_processor_accel_calc_level1.c - Level 1 (Simple) calculation functions
// Optimized for safety and simplicity
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
// LEVEL 1 CALCULATION FUNCTION
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
    // Enhanced safety: Input value validation for reasonable range
    const int32_t MAX_REASONABLE_INPUT = 200;  // Covers most legitimate use cases
    
    if (abs(input_value) > MAX_REASONABLE_INPUT) {
        if (abs(input_value) > MAX_REASONABLE_INPUT * 3) {
            // Extremely large values (>600) are likely sensor noise - ignore them
            LOG_WRN("Level1: Input value %d too extreme, ignoring", input_value);
            return 0;
        } else {
            // Large but reasonable values (200-600) - clamp to limit
            LOG_DBG("Level1: Input value %d clamped to %d", input_value, MAX_REASONABLE_INPUT);
            input_value = (input_value > 0) ? MAX_REASONABLE_INPUT : -MAX_REASONABLE_INPUT;
        }
    }
    
    // Enhanced safety: Configuration validation
    if (cfg->sensitivity == 0 || cfg->sensitivity > MAX_SAFE_SENSITIVITY) {
        LOG_ERR("Level1: Invalid sensitivity %u, using default", cfg->sensitivity);
        return input_value; // Safe fallback
    }
    
    uint32_t dpi_adjusted_sensitivity = calculate_dpi_adjusted_sensitivity(cfg);
    
    // Analysis logging disabled for performance
    // LOG_DBG("Level1: input=%d, orig_sens=%u, sensor_dpi=%u, adj_sens=%u", 
    //         input_value, cfg->sensitivity, cfg->sensor_dpi, dpi_adjusted_sensitivity);
    
    // Enhanced safety: Check sensitivity bounds
    if (dpi_adjusted_sensitivity == 0 || dpi_adjusted_sensitivity > MAX_SAFE_SENSITIVITY) {
        LOG_ERR("Level1: Invalid DPI-adjusted sensitivity %u, using passthrough", 
                dpi_adjusted_sensitivity);
        return input_value;
    }
    
    // CRITICAL FIX: The DPI adjustment was being applied incorrectly
    // We need to apply sensitivity as a multiplier, not a direct multiplication
    int64_t result = (int64_t)input_value * (int64_t)dpi_adjusted_sensitivity;
    
    // Analysis logging disabled for performance
    // LOG_DBG("Level1: input=%d * adj_sens=%u = raw_result=%lld", 
    //         input_value, dpi_adjusted_sensitivity, result);
    
    // Enhanced safety: Check intermediate result
    if (abs(result) > (int64_t)INT32_MAX * SENSITIVITY_SCALE / 2) {
        LOG_WRN("Level1: Intermediate result %lld too large, scaling down", result);
        result = result / 2; // Emergency scaling
    }
    
    // CRITICAL FIX: Always apply sensitivity scaling
    result = result / SENSITIVITY_SCALE;
    
    // Level 1 curve processing
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
            case 1: // Mild - Improved quadratic approximation
                {
                    // More effective calculation: input^2 * multiplier / divisor
                    uint64_t quad_add = safe_multiply_64((int64_t)abs_input * abs_input, 
                                                       25LL, (int64_t)safe_max_factor);
                    quad_add = quad_add / 100; // Scale down
                    uint32_t max_add = (safe_max_factor > SENSITIVITY_SCALE) ? 
                        safe_max_factor - SENSITIVITY_SCALE : 0;
                    curve_factor = SENSITIVITY_SCALE + ACCEL_CLAMP((uint32_t)quad_add, 0, max_add);
                    
                    // Debug log for curve calculation
                    LOG_DBG("Level1 Mild: input=%d, quad_add=%llu, curve_factor=%u", 
                            abs_input, quad_add, curve_factor);
                }
                break;
            case 2: // Strong - Improved quadratic approximation
                {
                    // More aggressive calculation for strong curve
                    uint64_t quad_add = safe_multiply_64((int64_t)abs_input * abs_input, 
                                                       50LL, (int64_t)safe_max_factor);
                    quad_add = quad_add / 100; // Scale down
                    uint32_t max_add = (safe_max_factor > SENSITIVITY_SCALE) ? 
                        safe_max_factor - SENSITIVITY_SCALE : 0;
                    curve_factor = SENSITIVITY_SCALE + ACCEL_CLAMP((uint32_t)quad_add, 0, max_add);
                    
                    // Debug log for curve calculation
                    LOG_DBG("Level1 Strong: input=%d, quad_add=%llu, curve_factor=%u", 
                            abs_input, quad_add, curve_factor);
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
    
    // Enhanced safety: Improved minimum movement guarantee based on calculation result
    if (input_value != 0 && result == 0) {
        // Calculate what the raw result would have been before scaling
        int64_t raw_result = (int64_t)input_value * (int64_t)dpi_adjusted_sensitivity;
        
        // Only output movement if the raw calculation was >= 0.5 (half of SENSITIVITY_SCALE)
        if (abs(raw_result) >= SENSITIVITY_SCALE / 2) {
            result = (raw_result > 0) ? 1 : -1;
            LOG_DBG("Level1: Minimum movement applied - raw=%lld -> output=%lld", raw_result, result);
        } else {
            // Raw calculation was < 0.5, legitimately should be 0
            result = 0;
            LOG_DBG("Level1: Micro movement ignored - raw=%lld (< 0.5 threshold)", raw_result);
        }
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
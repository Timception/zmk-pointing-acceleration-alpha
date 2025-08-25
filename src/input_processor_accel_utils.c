// input_processor_accel_utils.c - Utility functions for acceleration processor
// Separated for better code organization and maintainability

#include <zephyr/logging/log.h>
#include <stdlib.h>
#include "../include/drivers/input_processor_accel.h"

LOG_MODULE_DECLARE(input_processor_accel);



// =============================================================================
// MATHEMATICAL UTILITY FUNCTIONS
// =============================================================================

uint32_t accel_safe_quadratic_curve(int32_t abs_input, uint32_t multiplier) {
    // Enhanced safety: Input validation
    if (abs_input < 0) {
        LOG_ERR("Invalid negative input to quadratic curve: %d", abs_input);
        return SENSITIVITY_SCALE; // Return 1.0x factor
    }
    
    // Enhanced safety: Multiplier validation
    if (multiplier == 0 || multiplier > 1000) {
        LOG_ERR("Invalid multiplier for quadratic curve: %u", multiplier);
        return SENSITIVITY_SCALE; // Return 1.0x factor
    }
    
    // Enhanced safety: Prevent overflow with more conservative limit
    const uint32_t max_safe_input = 1000;  // More conservative for Level 1
    
    if (abs_input > max_safe_input) {
        LOG_DBG("Clamping input %d to safe limit %u", abs_input, max_safe_input);
        abs_input = max_safe_input;
    }
    
    // Enhanced safety: Check for potential overflow before calculation
    if (abs_input > 0 && multiplier > UINT32_MAX / (abs_input * abs_input)) {
        LOG_WRN("Potential overflow detected, using conservative calculation");
        // Use linear approximation instead
        uint32_t linear_result = SENSITIVITY_SCALE + (abs_input * multiplier / 10);
        return ACCEL_CLAMP(linear_result, SENSITIVITY_SCALE, MAX_SAFE_FACTOR);
    }
    
    // Safe calculation with enhanced overflow check
    uint64_t temp = (uint64_t)abs_input * abs_input * multiplier;
    
    // Enhanced safety: More conservative overflow check
    if (temp > (UINT32_MAX - SENSITIVITY_SCALE) / 100) {
        LOG_WRN("Quadratic result too large, using maximum safe value");
        return MAX_SAFE_FACTOR;
    }
    
    // Scale down the result to prevent excessive acceleration
    uint32_t scaled_result = SENSITIVITY_SCALE + (uint32_t)(temp / 100);
    
    // Enhanced safety: Final bounds check
    uint32_t final_result = ACCEL_CLAMP(scaled_result, SENSITIVITY_SCALE, MAX_SAFE_FACTOR);
    
    // Enhanced safety: Sanity check - result should be reasonable
    if (final_result > SENSITIVITY_SCALE * 5) { // Max 5x acceleration
        LOG_WRN("Quadratic curve result %u seems excessive for input %d, limiting to 5x", 
                final_result, abs_input);
        final_result = SENSITIVITY_SCALE * 5;
    }
    
    return final_result;
}

// =============================================================================
// TIMING FUNCTIONS
// =============================================================================

/**
 * @brief Simplified speed calculation - no accumulation risk
 * @param data Acceleration data structure
 * @param input_value Current input value
 * @return Calculated speed (safe, bounded)
 */
uint32_t accel_calculate_simple_speed(struct accel_data *data, int32_t input_value) {
    if (!data) {
        LOG_ERR("Data pointer is NULL in speed calculation");
        return abs(input_value) * ACCEL_SPEED_SCALE_FACTOR; // Simple fallback
    }
    
    uint32_t current_time_ms = k_uptime_get_32();
    uint32_t last_time_ms = data->last_time_ms;
    
    // Input value validation
    int32_t abs_input = abs(input_value);
    if (abs_input > MAX_SAFE_INPUT_VALUE) {
        abs_input = MAX_SAFE_INPUT_VALUE;
    }
    
    // Handle first call or time overflow
    if (last_time_ms == 0 || current_time_ms < last_time_ms) {
        data->last_time_ms = current_time_ms;
        data->recent_speed = abs_input * ACCEL_SPEED_SCALE_FACTOR; // Initial speed estimation
        // speed_samples removed for memory optimization
        return data->recent_speed;
    }
    
    uint32_t time_delta_ms = current_time_ms - last_time_ms;
    uint16_t current_speed;
    
    // **Fixed**: Correct speed calculation (counts per second) with overflow protection
    if (time_delta_ms > 0 && time_delta_ms < 1000) { // Within 1 second
        // Speed = movement / time * 1000 (counts/sec)
        // Enhanced safety: Check for potential overflow before multiplication
        if (abs_input > UINT32_MAX / 1000) {
            current_speed = UINT16_MAX; // Cap at maximum
        } else {
            uint32_t temp_speed = (abs_input * 1000) / time_delta_ms;
            current_speed = (temp_speed > UINT16_MAX) ? UINT16_MAX : (uint16_t)temp_speed;
        }
    } else {
        // Input-based estimation when time is too long
        // Enhanced safety: Prevent overflow in multiplication
        uint32_t temp_speed = (uint32_t)abs_input * ACCEL_SPEED_SCALE_FACTOR;
        current_speed = (temp_speed > UINT16_MAX) ? UINT16_MAX : (uint16_t)temp_speed;
    }
    
    // Speed samples removed for memory optimization
    
    // Exponential moving average (smoother speed changes)
    uint16_t alpha = 300; // 0.3 in thousandths
    uint16_t averaged_speed = (data->recent_speed * (1000 - alpha) + current_speed * alpha) / 1000;
    
    // Update state
    data->last_time_ms = current_time_ms;
    data->recent_speed = averaged_speed;
    
    return (uint32_t)averaged_speed;
}

// Enhanced safety: Safe fallback calculation for when Level 2 causes issues
int32_t accel_safe_fallback_calculate(int32_t input_value, uint32_t max_factor) {
    if (input_value == 0) {
        return 0;
    }
    
    // Enhanced safety: Input validation
    input_value = accel_clamp_input_value(input_value);
    int32_t abs_input = abs(input_value);
    int32_t result = input_value;
    
    // Enhanced safety: Max factor validation
    uint32_t safe_max_factor = ACCEL_CLAMP(max_factor, SENSITIVITY_SCALE, MAX_SAFE_FACTOR);
    if (safe_max_factor != max_factor) {
        LOG_DBG("Fallback: Clamping max_factor from %u to %u", max_factor, safe_max_factor);
    }
    
    // Enhanced safety: Simple linear acceleration based on input magnitude
    if (abs_input > 5) { // Lower threshold for more responsive fallback
        // Apply conservative acceleration for larger movements
        uint32_t base_factor = SENSITIVITY_SCALE;
        uint32_t accel_add = (abs_input * 3); // More conservative than before
        
        // Enhanced safety: Prevent overflow in factor calculation
        if (accel_add > safe_max_factor - base_factor) {
            accel_add = safe_max_factor - base_factor;
        }
        
        uint32_t factor = base_factor + accel_add;
        factor = ACCEL_CLAMP(factor, SENSITIVITY_SCALE, safe_max_factor);
        
        // Enhanced safety: Safe multiplication with overflow check
        if (abs(input_value) > INT16_MAX * SENSITIVITY_SCALE / factor) {
            LOG_WRN("Fallback: Would overflow, using linear scaling");
            result = input_value * 2; // Simple 2x scaling
        } else {
            int64_t temp = safe_multiply_64((int64_t)input_value, (int64_t)factor, 
                                          (int64_t)INT16_MAX * SENSITIVITY_SCALE);
            temp = temp / SENSITIVITY_SCALE;
            result = (int32_t)ACCEL_CLAMP(temp, INT16_MIN, INT16_MAX);
        }
    }
    
    // Enhanced safety: Final validation
    result = (int32_t)ACCEL_CLAMP(result, INT16_MIN, INT16_MAX);
    
    // Enhanced safety: Sanity check
    if (abs(input_value) <= 20 && abs(result) > abs(input_value) * 10) {
        LOG_WRN("Fallback: Suspicious result %d for input %d, using conservative", 
                result, input_value);
        result = input_value * 2; // Very conservative fallback
    }
    
    return result;
}

// Utility functions for acceleration calculations
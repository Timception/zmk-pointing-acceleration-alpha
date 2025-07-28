// input_processor_accel_calc.c - Level-specific calculation functions
// Separated for better code organization and maintainability

#include <zephyr/logging/log.h>
#include <math.h>
#include <stdlib.h>
#include <drivers/input_processor_accel.h>

LOG_MODULE_DECLARE(input_processor_accel);

// =============================================================================
// LEVEL-SPECIFIC CALCULATION FUNCTIONS
// =============================================================================

// Simple acceleration: Just apply sensitivity and basic curve with safety
int32_t accel_simple_calculate(const struct accel_config *cfg, int32_t input_value, uint16_t code) {
    // Apply base sensitivity with overflow protection
    int64_t result = ((int64_t)input_value * cfg->sensitivity) / 1000;
    
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
}

// Standard acceleration: Speed-based with Y-axis boost and enhanced timing
int32_t accel_standard_calculate(const struct accel_config *cfg, struct accel_data *data, 
                                int32_t input_value, uint16_t code) {
    // Use enhanced speed calculation
    uint32_t speed = accel_calculate_enhanced_speed(&data->timing, input_value);
    
    // Base sensitivity with overflow protection
    int64_t result = ((int64_t)input_value * cfg->sensitivity) / 1000;
    
    // Speed-based acceleration
    if (speed > cfg->speed_threshold) {
        uint32_t factor = 1000;
        if (speed >= cfg->speed_max) {
            factor = cfg->max_factor;
        } else {
            uint32_t speed_range = cfg->speed_max - cfg->speed_threshold;
            uint32_t speed_offset = speed - cfg->speed_threshold;
            uint32_t t = (speed_offset * 1000) / speed_range;
            
            // Apply curve with overflow protection
            uint32_t curve;
            switch (cfg->curve_type) {
                case 0: curve = t; break; // Linear
                case 1: curve = (t * t) / 1000; break; // Quadratic
                case 2: {
                    // Cubic with overflow protection
                    uint64_t temp = (uint64_t)t * t * t;
                    curve = (uint32_t)(temp / (1000ULL * 1000ULL));
                    break;
                }
                default: curve = (t * t) / 1000; break;
            }
            
            curve = ACCEL_CLAMP(curve, 0, 1000);
            factor = 1000 + (((cfg->max_factor - 1000) * curve) / 1000);
        }
        
        factor = ACCEL_CLAMP(factor, 1000, cfg->max_factor);
        result = (result * factor) / 1000;
    }
    
    // Y-axis boost with overflow protection
    if (code == INPUT_REL_Y) {
        result = (result * cfg->y_boost) / 1000;
    }
    
    // Clamp result to safe range
    return (int32_t)ACCEL_CLAMP(result, INT16_MIN, INT16_MAX);
}

// Advanced acceleration: Full-featured with enhanced safety and timing
int32_t accel_advanced_calculate(const struct accel_config *cfg, struct accel_data *data, 
                                int32_t input_value, uint16_t code) {
    // Use enhanced speed calculation
    uint32_t speed = accel_calculate_enhanced_speed(&data->timing, input_value);
    
    // Acceleration factor calculation
    uint16_t factor = cfg->min_factor;
    
    if (speed > cfg->speed_threshold) {
        if (speed >= cfg->speed_max) {
            factor = cfg->max_factor;
        } else {
            uint32_t speed_range = cfg->speed_max - cfg->speed_threshold;
            uint32_t speed_offset = speed - cfg->speed_threshold;
            uint32_t t_int = (speed_offset * 1000) / speed_range;

            // Apply acceleration curve based on exponent with overflow protection
            uint32_t curve;
            switch (cfg->acceleration_exponent) {
                case 1:
                    curve = t_int;
                    break;
                case 2:
                    {
                        t_int = ACCEL_CLAMP(t_int, 0, 500); // Prevent overflow
                        uint32_t x = (t_int * 2000) / 1000;
                        uint32_t x2 = (x * x) / 1000;
                        uint32_t x3 = (x2 * x) / 1000;
                        curve = x + (x2 / 2) + (x3 / 6);
                        curve = ACCEL_CLAMP(curve, 0, 1000);
                    }
                    break;
                case 10:
                    curve = (t_int * t_int) / 1000;
                    break;
                default:
                    curve = (t_int * t_int) / 1000;
                    break;
            }
            
            curve = ACCEL_CLAMP(curve, 0, 1000);
            factor = cfg->min_factor + (uint16_t)(((cfg->max_factor - cfg->min_factor) * curve) / 1000);
            factor = ACCEL_CLAMP(factor, cfg->min_factor, cfg->max_factor);
        }
    }

    // DPI adjustment factor with overflow protection
    uint32_t dpi_factor = ((uint32_t)cfg->target_dpi * cfg->dpi_multiplier) / cfg->sensor_dpi;
    
    // Auto-scale for high-resolution displays
    #if CONFIG_INPUT_PROCESSOR_ACCEL_AUTO_SCALE_4K
    if (dpi_factor < 1000) {
        dpi_factor = (dpi_factor * 1500) / 1000;
    }
    dpi_factor = ACCEL_CLAMP(dpi_factor, 100, 5000);
    #endif
    
    // Aspect ratio adjustment
    uint16_t aspect_scale = (code == INPUT_REL_X) ? cfg->x_aspect_scale : cfg->y_aspect_scale;
    
    // Additional Y-axis acceleration boost with overflow protection
    if (code == INPUT_REL_Y && factor > cfg->min_factor) {
        uint16_t y_boost = ((factor - cfg->min_factor) * 200) / (cfg->max_factor - cfg->min_factor);
        y_boost = ACCEL_CLAMP(y_boost, 0, 500);
        aspect_scale = (aspect_scale * (1000 + y_boost)) / 1000;
        aspect_scale = ACCEL_CLAMP(aspect_scale, 500, 3000);
    }
    
    // Precise calculation with overflow protection
    int64_t precise_value = ((int64_t)input_value * factor * dpi_factor * aspect_scale);
    int32_t accelerated_value = (int32_t)(precise_value / (1000LL * 1000LL * 1000LL));
    
    // Clamp to prevent extreme values
    accelerated_value = ACCEL_CLAMP(accelerated_value, INT16_MIN, INT16_MAX);
    
    // Thread-safe remainder processing
    if (cfg->track_remainders) {
        // Use mutex for complex remainder operations
        if (k_mutex_lock(&data->mutex, K_MSEC(1)) == 0) {
            uint8_t remainder_idx = (code == INPUT_REL_X) ? 0 : 1;
            int32_t remainder = (int32_t)((precise_value % (1000LL * 1000LL * 1000LL)) / (1000LL * 1000LL));
            
            remainder = ACCEL_CLAMP(remainder, -10000, 10000);
            
            int32_t current_remainder = atomic_get(&data->remainders[remainder_idx]);
            int32_t new_remainder = current_remainder + remainder;
            new_remainder = ACCEL_CLAMP(new_remainder, INT16_MIN, INT16_MAX);
            
            atomic_set(&data->remainders[remainder_idx], new_remainder);
            
            if (abs(new_remainder) >= 1000) {
                int32_t carry = new_remainder / 1000;
                carry = ACCEL_CLAMP(carry, -10, 10);
                
                accelerated_value += carry;
                accelerated_value = ACCEL_CLAMP(accelerated_value, INT16_MIN, INT16_MAX);
                
                atomic_set(&data->remainders[remainder_idx], new_remainder - carry * 1000);
            }
            
            k_mutex_unlock(&data->mutex);
        }
    }
    
    // Minimum movement guarantee
    if (input_value != 0 && accelerated_value == 0) {
        accelerated_value = (input_value > 0) ? 1 : -1;
    }
    
    // Thread-safe smooth large jumps prevention
    int32_t last_factor = atomic_get(&data->last_factor);
    if (last_factor > 0) {
        int32_t factor_diff = abs((int32_t)factor - last_factor);
        if (factor_diff > 500) {
            uint16_t smooth_factor = last_factor + ((factor > last_factor) ? 250 : -250);
            smooth_factor = ACCEL_CLAMP(smooth_factor, cfg->min_factor, cfg->max_factor);
            
            int64_t smooth_value = ((int64_t)input_value * smooth_factor * dpi_factor * aspect_scale);
            int32_t smooth_accelerated = (int32_t)(smooth_value / (1000LL * 1000LL * 1000LL));
            smooth_accelerated = ACCEL_CLAMP(smooth_accelerated, INT16_MIN, INT16_MAX);
            
            if (abs(smooth_accelerated - accelerated_value) > abs(accelerated_value) / 4) {
                accelerated_value = smooth_accelerated;
                factor = smooth_factor;
            }
        }
    }
    
    // Thread-safe factor update
    atomic_set(&data->last_factor, factor);
    
    return accelerated_value;
}
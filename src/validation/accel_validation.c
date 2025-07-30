// accel_validation.c - Minimal validation for MCU efficiency
// Only essential checks to prevent system crashes

#include <zephyr/logging/log.h>
#include <drivers/input_processor_accel.h>

LOG_MODULE_DECLARE(input_processor_accel_main);

// =============================================================================
// MINIMAL VALIDATION FUNCTIONS
// =============================================================================

int accel_validate_config(const struct accel_config *cfg) {
    // Basic null check
    if (!cfg) {
        return -EINVAL;
    }

    // Only check values that could cause crashes or undefined behavior
    
    // Prevent division by zero
    if (cfg->sensor_dpi == 0) {
        return -EINVAL;
    }
    
    // Prevent extreme values that could cause overflow
    if (cfg->max_factor > MAX_SAFE_FACTOR || cfg->max_factor < 1000) {
        return -EINVAL;
    }
    
    if (cfg->sensitivity > MAX_SAFE_SENSITIVITY || cfg->sensitivity < MIN_SAFE_SENSITIVITY) {
        return -EINVAL;
    }
    
    // Level 2 specific checks
    if (cfg->level == 2) {
        // Prevent division by zero in speed calculation
        if (cfg->speed_max <= cfg->speed_threshold) {
            return -EINVAL;
        }
        
        // Prevent invalid factor relationship
        if (cfg->min_factor > cfg->max_factor) {
            return -EINVAL;
        }
        
        // Basic range check for exponent
        if (cfg->acceleration_exponent < 1 || cfg->acceleration_exponent > 5) {
            return -EINVAL;
        }
    }
    
    return 0;
}
// accel_validation.c - Minimal validation for MCU efficiency
// Only essential checks to prevent system crashes

#include <zephyr/logging/log.h>
#include "../include/drivers/input_processor_accel.h"

LOG_MODULE_DECLARE(input_processor_accel);

// =============================================================================
// MINIMAL VALIDATION FUNCTIONS
// =============================================================================

int accel_validate_config(const struct accel_config *cfg) {

    if (!cfg) {
        LOG_ERR("Configuration pointer is NULL");
        return -EINVAL;
    }

    // Validate configuration level
    if (cfg->level < 1 || cfg->level > 2) {
        LOG_ERR("Invalid configuration level: %u (must be 1 or 2)", cfg->level);
        return -EINVAL;
    }

    // Prevent division by zero in DPI calculations
    if (cfg->sensor_dpi == 0) {
        LOG_ERR("Invalid sensor DPI: 0 (division by zero risk)");
        return -EINVAL;
    }
    
    // Validate DPI range
    if (cfg->sensor_dpi < 400 || cfg->sensor_dpi > MAX_SENSOR_DPI) {
        LOG_ERR("Sensor DPI %u out of reasonable range (400-%u)", cfg->sensor_dpi, MAX_SENSOR_DPI);
        return -EINVAL;
    }
    
    // Prevent extreme values that could cause overflow
    if (cfg->max_factor > MAX_SAFE_FACTOR) {
        LOG_ERR("Max factor %u exceeds safe limit %u", cfg->max_factor, MAX_SAFE_FACTOR);
        return -EINVAL;
    }
    if (cfg->max_factor < 1000) {
        LOG_ERR("Max factor %u below minimum 1000 (1.0x)", cfg->max_factor);
        return -EINVAL;
    }
    
    // Validate sensitivity range
    if (cfg->sensitivity > MAX_SAFE_SENSITIVITY) {
        LOG_ERR("Sensitivity %u exceeds safe limit %u", cfg->sensitivity, MAX_SAFE_SENSITIVITY);
        return -EINVAL;
    }
    if (cfg->sensitivity < MIN_SAFE_SENSITIVITY) {
        LOG_ERR("Sensitivity %u below minimum %u", cfg->sensitivity, MIN_SAFE_SENSITIVITY);
        return -EINVAL;
    }
    
    // Validate curve type
    if (cfg->curve_type > 2) {
        LOG_ERR("Invalid curve type: %u (must be 0-2)", cfg->curve_type);
        return -EINVAL;
    }
    
    // Level 2 specific validation
    if (cfg->level == 2) {
        // Prevent division by zero in speed calculation
        if (cfg->speed_max <= cfg->speed_threshold) {
            LOG_ERR("Speed max (%u) must be greater than speed threshold (%u)", 
                    cfg->speed_max, cfg->speed_threshold);
            return -EINVAL;
        }
        
        // Validate speed ranges
        if (cfg->speed_threshold > MAX_REASONABLE_SPEED) {
            LOG_ERR("Speed threshold %u exceeds reasonable limit %u", 
                    cfg->speed_threshold, MAX_REASONABLE_SPEED);
            return -EINVAL;
        }
        if (cfg->speed_max > MAX_REASONABLE_SPEED) {
            LOG_ERR("Speed max %u exceeds reasonable limit %u", 
                    cfg->speed_max, MAX_REASONABLE_SPEED);
            return -EINVAL;
        }
        
        // Prevent invalid factor relationship
        if (cfg->min_factor > cfg->max_factor) {
            LOG_ERR("Min factor (%u) cannot be greater than max factor (%u)", 
                    cfg->min_factor, cfg->max_factor);
            return -EINVAL;
        }
        
        // Validate factor ranges
        if (cfg->min_factor < 200 || cfg->min_factor > 2000) {
            LOG_ERR("Min factor %u out of reasonable range (200-2000)", cfg->min_factor);
            return -EINVAL;
        }
        
        // Validate Y-axis boost
        if (cfg->y_boost < 500 || cfg->y_boost > 3000) {
            LOG_ERR("Y-axis boost %u out of reasonable range (500-3000)", cfg->y_boost);
            return -EINVAL;
        }
        
        // Validate acceleration exponent
        if (cfg->acceleration_exponent < 1 || cfg->acceleration_exponent > 5) {
            LOG_ERR("Acceleration exponent %u out of valid range (1-5)", cfg->acceleration_exponent);
            return -EINVAL;
        }
    }
    
    // Logical consistency checks
    if (cfg->max_factor <= cfg->sensitivity) {
        LOG_WRN("Max factor (%u) should typically be greater than sensitivity (%u)", 
                cfg->max_factor, cfg->sensitivity);
    }
    
    LOG_DBG("Configuration validation passed for level %u", cfg->level);
    return 0;
}
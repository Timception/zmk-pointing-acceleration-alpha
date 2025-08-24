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

    // Validate DPI class
    if (cfg->sensor_dpi_class > 6) {
        LOG_ERR("Invalid sensor DPI class: %u (must be 0-6)", cfg->sensor_dpi_class);
        return -EINVAL;
    }
    
    uint16_t sensor_dpi = accel_decode_sensor_dpi(cfg->sensor_dpi_class);
    if (sensor_dpi < 400 || sensor_dpi > MAX_SENSOR_DPI) {
        LOG_ERR("Sensor DPI %u out of reasonable range (400-%u)", sensor_dpi, MAX_SENSOR_DPI);
        return -EINVAL;
    }
    
    // Prevent extreme values that could cause overflow
    uint16_t max_factor = (cfg->level == 1) ? cfg->cfg.level1.max_factor : cfg->cfg.level2.max_factor;
    if (max_factor > MAX_SAFE_FACTOR) {
        LOG_ERR("Max factor %u exceeds safe limit %u", max_factor, MAX_SAFE_FACTOR);
        return -EINVAL;
    }
    if (max_factor < 1000) {
        LOG_ERR("Max factor %u below minimum 1000 (1.0x)", max_factor);
        return -EINVAL;
    }
    
    // Validate sensitivity range
    uint16_t sensitivity = (cfg->level == 1) ? cfg->cfg.level1.sensitivity : cfg->cfg.level2.sensitivity;
    if (sensitivity > MAX_SAFE_SENSITIVITY) {
        LOG_ERR("Sensitivity %u exceeds safe limit %u", sensitivity, MAX_SAFE_SENSITIVITY);
        return -EINVAL;
    }
    if (sensitivity < MIN_SAFE_SENSITIVITY) {
        LOG_ERR("Sensitivity %u below minimum %u", sensitivity, MIN_SAFE_SENSITIVITY);
        return -EINVAL;
    }
    
    // Validate curve type (Level 1 only)
    if (cfg->level == 1 && cfg->cfg.level1.curve_type > 2) {
        LOG_ERR("Invalid curve type: %u (must be 0-2)", cfg->cfg.level1.curve_type);
        return -EINVAL;
    }
    
    // Validate Y-axis boost (common to both levels)
    uint16_t y_boost = accel_decode_y_boost(cfg->y_boost_scaled);
    if (y_boost < 500 || y_boost > 3000) {
        LOG_ERR("Y-axis boost %u out of reasonable range (500-3000)", y_boost);
        return -EINVAL;
    }
    
    // Level-specific validation
    if (cfg->level == 1) {
        LOG_DBG("Level 1 (Simple) validation: basic checks only");
        // Level 1 uses basic validation only
        // speed_threshold, speed_max, min_factor, acceleration_exponent are not used
        
    } else if (cfg->level == 2) {
        LOG_DBG("Level 2 (Standard) validation: comprehensive checks");
        
        // Prevent division by zero in speed calculation
        if (cfg->cfg.level2.speed_max <= cfg->cfg.level2.speed_threshold) {
            LOG_ERR("Speed max (%u) must be greater than speed threshold (%u)", 
                    cfg->cfg.level2.speed_max, cfg->cfg.level2.speed_threshold);
            return -EINVAL;
        }
        
        // Validate speed ranges
        if (cfg->cfg.level2.speed_threshold > MAX_REASONABLE_SPEED) {
            LOG_ERR("Speed threshold %u exceeds reasonable limit %u", 
                    cfg->cfg.level2.speed_threshold, MAX_REASONABLE_SPEED);
            return -EINVAL;
        }
        if (cfg->cfg.level2.speed_max > MAX_REASONABLE_SPEED) {
            LOG_ERR("Speed max %u exceeds reasonable limit %u", 
                    cfg->cfg.level2.speed_max, MAX_REASONABLE_SPEED);
            return -EINVAL;
        }
        
        // Prevent invalid factor relationship
        if (cfg->cfg.level2.min_factor > cfg->cfg.level2.max_factor) {
            LOG_ERR("Min factor (%u) cannot be greater than max factor (%u)", 
                    cfg->cfg.level2.min_factor, cfg->cfg.level2.max_factor);
            return -EINVAL;
        }
        
        // Validate factor ranges
        if (cfg->cfg.level2.min_factor < 200 || cfg->cfg.level2.min_factor > 2000) {
            LOG_ERR("Min factor %u out of reasonable range (200-2000)", cfg->cfg.level2.min_factor);
            return -EINVAL;
        }
        
        // Validate acceleration exponent
        if (cfg->cfg.level2.acceleration_exponent < 1 || cfg->cfg.level2.acceleration_exponent > 5) {
            LOG_ERR("Acceleration exponent %u out of valid range (1-5)", cfg->cfg.level2.acceleration_exponent);
            return -EINVAL;
        }
    }
    
    // Logical consistency checks
    if (max_factor <= sensitivity) {
        LOG_WRN("Max factor (%u) should typically be greater than sensitivity (%u)", 
                max_factor, sensitivity);
    }
    
    LOG_DBG("Configuration validation passed for level %u", cfg->level);
    return 0;
}
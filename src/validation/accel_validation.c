// accel_validation.c - Configuration validation
// Centralized validation logic with detailed error reporting

#include <zephyr/logging/log.h>
#include <drivers/input_processor_accel.h>

LOG_MODULE_DECLARE(input_processor_accel_main);

// =============================================================================
// VALIDATION FUNCTIONS
// =============================================================================

static int validate_common_config(const struct accel_config *cfg) {
    if (!IS_VALID_RANGE(cfg->max_factor, 1000, MAX_SAFE_FACTOR)) {
        LOG_ERR("Invalid max_factor: %u (range: 1000-%u)", 
                cfg->max_factor, MAX_SAFE_FACTOR);
        return -EINVAL;
    }
    
    if (!IS_VALID_RANGE(cfg->sensor_dpi, 400, 8000)) {
        LOG_ERR("Invalid sensor_dpi: %u (range: 400-8000)", cfg->sensor_dpi);
        return -EINVAL;
    }
    
    return 0;
}

static int validate_level1_config(const struct accel_config *cfg) {
    if (!IS_VALID_RANGE(cfg->sensitivity, MIN_SAFE_SENSITIVITY, MAX_SAFE_SENSITIVITY)) {
        LOG_ERR("Level 1: Invalid sensitivity: %u (range: %u-%u)", 
                cfg->sensitivity, MIN_SAFE_SENSITIVITY, MAX_SAFE_SENSITIVITY);
        return -EINVAL;
    }
    
    if (cfg->curve_type > 2) {
        LOG_ERR("Level 1: Invalid curve_type: %u (range: 0-2)", cfg->curve_type);
        return -EINVAL;
    }
    
    return 0;
}

static int validate_level2_config(const struct accel_config *cfg) {
    int ret = validate_level1_config(cfg);
    if (ret < 0) return ret;
    
    if (cfg->speed_threshold >= cfg->speed_max) {
        LOG_ERR("Level 2: speed_threshold (%u) must be < speed_max (%u)", 
                cfg->speed_threshold, cfg->speed_max);
        return -EINVAL;
    }
    
    if (!IS_VALID_RANGE(cfg->y_boost, 500, 3000)) {
        LOG_ERR("Level 2: Invalid y_boost: %u (range: 500-3000)", cfg->y_boost);
        return -EINVAL;
    }
    
    return 0;
}

static int validate_level3_config(const struct accel_config *cfg) {
    if (cfg->min_factor > cfg->max_factor) {
        LOG_ERR("Level 3: min_factor (%u) must be <= max_factor (%u)", 
                cfg->min_factor, cfg->max_factor);
        return -EINVAL;
    }
    
    if (cfg->speed_threshold >= cfg->speed_max) {
        LOG_ERR("Level 3: speed_threshold (%u) must be < speed_max (%u)", 
                cfg->speed_threshold, cfg->speed_max);
        return -EINVAL;
    }
    
    if (!IS_VALID_RANGE(cfg->acceleration_exponent, 1, 13)) {
        LOG_ERR("Level 3: Invalid acceleration_exponent: %u (range: 1-13)", 
                cfg->acceleration_exponent);
        return -EINVAL;
    }
    
    return 0;
}

int accel_validate_config(const struct accel_config *cfg) {
    if (!cfg) {
        LOG_ERR("Configuration is NULL");
        return -EINVAL;
    }

    // Validate common settings
    int ret = validate_common_config(cfg);
    if (ret < 0) return ret;

    // Validate level-specific settings
    switch (cfg->level) {
        case 1:
            ret = validate_level1_config(cfg);
            break;
        case 2:
            ret = validate_level2_config(cfg);
            break;
        case 3:
            ret = validate_level3_config(cfg);
            break;
        default:
            LOG_ERR("Invalid configuration level: %u", cfg->level);
            return -EINVAL;
    }

    if (ret == 0) {
        LOG_DBG("Configuration validation passed for level %u", cfg->level);
    }

    return ret;
}
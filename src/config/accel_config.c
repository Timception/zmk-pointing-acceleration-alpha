// accel_config.c - Unified configuration management
// Centralizes all configuration logic and reduces complexity
//
// Copyright (c) 2024 The ZMK Contributors
// Modifications (c) 2025 NUOVOTAKA
// SPDX-License-Identifier: MIT

#include <zephyr/logging/log.h>
#include <zephyr/devicetree.h>
#include <string.h>
#include "accel_config.h"

LOG_MODULE_DECLARE(input_processor_accel);

// =============================================================================
// DEFAULT CONFIGURATIONS
// =============================================================================

static const struct accel_config level1_defaults = {
    .level = 1,
    .sensitivity = 1000,       // 1.0x (neutral base sensitivity)
    .max_factor = 2000,        // 2.0x (mild acceleration, not too aggressive)
    .curve_type = 1,           // Mild (smooth acceleration curve)
    .y_boost = 1000,           // 1.0x (no Y-axis boost)
    .speed_threshold = 800,    // Moderate threshold for balanced control
    .speed_max = 3000,         // Reasonable maximum speed
    .min_factor = 1000,        // 1.0x (no minimum acceleration)
    .acceleration_exponent = 2,
    .sensor_dpi = 800          // Standard DPI sensor support
};

static const struct accel_config level2_defaults = {
    .level = 2,
    .sensitivity = 1000,       // 1.0x (neutral base sensitivity)
    .max_factor = 2500,        // 2.5x (moderate acceleration for standard level)
    .curve_type = 1,           // Mild (smooth acceleration curve)
    .y_boost = 1000,           // 1.0x (no Y-axis boost by default)
    .speed_threshold = 600,    // Lower threshold for more responsive acceleration
    .speed_max = 3500,         // Higher maximum speed for standard level
    .min_factor = 900,         // 0.9x (slight precision boost for slow movements)
    .acceleration_exponent = 2, // Mild exponential curve
    .sensor_dpi = 800          // Standard DPI sensor support
};



// =============================================================================
// CONFIGURATION FUNCTIONS
// =============================================================================

const struct accel_config *accel_config_get_defaults(uint8_t level) {
    switch (level) {
        case 1: return &level1_defaults;
        case 2: return &level2_defaults;
        default:
            LOG_ERR("Invalid configuration level: %u", level);
            return &level1_defaults;
    }
}

int accel_config_init(struct accel_config *cfg, uint8_t level, int inst) {
    if (!cfg) {
        return -EINVAL;
    }

    // Start with defaults
    const struct accel_config *defaults = accel_config_get_defaults(level);
    memcpy(cfg, defaults, sizeof(struct accel_config));

    // Override with device tree values if present
    #define DT_DRV_COMPAT zmk_input_processor_acceleration
    
    if (level == 1) {
        cfg->sensitivity = ACCEL_CLAMP(
            DT_INST_PROP_OR(inst, sensitivity, cfg->sensitivity),
            MIN_SAFE_SENSITIVITY, MAX_SAFE_SENSITIVITY);
        cfg->max_factor = ACCEL_CLAMP(
            DT_INST_PROP_OR(inst, max_factor, cfg->max_factor),
            1000, MAX_SAFE_FACTOR);
        cfg->curve_type = ACCEL_CLAMP(
            DT_INST_PROP_OR(inst, curve_type, cfg->curve_type),
            0, 2);
        cfg->y_boost = ACCEL_CLAMP(
            DT_INST_PROP_OR(inst, y_boost, cfg->y_boost),
            500, 3000);
        cfg->sensor_dpi = ACCEL_CLAMP(
            DT_INST_PROP_OR(inst, sensor_dpi, cfg->sensor_dpi),
            400, 8000);
    } else if (level == 2) {
        cfg->sensitivity = ACCEL_CLAMP(
            DT_INST_PROP_OR(inst, sensitivity, cfg->sensitivity),
            MIN_SAFE_SENSITIVITY, MAX_SAFE_SENSITIVITY);
        cfg->max_factor = ACCEL_CLAMP(
            DT_INST_PROP_OR(inst, max_factor, cfg->max_factor),
            1000, MAX_SAFE_FACTOR);
        cfg->curve_type = ACCEL_CLAMP(
            DT_INST_PROP_OR(inst, curve_type, cfg->curve_type),
            0, 2);
        cfg->y_boost = ACCEL_CLAMP(
            DT_INST_PROP_OR(inst, y_boost, cfg->y_boost),
            500, 3000);
        cfg->speed_threshold = DT_INST_PROP_OR(inst, speed_threshold, cfg->speed_threshold);
        cfg->speed_max = MAX(ACCEL_CLAMP(
            DT_INST_PROP_OR(inst, speed_max, cfg->speed_max),
            cfg->speed_threshold + 100, MAX_REASONABLE_SPEED),
            cfg->speed_threshold + 100);
        cfg->min_factor = ACCEL_CLAMP(
            DT_INST_PROP_OR(inst, min_factor, cfg->min_factor),
            200, 2000);
        cfg->acceleration_exponent = ACCEL_CLAMP(
            DT_INST_PROP_OR(inst, acceleration_exponent, cfg->acceleration_exponent),
            1, 5);
        cfg->sensor_dpi = ACCEL_CLAMP(
            DT_INST_PROP_OR(inst, sensor_dpi, cfg->sensor_dpi),
            400, 8000);
    }

    // Enhanced safety: Validate configuration after initialization
    int validation_result = accel_validate_config(cfg);
    if (validation_result != 0) {
        LOG_ERR("Configuration validation failed for level %u: %d", level, validation_result);
        // Reset to safe defaults on validation failure
        memcpy(cfg, defaults, sizeof(struct accel_config));
    }

    LOG_DBG("Configuration initialized for level %u", level);
    return validation_result;
}

// =============================================================================
// CONFIGURATION VALIDATION
// =============================================================================

int accel_validate_config(const struct accel_config *cfg) {
    if (!cfg) {
        LOG_ERR("Configuration pointer is NULL");
        return -EINVAL;
    }

    // Enhanced safety: Level validation
    if (cfg->level < 1 || cfg->level > 2) {
        LOG_ERR("Invalid acceleration level: %u (must be 1 or 2)", cfg->level);
        return -EINVAL;
    }

    // Enhanced safety: Sensitivity validation
    if (cfg->sensitivity < MIN_SAFE_SENSITIVITY || cfg->sensitivity > MAX_SAFE_SENSITIVITY) {
        LOG_ERR("Invalid sensitivity: %u (range: %u-%u)", 
                cfg->sensitivity, MIN_SAFE_SENSITIVITY, MAX_SAFE_SENSITIVITY);
        return -EINVAL;
    }

    // Enhanced safety: Max factor validation
    if (cfg->max_factor < SENSITIVITY_SCALE || cfg->max_factor > MAX_SAFE_FACTOR) {
        LOG_ERR("Invalid max_factor: %u (range: %u-%u)", 
                cfg->max_factor, SENSITIVITY_SCALE, MAX_SAFE_FACTOR);
        return -EINVAL;
    }

    // Enhanced safety: Curve type validation
    if (cfg->curve_type > 2) {
        LOG_ERR("Invalid curve_type: %u (range: 0-2)", cfg->curve_type);
        return -EINVAL;
    }

    // Enhanced safety: Y-boost validation
    if (cfg->y_boost < 500 || cfg->y_boost > 3000) {
        LOG_ERR("Invalid y_boost: %u (range: 500-3000)", cfg->y_boost);
        return -EINVAL;
    }

    // Enhanced safety: DPI validation
    if (cfg->sensor_dpi < 400 || cfg->sensor_dpi > MAX_SENSOR_DPI) {
        LOG_ERR("Invalid sensor_dpi: %u (range: 400-%u)", cfg->sensor_dpi, MAX_SENSOR_DPI);
        return -EINVAL;
    }

    // Level-specific validation
    if (cfg->level == 2) {
        // Enhanced safety: Speed threshold validation
        if (cfg->speed_threshold > MAX_REASONABLE_SPEED) {
            LOG_ERR("Invalid speed_threshold: %u (max: %u)", 
                    cfg->speed_threshold, MAX_REASONABLE_SPEED);
            return -EINVAL;
        }

        // Enhanced safety: Speed max validation
        if (cfg->speed_max <= cfg->speed_threshold || cfg->speed_max > MAX_REASONABLE_SPEED) {
            LOG_ERR("Invalid speed_max: %u (must be > %u and <= %u)", 
                    cfg->speed_max, cfg->speed_threshold, MAX_REASONABLE_SPEED);
            return -EINVAL;
        }

        // Enhanced safety: Min factor validation
        if (cfg->min_factor < 200 || cfg->min_factor > 2000) {
            LOG_ERR("Invalid min_factor: %u (range: 200-2000)", cfg->min_factor);
            return -EINVAL;
        }

        // Enhanced safety: Acceleration exponent validation
        if (cfg->acceleration_exponent < 1 || cfg->acceleration_exponent > 5) {
            LOG_ERR("Invalid acceleration_exponent: %u (range: 1-5)", cfg->acceleration_exponent);
            return -EINVAL;
        }
    }

    // Enhanced safety: Level 1 specific validation for conservative settings
    if (cfg->level == 1) {
        // Level 1 should have more conservative limits
        if (cfg->max_factor > 3000) { // Max 3x for Level 1
            LOG_WRN("Level 1 max_factor %u is high, consider reducing for stability", cfg->max_factor);
        }
        
        if (cfg->sensitivity > 2000) { // Conservative sensitivity for Level 1
            LOG_WRN("Level 1 sensitivity %u is high, consider reducing for precision", cfg->sensitivity);
        }
    }

    LOG_DBG("Configuration validation passed for level %u", cfg->level);
    return 0;
}
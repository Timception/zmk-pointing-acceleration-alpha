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

    LOG_DBG("Configuration initialized for level %u", level);
    return 0;
}
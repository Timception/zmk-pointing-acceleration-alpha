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
    .max_factor = 2500,        // 2.5x (more noticeable acceleration)
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

    LOG_INF("Initializing acceleration config with level: %u", level);
    
    // Start with defaults for the specified level
    const struct accel_config *defaults = accel_config_get_defaults(level);
    memcpy(cfg, defaults, sizeof(struct accel_config));
    
    // Set level from configuration
    cfg->level = level;
    
    // NOTE: DTS property handling is now controlled by the main initialization function
    // This allows proper preset vs custom mode handling
    // DTS properties will only be applied in custom mode
    
    LOG_INF("Base configuration initialized: level=%u, max_factor=%u, sensitivity=%u", 
            cfg->level, cfg->max_factor, cfg->sensitivity);
    
    return 0;
}

// Configuration validation is handled in src/validation/accel_validation.c
// to avoid duplicate symbol errors
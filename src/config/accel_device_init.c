// accel_device_init.c - Device instance initialization
// Separated from main.c for better maintainability
//
// Copyright (c) 2024 The ZMK Contributors
// Modifications (c) 2025 NUOVOTAKA
// SPDX-License-Identifier: MIT

#include <zephyr/logging/log.h>
#include <zephyr/devicetree.h>
#include <string.h>
#include "accel_device_init.h"
#include "accel_config.h"
#include "../presets/accel_presets.h"

LOG_MODULE_DECLARE(input_processor_accel);

// =============================================================================
// DEVICE TREE CUSTOM PROPERTIES APPLICATION
// =============================================================================

// DT property application must be done via macros, not functions
// This function is a placeholder for future non-DT custom properties
int accel_apply_dt_custom_properties(struct accel_config *cfg, uint8_t config_level, int inst) {
    if (!cfg) {
        return ACCEL_ERR_INVALID_ARG;
    }

    LOG_INF("Instance %d: Custom configuration applied via macro (Level %u)", inst, config_level);
    
    // Note: DT property access must be done in macros, not functions
    // The actual DT property application is handled in the main.c macro
    
    return 0;
}

// =============================================================================
// CONFIGURATION LOGGING
// =============================================================================

void accel_log_final_config(const struct accel_config *cfg, uint8_t config_level) {
    if (!cfg) {
        return;
    }

    if (config_level == 1) {
        uint16_t y_boost = accel_decode_y_boost(cfg->y_boost_scaled);
        uint16_t sensor_dpi = accel_decode_sensor_dpi(cfg->sensor_dpi_class);
        LOG_INF("Final Level 1 config: sens=%u, max=%u, curve=%u, y_boost=%u, dpi=%u",
                cfg->cfg.level1.sensitivity, cfg->cfg.level1.max_factor, cfg->cfg.level1.curve_type, y_boost, sensor_dpi);
    } else {
        uint16_t y_boost = accel_decode_y_boost(cfg->y_boost_scaled);
        uint16_t sensor_dpi = accel_decode_sensor_dpi(cfg->sensor_dpi_class);
        LOG_INF("Final Level 2 config: sens=%u, max=%u, thresh=%u, max_speed=%u, min=%u, exp=%u, y_boost=%u, dpi=%u",
                cfg->cfg.level2.sensitivity, cfg->cfg.level2.max_factor,
                cfg->cfg.level2.speed_threshold, cfg->cfg.level2.speed_max, cfg->cfg.level2.min_factor, cfg->cfg.level2.acceleration_exponent, y_boost, sensor_dpi);
    }
}

// =============================================================================
// DEVICE INSTANCE INITIALIZATION
// =============================================================================

int accel_device_init_instance(const struct device *dev, int inst) {
    struct accel_config *cfg = (struct accel_config *)dev->config;
    
    if (!cfg) {
        LOG_ERR("Configuration pointer is NULL");
        return ACCEL_ERR_INVALID_ARG;
    }

    LOG_INF("Accel init: instance %d", inst);

    // Determine configuration level from Kconfig
    uint8_t config_level = 1; // Default to Level 1
    if (IS_ENABLED(CONFIG_INPUT_PROCESSOR_ACCEL_LEVEL_STANDARD)) {
        config_level = 2;
    } else if (IS_ENABLED(CONFIG_INPUT_PROCESSOR_ACCEL_LEVEL_SIMPLE)) {
        config_level = 1;
    }
    LOG_INF("Detected configuration level: %u", config_level);

    // Initialize configuration with level and instance
    int ret = accel_config_init(cfg, config_level, inst);
    if (ret < 0) {
        LOG_ERR("Configuration initialization failed: %d", ret);
        return ret;
    }

    // Set input type and codes - these are always the same
    static const uint16_t accel_codes[] = { INPUT_REL_X, INPUT_REL_Y, INPUT_REL_WHEEL, INPUT_REL_HWHEEL };
    cfg->input_type = INPUT_EV_REL;
    cfg->codes = accel_codes;
    cfg->codes_count = ARRAY_SIZE(accel_codes);

    // Check configuration source: Custom DTS vs Preset based on Kconfig
    bool use_custom_config = IS_ENABLED(CONFIG_INPUT_PROCESSOR_ACCEL_PRESET_CUSTOM);

    if (!use_custom_config) {
        LOG_INF("Instance %d: Using PRESET configuration (Kconfig selected)", inst);
        // Apply preset configuration - level is already set correctly
        accel_config_apply_kconfig_preset(cfg);
    }
    // Note: Custom DT properties are applied in the main.c macro context

    // Log final configuration for debugging
    accel_log_final_config(cfg, config_level);

    return 0;
}
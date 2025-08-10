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

int accel_apply_dt_custom_properties(struct accel_config *cfg, uint8_t config_level, int inst) {
    if (!cfg) {
        return -EINVAL;
    }

    LOG_INF("Instance %d: Using CUSTOM configuration (Level %u)", inst, config_level);

    // Apply common DTS properties for both levels
    if (DT_INST_NODE_HAS_PROP(inst, sensitivity)) {
        cfg->sensitivity = ACCEL_CLAMP(DT_INST_PROP(inst, sensitivity), 200, 2000);
        LOG_DBG("Custom sensitivity: %u", cfg->sensitivity);
    }
    if (DT_INST_NODE_HAS_PROP(inst, max_factor)) {
        cfg->max_factor = ACCEL_CLAMP(DT_INST_PROP(inst, max_factor), 1000, 5000);
        LOG_DBG("Custom max_factor: %u", cfg->max_factor);
    }
    if (DT_INST_NODE_HAS_PROP(inst, curve_type)) {
        cfg->curve_type = ACCEL_CLAMP(DT_INST_PROP(inst, curve_type), 0, 2);
        LOG_DBG("Custom curve_type: %u", cfg->curve_type);
    }
    if (DT_INST_NODE_HAS_PROP(inst, y_boost)) {
        cfg->y_boost = ACCEL_CLAMP(DT_INST_PROP(inst, y_boost), 500, 3000);
        LOG_DBG("Custom y_boost: %u", cfg->y_boost);
    }
    if (DT_INST_NODE_HAS_PROP(inst, sensor_dpi)) {
        cfg->sensor_dpi = ACCEL_CLAMP(DT_INST_PROP(inst, sensor_dpi), 400, 8000);
        LOG_DBG("Custom sensor_dpi: %u", cfg->sensor_dpi);
    }

    // Apply Level 2 specific DTS properties only for Standard level
    if (config_level == 2) {
        LOG_DBG("Applying Level 2 (Standard) custom properties");
        if (DT_INST_NODE_HAS_PROP(inst, speed_threshold)) {
            cfg->speed_threshold = ACCEL_CLAMP(DT_INST_PROP(inst, speed_threshold), 100, 2000);
            LOG_DBG("Custom speed_threshold: %u", cfg->speed_threshold);
        }
        if (DT_INST_NODE_HAS_PROP(inst, speed_max)) {
            cfg->speed_max = ACCEL_CLAMP(DT_INST_PROP(inst, speed_max), 1000, 8000);
            LOG_DBG("Custom speed_max: %u", cfg->speed_max);
        }
        if (DT_INST_NODE_HAS_PROP(inst, min_factor)) {
            cfg->min_factor = ACCEL_CLAMP(DT_INST_PROP(inst, min_factor), 200, 1500);
            LOG_DBG("Custom min_factor: %u", cfg->min_factor);
        }
        if (DT_INST_NODE_HAS_PROP(inst, acceleration_exponent)) {
            cfg->acceleration_exponent = ACCEL_CLAMP(DT_INST_PROP(inst, acceleration_exponent), 1, 5);
            LOG_DBG("Custom acceleration_exponent: %u", cfg->acceleration_exponent);
        }
    } else {
        LOG_DBG("Level 1 (Simple): Skipping Level 2 specific properties");
        // Level 1では Level 2専用設定を無視し、警告を出す
        if (DT_INST_NODE_HAS_PROP(inst, speed_threshold) ||
            DT_INST_NODE_HAS_PROP(inst, speed_max) ||
            DT_INST_NODE_HAS_PROP(inst, min_factor) ||
            DT_INST_NODE_HAS_PROP(inst, acceleration_exponent)) {
            LOG_WRN("Level 1 ignoring Level 2 properties (speed_threshold, speed_max, min_factor, acceleration_exponent)");
        }
    }

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
        LOG_INF("Final Level 1 config: sens=%u, max=%u, curve=%u, y_boost=%u, dpi=%u",
                cfg->sensitivity, cfg->max_factor, cfg->curve_type, cfg->y_boost, cfg->sensor_dpi);
    } else {
        LOG_INF("Final Level 2 config: sens=%u, max=%u, curve=%u, y_boost=%u, thresh=%u, max_speed=%u, min=%u, exp=%u, dpi=%u",
                cfg->sensitivity, cfg->max_factor, cfg->curve_type, cfg->y_boost,
                cfg->speed_threshold, cfg->speed_max, cfg->min_factor, cfg->acceleration_exponent, cfg->sensor_dpi);
    }
}

// =============================================================================
// DEVICE INSTANCE INITIALIZATION
// =============================================================================

int accel_device_init_instance(const struct device *dev, int inst) {
    struct accel_config *cfg = (struct accel_config *)dev->config;
    
    if (!cfg) {
        LOG_ERR("Configuration pointer is NULL");
        return -EINVAL;
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

    if (use_custom_config) {
        // Apply DTS custom properties with validation
        ret = accel_apply_dt_custom_properties(cfg, config_level, inst);
        if (ret < 0) {
            LOG_ERR("Failed to apply custom DT properties: %d", ret);
            return ret;
        }
    } else {
        LOG_INF("Instance %d: Using PRESET configuration (Kconfig selected)", inst);
        // Apply preset configuration - level is already set correctly
        accel_config_apply_kconfig_preset(cfg);
    }

    // Log final configuration for debugging
    accel_log_final_config(cfg, config_level);

    return 0;
}
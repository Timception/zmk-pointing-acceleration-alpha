// accel_config.c - Unified configuration management
// Centralizes all configuration logic and reduces complexity

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
    .sensitivity = 1200,
    .max_factor = 2500,
    .curve_type = 1,
    // Unused fields set to safe defaults
    .y_boost = 1000,
    .speed_threshold = 0,
    .speed_max = 1000,
    .min_factor = 1000,
    .acceleration_exponent = 2,
    .y_aspect_scale = 1000,
    .x_aspect_scale = 1000,
    .sensor_dpi = 1600,
    .dpi_multiplier = 1000,
    .target_dpi = 1600,
    .auto_scale_4k = false
};

static const struct accel_config level2_defaults = {
    .level = 2,
    .sensitivity = 1200,
    .max_factor = 3000,
    .curve_type = 1,
    .y_boost = 1200,
    .speed_threshold = 500,
    .speed_max = 3500,
    // Advanced fields set to safe defaults
    .min_factor = 1000,
    .acceleration_exponent = 2,
    .y_aspect_scale = 1000,
    .x_aspect_scale = 1000,
    .sensor_dpi = 1600,
    .dpi_multiplier = 1000,
    .target_dpi = 1600,
    .auto_scale_4k = false
};

static const struct accel_config level3_defaults = {
    .level = 3,
    .sensitivity = 1000,  // Not used in level 3, but set for safety
    .max_factor = 4000,
    .curve_type = 0,      // Not used in level 3
    .y_boost = 1000,      // Not used in level 3
    .speed_threshold = 500,
    .speed_max = 4000,
    .min_factor = 1000,
    .acceleration_exponent = 2,
    .y_aspect_scale = 1500,
    .x_aspect_scale = 1000,
    .sensor_dpi = 1600,
    .dpi_multiplier = 1500,
    .target_dpi = 1600,
    .auto_scale_4k = true
};

// =============================================================================
// CONFIGURATION FUNCTIONS
// =============================================================================

const struct accel_config *accel_config_get_defaults(uint8_t level) {
    switch (level) {
        case 1: return &level1_defaults;
        case 2: return &level2_defaults;
        case 3: return &level3_defaults;
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
        cfg->speed_max = MAX(DT_INST_PROP_OR(inst, speed_max, cfg->speed_max),
                            cfg->speed_threshold + 100);
    } else { // level == 3
        cfg->min_factor = ACCEL_CLAMP(
            DT_INST_PROP_OR(inst, min_factor, cfg->min_factor),
            200, 2000);
        cfg->max_factor = MAX(ACCEL_CLAMP(
            DT_INST_PROP_OR(inst, max_factor, cfg->max_factor),
            1000, MAX_SAFE_FACTOR), cfg->min_factor);
        cfg->speed_threshold = DT_INST_PROP_OR(inst, speed_threshold, cfg->speed_threshold);
        cfg->speed_max = MAX(DT_INST_PROP_OR(inst, speed_max, cfg->speed_max),
                            cfg->speed_threshold + 100);
        cfg->acceleration_exponent = ACCEL_CLAMP(
            DT_INST_PROP_OR(inst, acceleration_exponent, cfg->acceleration_exponent),
            1, 13);
        cfg->y_aspect_scale = ACCEL_CLAMP(
            DT_INST_PROP_OR(inst, y_aspect_scale, cfg->y_aspect_scale),
            500, 3000);
        cfg->x_aspect_scale = ACCEL_CLAMP(
            DT_INST_PROP_OR(inst, x_aspect_scale, cfg->x_aspect_scale),
            500, 3000);
        cfg->sensor_dpi = ACCEL_CLAMP(
            DT_INST_PROP_OR(inst, sensor_dpi, cfg->sensor_dpi),
            400, 8000);
        cfg->dpi_multiplier = ACCEL_CLAMP(
            DT_INST_PROP_OR(inst, dpi_multiplier, cfg->dpi_multiplier),
            500, 3000);
        cfg->target_dpi = ACCEL_CLAMP(
            DT_INST_PROP_OR(inst, target_dpi, cfg->target_dpi),
            400, 8000);
    }

    LOG_DBG("Configuration initialized for level %u", level);
    return 0;
}
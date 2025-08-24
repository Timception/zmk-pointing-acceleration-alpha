// accel_config_adapter.c - Adapter functions for new optimized config structure
// Provides backward compatibility and helper functions
//
// Copyright (c) 2024 The ZMK Contributors
// Modifications (c) 2025 NUOVOTAKA
// SPDX-License-Identifier: MIT

#include <zephyr/logging/log.h>
#include "accel_config.h"
#include "../../include/drivers/input_processor_accel.h"

LOG_MODULE_DECLARE(input_processor_accel);

// =============================================================================
// CONFIGURATION ADAPTER FUNCTIONS
// =============================================================================

uint16_t accel_get_sensitivity(const struct accel_config *cfg) {
    if (!cfg) return 1000;
    
    if (cfg->level == 1) {
        return cfg->cfg.level1.sensitivity;
    } else {
        return cfg->cfg.level2.sensitivity;
    }
}

uint16_t accel_get_max_factor(const struct accel_config *cfg) {
    if (!cfg) return 2000;
    
    if (cfg->level == 1) {
        return cfg->cfg.level1.max_factor;
    } else {
        return cfg->cfg.level2.max_factor;
    }
}

uint16_t accel_get_min_factor(const struct accel_config *cfg) {
    if (!cfg || cfg->level != 2) return 1000;
    return cfg->cfg.level2.min_factor;
}

uint16_t accel_get_speed_threshold(const struct accel_config *cfg) {
    if (!cfg || cfg->level != 2) return 600;
    return cfg->cfg.level2.speed_threshold;
}

uint16_t accel_get_speed_max(const struct accel_config *cfg) {
    if (!cfg || cfg->level != 2) return 3500;
    return cfg->cfg.level2.speed_max;
}

uint8_t accel_get_curve_type(const struct accel_config *cfg) {
    if (!cfg) return 1;
    
    if (cfg->level == 1) {
        return cfg->cfg.level1.curve_type;
    } else {
        return 2; // Level 2 uses exponential curves
    }
}

uint8_t accel_get_acceleration_exponent(const struct accel_config *cfg) {
    if (!cfg || cfg->level != 2) return 2;
    return cfg->cfg.level2.acceleration_exponent;
}

uint16_t accel_get_y_boost(const struct accel_config *cfg) {
    if (!cfg) return 1000;
    return accel_decode_y_boost(cfg->y_boost_scaled);
}

uint16_t accel_get_sensor_dpi(const struct accel_config *cfg) {
    if (!cfg) return 800;
    return accel_decode_sensor_dpi(cfg->sensor_dpi_class);
}

// =============================================================================
// CONFIGURATION SETTER FUNCTIONS
// =============================================================================

void accel_set_sensitivity(struct accel_config *cfg, uint16_t sensitivity) {
    if (!cfg) return;
    
    if (cfg->level == 1) {
        cfg->cfg.level1.sensitivity = sensitivity;
    } else {
        cfg->cfg.level2.sensitivity = sensitivity;
    }
}

void accel_set_max_factor(struct accel_config *cfg, uint16_t max_factor) {
    if (!cfg) return;
    
    if (cfg->level == 1) {
        cfg->cfg.level1.max_factor = max_factor;
    } else {
        cfg->cfg.level2.max_factor = max_factor;
    }
}

void accel_set_y_boost(struct accel_config *cfg, uint16_t y_boost) {
    if (!cfg) return;
    cfg->y_boost_scaled = accel_encode_y_boost(y_boost);
}

void accel_set_sensor_dpi(struct accel_config *cfg, uint16_t sensor_dpi) {
    if (!cfg) return;
    cfg->sensor_dpi_class = accel_encode_sensor_dpi(sensor_dpi);
}
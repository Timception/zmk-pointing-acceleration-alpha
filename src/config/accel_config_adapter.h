/*
 * Copyright (c) 2024 The ZMK Contributors
 * Modifications (c) 2025 NUOVOTAKA
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "../../include/drivers/input_processor_accel.h"

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// CONFIGURATION ADAPTER FUNCTIONS
// =============================================================================

/**
 * @brief Get configuration values from optimized structure
 */
uint16_t accel_get_sensitivity(const struct accel_config *cfg);
uint16_t accel_get_max_factor(const struct accel_config *cfg);
uint16_t accel_get_min_factor(const struct accel_config *cfg);
uint16_t accel_get_speed_threshold(const struct accel_config *cfg);
uint16_t accel_get_speed_max(const struct accel_config *cfg);
uint8_t accel_get_curve_type(const struct accel_config *cfg);
uint8_t accel_get_acceleration_exponent(const struct accel_config *cfg);
uint16_t accel_get_y_boost(const struct accel_config *cfg);
uint16_t accel_get_sensor_dpi(const struct accel_config *cfg);

/**
 * @brief Set configuration values in optimized structure
 */
void accel_set_sensitivity(struct accel_config *cfg, uint16_t sensitivity);
void accel_set_max_factor(struct accel_config *cfg, uint16_t max_factor);
void accel_set_y_boost(struct accel_config *cfg, uint16_t y_boost);
void accel_set_sensor_dpi(struct accel_config *cfg, uint16_t sensor_dpi);

#ifdef __cplusplus
}
#endif
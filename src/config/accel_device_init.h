/*
 * Copyright (c) 2024 The ZMK Contributors
 * Modifications (c) 2025 NUOVOTAKA
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <zephyr/device.h>
#include "../../include/drivers/input_processor_accel.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize device instance configuration
 * @param dev Device pointer
 * @param inst Device tree instance number
 * @return 0 on success, negative error code on failure
 */
int accel_device_init_instance(const struct device *dev, int inst);

/**
 * @brief Apply device tree custom properties to configuration
 * Note: This is a placeholder. Actual DT property access must be done in macros.
 * @param cfg Configuration structure to modify
 * @param config_level Configuration level (1 or 2)
 * @param inst Device tree instance number
 * @return 0 on success, negative error code on failure
 */
int accel_apply_dt_custom_properties(struct accel_config *cfg, uint8_t config_level, int inst);

/**
 * @brief Log final configuration for debugging
 * @param cfg Configuration structure
 * @param config_level Configuration level
 */
void accel_log_final_config(const struct accel_config *cfg, uint8_t config_level);

#ifdef __cplusplus
}
#endif
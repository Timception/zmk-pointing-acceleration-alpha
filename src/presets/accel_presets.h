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

/**
 * @brief Apply preset configuration
 * @param cfg Configuration structure to modify
 * @param preset_name Preset name ("office", "gaming", "high_sens")
 * @return 0 on success, negative error code on failure
 */
int accel_config_apply_preset(struct accel_config *cfg, const char *preset_name);

/**
 * @brief Apply Kconfig preset to configuration
 * @param cfg Configuration structure to modify
 */
void accel_config_apply_kconfig_preset(struct accel_config *cfg);

#ifdef __cplusplus
}
#endif
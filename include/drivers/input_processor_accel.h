/*
 * Copyright (c) 2024 The ZMK Contributors
 * Modifications (c) 2025 NUOVOTAKA
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/input/input.h>
#include <zephyr/sys/util.h>
#include "input_processor.h"



#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// CONSTANTS AND CONFIGURATION
// =============================================================================

// Core safety limits to prevent overflow and system crashes
#define MAX_SAFE_INPUT_VALUE    2000    // Maximum safe input value (increased for trackball support)
#define MAX_SAFE_FACTOR         10000   // Maximum safe acceleration factor
#define MAX_SAFE_SENSITIVITY    5000    // Maximum safe sensitivity
#define MIN_SAFE_SENSITIVITY    100     // Minimum safe sensitivity
#define MAX_REASONABLE_SPEED    50000   // Maximum reasonable speed (counts/sec)

// DPI calculation constants
#define STANDARD_DPI_REFERENCE  800     // Reference DPI for normalization
#define MAX_SENSOR_DPI          8000    // Maximum supported sensor DPI

// Exponential curve calculation constants
#define CURVE_MILD_DIVISOR      2000ULL    // Divisor for mild exponential curve
#define CURVE_MODERATE_QUAD_DIV 1000ULL    // Quadratic divisor for moderate curve
#define CURVE_MODERATE_CUBIC_DIV 3000000ULL // Cubic divisor for moderate curve
#define CURVE_STRONG_QUAD_DIV   800ULL     // Quadratic divisor for strong curve
#define CURVE_STRONG_CUBIC_DIV  2000000ULL // Cubic divisor for strong curve
#define CURVE_AGGRESSIVE_QUAD_DIV 600ULL   // Quadratic divisor for aggressive curve
#define CURVE_AGGRESSIVE_CUBIC_DIV 1500000ULL // Cubic divisor for aggressive curve
#define CURVE_DEFAULT_DIVISOR   1000ULL    // Default curve divisor

// Calculation scaling constants
#define SENSITIVITY_SCALE       1000    // Sensitivity scaling factor
#define SPEED_NORMALIZATION     1000    // Speed normalization factor
#define LINEAR_CURVE_MULTIPLIER 10ULL   // Linear curve multiplication factor (reduced for proper scaling)

// Essential utility macros (optimized for MCU)
#define ACCEL_CLAMP(val, min, max) ((val) < (min) ? (min) : ((val) > (max) ? (max) : (val)))

// Backward compatibility
#ifndef CLAMP
#define CLAMP(val, min, max) ACCEL_CLAMP(val, min, max)
#endif

// =============================================================================
// DATA STRUCTURES
// =============================================================================

/**
 * @brief Acceleration data structure for MCU efficiency
 */
struct accel_data {
    uint32_t last_time_ms;
    uint32_t stable_speed;
    int32_t remainder_x;
    int32_t remainder_y;
    uint32_t last_factor;
};

/**
 * @brief Acceleration configuration structure
 */
struct accel_config {
    uint8_t input_type;
    const uint16_t *codes;
    uint32_t codes_count;
    bool track_remainders;
    uint8_t level;
    
    uint16_t sensitivity;
    uint16_t max_factor;
    uint8_t curve_type;
    uint16_t y_boost;
    uint32_t speed_threshold;
    uint32_t speed_max;
    uint16_t min_factor;
    uint8_t acceleration_exponent;
    uint16_t sensor_dpi;
};

// =============================================================================
// FUNCTION DECLARATIONS
// =============================================================================

/**
 * @brief Validate configuration values at initialization
 * @param cfg Configuration structure to validate
 * @return 0 if valid, negative error code if invalid
 */
int accel_validate_config(const struct accel_config *cfg);

/**
 * @brief Apply Kconfig preset to configuration
 * @param cfg Configuration structure to modify
 */
void accel_config_apply_kconfig_preset(struct accel_config *cfg);

/**
 * @brief Safely clamp input value to prevent overflow
 * @param input_value Raw input value
 * @return Clamped safe input value
 */
static inline int32_t accel_clamp_input_value(int32_t input_value) {
    if (input_value > MAX_SAFE_INPUT_VALUE) {
        return MAX_SAFE_INPUT_VALUE;
    }
    if (input_value < -MAX_SAFE_INPUT_VALUE) {
        return -MAX_SAFE_INPUT_VALUE;
    }
    return input_value;
}

uint32_t accel_safe_quadratic_curve(int32_t abs_input, uint32_t multiplier);
uint32_t accel_calculate_speed(struct accel_data *data, int32_t input_value);

int accel_handle_event(const struct device *dev, struct input_event *event,
                      uint32_t param1, uint32_t param2,
                      struct zmk_input_processor_state *state);

int32_t accel_simple_calculate(const struct accel_config *cfg, int32_t input_value, uint16_t code);
int32_t accel_standard_calculate(const struct accel_config *cfg, struct accel_data *data, 
                                int32_t input_value, uint16_t code);

#ifdef __cplusplus
}
#endif
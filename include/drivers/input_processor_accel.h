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

// Standard input event types and codes (following Linux input subsystem)
// These are already defined in zephyr/dt-bindings/input/input-event-codes.h
// INPUT_EV_REL = 0x02, INPUT_REL_X = 0x00, INPUT_REL_Y = 0x01, etc.

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// CONSTANTS AND CONFIGURATION
// =============================================================================

// Removed - no longer using arrays indexed by code

// Enhanced time measurement constants
#define MIN_TIME_DELTA_US       100     // Minimum time delta in microseconds (0.1ms)
#define MAX_TIME_DELTA_MS       500     // Maximum time delta in milliseconds
// Removed - no longer using speed history
#define SPEED_SCALE_FACTOR      1000000 // Scale factor for microsecond-based speed calculation

// Input value limits to prevent overflow
#define MAX_SAFE_INPUT_VALUE    1000    // Maximum safe input value
#define MAX_SAFE_FACTOR         10000   // Maximum safe acceleration factor
#define MAX_SAFE_SENSITIVITY    5000    // Maximum safe sensitivity
#define MIN_SAFE_SENSITIVITY    100     // Minimum safe sensitivity
#define MAX_REASONABLE_SPEED    50000   // Maximum reasonable speed (counts/sec)

// Validation macros (use custom names to avoid conflicts)
#define ACCEL_CLAMP(val, min, max) ((val) < (min) ? (min) : ((val) > (max) ? (max) : (val)))
#define IS_VALID_RANGE(val, min, max) ((val) >= (min) && (val) <= (max))

// Overflow-safe multiplication macros for MCU
#define ACCEL_SAFE_MUL32(a, b, result) do { \
    int64_t temp = (int64_t)(a) * (int64_t)(b); \
    if (temp > INT32_MAX) temp = INT32_MAX; \
    if (temp < INT32_MIN) temp = INT32_MIN; \
    (result) = (int32_t)temp; \
} while(0)

#define ACCEL_SAFE_MUL16(a, b, result) do { \
    int32_t temp = (int32_t)(a) * (int32_t)(b); \
    if (temp > INT16_MAX) temp = INT16_MAX; \
    if (temp < INT16_MIN) temp = INT16_MIN; \
    (result) = (int16_t)temp; \
} while(0)

// Ensure CLAMP macro is available for backward compatibility
#ifndef CLAMP
#define CLAMP(val, min, max) ACCEL_CLAMP(val, min, max)
#endif

// Thread safety: Use atomic operations for shared data (macros removed - use atomic_* functions directly)

// =============================================================================
// DATA STRUCTURES
// =============================================================================

// Removed - no longer needed for simplified implementation

/**
 * @brief Minimal acceleration data structure for MCU efficiency
 * Total size: ~20 bytes (reduced from ~40 bytes)
 */
struct accel_data {
    // Combined timing and speed data (8 bytes)
    atomic_t last_time_ms;                          // Last event time in milliseconds
    atomic_t stable_speed;                          // Simple smoothed speed
    
    // Remainder tracking for X/Y only (8 bytes)
    atomic_t remainder_x;                           // X-axis remainder
    atomic_t remainder_y;                           // Y-axis remainder
    
    // Last acceleration factor (4 bytes)
    atomic_t last_factor;                           // Last applied factor
    
    // No mutex - using atomic operations only for thread safety
};

/**
 * @brief Unified acceleration configuration structure
 * All fields are present regardless of level - unused fields are set to defaults
 */
struct accel_config {
    uint8_t input_type;
    const uint16_t *codes;
    uint32_t codes_count;
    bool track_remainders;
    uint8_t level;  // Configuration level (1 or 2)
    
    // Core settings (used by all levels)
    uint16_t sensitivity;
    uint16_t max_factor;
    uint8_t curve_type;
    
    // Standard level settings (level 2+)
    uint16_t y_boost;
    uint32_t speed_threshold;
    uint32_t speed_max;
    uint16_t min_factor;
    uint8_t acceleration_exponent;
    
    // DPI setting (available for all levels when using custom configuration)
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
    // Additional safety check for extreme values
    if (input_value > MAX_SAFE_INPUT_VALUE) {
        return MAX_SAFE_INPUT_VALUE;
    }
    if (input_value < -MAX_SAFE_INPUT_VALUE) {
        return -MAX_SAFE_INPUT_VALUE;
    }
    return input_value;
}

/**
 * @brief Safely calculate quadratic curve with overflow protection
 * @param abs_input Absolute input value (already clamped)
 * @param multiplier Curve multiplier
 * @return Safe curve factor
 */
uint32_t accel_safe_quadratic_curve(int32_t abs_input, uint32_t multiplier);

/**
 * @brief Get high-precision timestamp
 * @return Timestamp in microseconds
 */
static inline int64_t accel_get_precise_time_us(void) {
#ifdef CONFIG_TIMER_HAS_64BIT_CYCLE_COUNTER
    // Use high-precision cycle counter if available
    return k_cyc_to_us_floor64(k_cycle_get_64());
#else
    // Fallback to millisecond precision
    return k_uptime_get() * 1000;
#endif
}

// Removed - integrated into enhanced speed calculation

/**
 * @brief Simplified speed calculation for MCU efficiency
 * @param data Acceleration data structure
 * @param input_value Current input value
 * @return Calculated and smoothed speed
 */
uint32_t accel_calculate_speed(struct accel_data *data, int32_t input_value);

/**
 * @brief Main event handler for acceleration processing
 * @param dev Device instance
 * @param event Input event to process
 * @param param1 Additional parameter (unused)
 * @param param2 Additional parameter (unused)
 * @param state Input processor state (unused)
 * @return 0: ZMK_INPUT_PROC_CONTINUE to continue processing, 
 *         1: ZMK_INPUT_PROC_STOP to stop on critical errors or invalid configuration
 * 
 * Returns ZMK_INPUT_PROC_STOP in the following cases:
 * - Critical parameter validation failure
 * - Invalid configuration level
 * - Abnormally large input values (hardware malfunction)
 * - Calculation errors resulting in unsafe values
 */
int accel_handle_event(const struct device *dev, struct input_event *event,
                      uint32_t param1, uint32_t param2,
                      struct zmk_input_processor_state *state);

// Level-specific calculation functions (always available)
int32_t accel_simple_calculate(const struct accel_config *cfg, int32_t input_value, uint16_t code);
int32_t accel_standard_calculate(const struct accel_config *cfg, struct accel_data *data, 
                                int32_t input_value, uint16_t code);

#ifdef __cplusplus
}
#endif
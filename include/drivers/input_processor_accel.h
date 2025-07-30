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
#include <drivers/input_processor.h>

// Standard relative input event codes (following Linux input subsystem)
// These are already included via drivers/input_processor.h -> zephyr/dt-bindings/input/input-event-codes.h
#define INPUT_REL_X 0x00
#define INPUT_REL_Y 0x01

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// CONSTANTS AND CONFIGURATION
// =============================================================================

#define ACCEL_MAX_CODES 4

// Enhanced time measurement constants
#define MIN_TIME_DELTA_US       100     // Minimum time delta in microseconds (0.1ms)
#define MAX_TIME_DELTA_MS       500     // Maximum time delta in milliseconds
#define SPEED_HISTORY_SIZE      4       // Reduced for MCU efficiency
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

// Ensure CLAMP macro is available for backward compatibility
#ifndef CLAMP
#define CLAMP(val, min, max) ACCEL_CLAMP(val, min, max)
#endif

// Thread safety: Use atomic operations for shared data (macros removed - use atomic_* functions directly)

// =============================================================================
// DATA STRUCTURES
// =============================================================================

/**
 * @brief Speed history entry for smoothing calculations
 */
struct speed_sample {
    uint32_t speed;         // Speed in counts per second
    int64_t timestamp_us;   // Timestamp in microseconds
    bool valid;             // Whether this sample is valid
};

/**
 * @brief Enhanced timing data structure
 */
struct timing_data {
    atomic_t last_time_us;                          // Last event time in microseconds
    struct speed_sample speed_history[SPEED_HISTORY_SIZE]; // Speed history for smoothing
    atomic_t history_index;                         // Current history index
    atomic_t stable_speed;                          // Smoothed stable speed
    atomic_t event_count;                           // Total event count for statistics
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

/**
 * @brief Thread-safe acceleration data structure
 */
struct accel_data {
    struct timing_data timing;                      // Enhanced timing data
    atomic_t remainders[ACCEL_MAX_CODES];          // Thread-safe remainders
    atomic_t last_factor;                          // Thread-safe last factor
    struct k_mutex mutex;                          // Mutex for complex operations
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
    return CLAMP(input_value, -MAX_SAFE_INPUT_VALUE, MAX_SAFE_INPUT_VALUE);
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

/**
 * @brief Calculate smoothed speed from history
 * @param timing Timing data structure
 * @param current_speed Current calculated speed
 * @return Smoothed speed value
 */
uint32_t accel_calculate_smoothed_speed(struct timing_data *timing, uint32_t current_speed);

/**
 * @brief Enhanced speed calculation with improved precision
 * @param timing Timing data structure
 * @param input_value Current input value
 * @return Calculated and smoothed speed
 */
uint32_t accel_calculate_enhanced_speed(struct timing_data *timing, int32_t input_value);

// Main event handler
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
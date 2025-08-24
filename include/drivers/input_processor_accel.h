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

// Simplified speed calculation constants
#define ACCEL_MAX_SPEED_SAMPLES     8       // Maximum speed samples for averaging
#define ACCEL_SPEED_SCALE_FACTOR    10      // Speed scaling factor (simpler than 1000)

// Backward compatibility
#ifndef CLAMP
#define CLAMP(val, min, max) ACCEL_CLAMP(val, min, max)
#endif

// =============================================================================
// MEMORY POOL OPTIMIZATION
// =============================================================================

// Memory pool for acceleration data - reduces heap fragmentation
#define ACCEL_MAX_INSTANCES 4

// =============================================================================
// DATA STRUCTURES - ULTRA-OPTIMIZED FOR MCU
// =============================================================================

/**
 * @brief Ultra-compact acceleration data structure - 6 bytes total
 * Memory layout optimized for 32-bit ARM Cortex-M:
 * - 4 bytes: last_time_ms (uint32_t) - aligned to 4-byte boundary
 * - 2 bytes: recent_speed (uint16_t) - packed efficiently
 * Total: 6 bytes (was 8 bytes, 25% reduction)
 */
struct accel_data {
    uint32_t last_time_ms;         // Time tracking for speed calculation
    uint16_t recent_speed;         // Recent speed (16-bit, sufficient for MCU)
    // Removed: speed_samples (not critical for performance)
    // Removed: reserved padding (not needed with 6-byte structure)
} __packed;

// Static memory pool for runtime data - defined after struct declaration
K_MEM_SLAB_DEFINE(accel_data_pool, sizeof(struct accel_data), ACCEL_MAX_INSTANCES, 4);

/**
 * @brief Level-specific configuration union - saves memory
 * Only stores configuration relevant to the active level
 */
union accel_level_config {
    struct {
        uint16_t sensitivity;      // Base sensitivity multiplier
        uint16_t max_factor;       // Maximum acceleration factor
        uint8_t curve_type;        // Acceleration curve type (0-2)
        uint8_t reserved;          // Padding for alignment
    } level1;                      // 6 bytes for Level 1
    
    struct {
        uint16_t sensitivity;      // Base sensitivity multiplier
        uint16_t max_factor;       // Maximum acceleration factor
        uint16_t min_factor;       // Minimum acceleration factor
        uint16_t speed_threshold;  // Speed threshold (16-bit sufficient)
        uint16_t speed_max;        // Speed for maximum acceleration (16-bit)
        uint8_t acceleration_exponent; // Exponential curve exponent
        uint8_t reserved;          // Padding for alignment
    } level2;                      // 10 bytes for Level 2
} __packed;

/**
 * @brief Ultra-optimized acceleration configuration structure
 * Memory layout: 20 bytes total (was ~32 bytes, 37.5% reduction)
 * - 8 bytes: pointer + uint32_t (codes, codes_count)
 * - 10 bytes: union accel_level_config (max size)
 * - 2 bytes: packed fields (y_boost, sensor_dpi as scaled values)
 */
struct accel_config {
    const uint16_t *codes;         // Pointer to codes array
    uint32_t codes_count;          // Number of codes
    union accel_level_config cfg;  // Level-specific configuration
    uint8_t y_boost_scaled;        // Y-axis boost (scaled: 100-300 = 1.0x-3.0x)
    uint8_t sensor_dpi_class;      // DPI class: 0=400, 1=800, 2=1200, 3=1600, 4=3200, 5=6400
    uint8_t input_type;            // Input event type
    uint8_t level;                 // Configuration level (1 or 2)
} __packed;

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
 * @brief Memory pool management functions
 */
struct accel_data *accel_data_alloc(void);
void accel_data_free(struct accel_data *data);

/**
 * @brief Decode scaled configuration values
 */
static inline uint16_t accel_decode_y_boost(uint8_t scaled) {
    return 1000 + (scaled * 10); // 100-300 -> 1000-3000
}

static inline uint16_t accel_decode_sensor_dpi(uint8_t dpi_class) {
    static const uint16_t dpi_table[] = {400, 800, 1200, 1600, 3200, 6400, 8000, 800};
    return (dpi_class < 7) ? dpi_table[dpi_class] : dpi_table[7]; // Default to 800
}

/**
 * @brief Encode configuration values to scaled format
 */
static inline uint8_t accel_encode_y_boost(uint16_t y_boost) {
    if (y_boost < 1000) return 0;
    if (y_boost > 3000) return 200;
    return (y_boost - 1000) / 10; // 1000-3000 -> 0-200
}

static inline uint8_t accel_encode_sensor_dpi(uint16_t sensor_dpi) {
    if (sensor_dpi <= 400) return 0;
    if (sensor_dpi <= 800) return 1;
    if (sensor_dpi <= 1200) return 2;
    if (sensor_dpi <= 1600) return 3;
    if (sensor_dpi <= 3200) return 4;
    if (sensor_dpi <= 6400) return 5;
    return 6; // 8000 or higher
}

/**
 * @brief Safely clamp input value to prevent overflow - optimized for speed
 */
static inline int32_t accel_clamp_input_value(int32_t input_value) {
    // Branchless clamping for better performance on ARM Cortex-M
    int32_t max_val = MAX_SAFE_INPUT_VALUE;
    int32_t min_val = -MAX_SAFE_INPUT_VALUE;
    
    input_value = (input_value > max_val) ? max_val : input_value;
    input_value = (input_value < min_val) ? min_val : input_value;
    return input_value;
}

uint32_t accel_safe_quadratic_curve(int32_t abs_input, uint32_t multiplier);
int32_t accel_safe_fallback_calculate(int32_t input_value, uint32_t max_factor);

int accel_handle_event(const struct device *dev, struct input_event *event,
                      uint32_t param1, uint32_t param2,
                      struct zmk_input_processor_state *state);

// Level-specific calculation functions
int32_t accel_simple_calculate(const struct accel_config *cfg, int32_t input_value, uint16_t code);
int32_t accel_standard_calculate(const struct accel_config *cfg, struct accel_data *data, 
                                int32_t input_value, uint16_t code);

// Common calculation functions (shared between levels)
int64_t safe_multiply_64(int64_t a, int64_t b, int64_t max_result);
int32_t safe_int64_to_int32(int64_t value);
int16_t safe_int32_to_int16(int32_t value);
uint32_t calculate_dpi_adjusted_sensitivity(const struct accel_config *cfg);

#if defined(CONFIG_INPUT_PROCESSOR_ACCEL_LEVEL_STANDARD)
uint32_t calculate_exponential_curve(uint32_t t, uint8_t exponent);
#endif

// Simplified speed calculation functions
uint32_t accel_calculate_simple_speed(struct accel_data *data, int32_t input_value);

#ifdef __cplusplus
}
#endif
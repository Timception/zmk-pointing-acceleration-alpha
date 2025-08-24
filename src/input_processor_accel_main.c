// input_processor_accel_main.c - ZMK Input Processor for Pointing Device Acceleration
// Main event handler and device registration
// 
// Copyright (c) 2024 The ZMK Contributors
// Modifications (c) 2025 NUOVOTAKA
// SPDX-License-Identifier: MIT

#include <zephyr/logging/log.h>
#include <stdlib.h>
#include <string.h>
#include "../include/drivers/input_processor_accel.h"
#include "config/accel_config.h"
#include "config/accel_device_init.h"

LOG_MODULE_REGISTER(input_processor_accel, CONFIG_ZMK_LOG_LEVEL);

#define DT_DRV_COMPAT zmk_input_processor_acceleration

#if DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT)

// =============================================================================
// INTERRUPT PROCESSING OPTIMIZATION
// =============================================================================

// Fast path processing flags - minimize branching in interrupt context
#define ACCEL_FAST_PATH_ENABLED     BIT(0)
#define ACCEL_BYPASS_VALIDATION     BIT(1)
#define ACCEL_SKIP_DEBUG_LOG        BIT(2)

// Pre-computed lookup tables for common calculations (ROM storage)
static const uint16_t dpi_adjustment_table[8] = {
    2000, 1250, 833, 625, 312, 156, 125, 1000  // For DPI classes 0-7
};

static const uint16_t curve_multiplier_table[3] = {
    10, 25, 50  // For curve types 0-2 (Linear, Mild, Strong)
};

// =============================================================================
// DEVICE INITIALIZATION
// =============================================================================

static int accel_init_device(const struct device *dev) {
    const struct accel_config *cfg = dev->config;
    struct accel_data *data = dev->data;

    // Validate final configuration
    int ret = accel_validate_config(cfg);
    if (ret < 0) {
        LOG_ERR("Device %s: Configuration validation failed: %d", dev->name, ret);
        return ret;
    }
    
    // Initialize runtime data structures
    memset(data, 0, sizeof(struct accel_data));
    
    LOG_INF("Device %s: Acceleration processor ready (Level %d)", dev->name, cfg->level);
    return 0;
}

// =============================================================================
// DEVICE INSTANCE CREATION USING DT_INST_FOREACH_STATUS_OKAY
// =============================================================================

// Macro to create device instance initialization function
#define ACCEL_INIT_FUNC(inst)                                                                     \
    static int accel_init_##inst(const struct device *dev) {                                     \
        /* Initialize device instance configuration */                                           \
        int ret = accel_device_init_instance(dev, inst);                                        \
        if (ret < 0) {                                                                           \
            return ret;                                                                          \
        }                                                                                        \
                                                                                                  \
        /* Apply DT custom properties if enabled (must be done in macro context) */            \
        struct accel_config *cfg = (struct accel_config *)dev->config;                          \
        bool use_custom_config = IS_ENABLED(CONFIG_INPUT_PROCESSOR_ACCEL_PRESET_CUSTOM);       \
        if (use_custom_config) {                                                                  \
            /* Apply common DTS properties for both levels */                                   \
            if (cfg->level == 1) { \
                cfg->cfg.level1.sensitivity = ACCEL_CLAMP(DT_INST_PROP_OR(inst, sensitivity, cfg->cfg.level1.sensitivity), 200, 2000); \
                cfg->cfg.level1.max_factor = ACCEL_CLAMP(DT_INST_PROP_OR(inst, max_factor, cfg->cfg.level1.max_factor), 1000, 5000); \
                cfg->cfg.level1.curve_type = ACCEL_CLAMP(DT_INST_PROP_OR(inst, curve_type, cfg->cfg.level1.curve_type), 0, 2); \
            } else { \
                cfg->cfg.level2.sensitivity = ACCEL_CLAMP(DT_INST_PROP_OR(inst, sensitivity, cfg->cfg.level2.sensitivity), 200, 2000); \
                cfg->cfg.level2.max_factor = ACCEL_CLAMP(DT_INST_PROP_OR(inst, max_factor, cfg->cfg.level2.max_factor), 1000, 5000); \
            } \
            cfg->y_boost_scaled = ACCEL_CLAMP((DT_INST_PROP_OR(inst, y_boost, 1000) - 1000) / 10, 0, 200); \
            uint16_t dpi = ACCEL_CLAMP(DT_INST_PROP_OR(inst, sensor_dpi, 800), 400, 8000); \
            cfg->sensor_dpi_class = (dpi <= 400) ? 0 : (dpi <= 800) ? 1 : (dpi <= 1200) ? 2 : (dpi <= 1600) ? 3 : (dpi <= 3200) ? 4 : (dpi <= 6400) ? 5 : 6; \
                                                                                                  \
            /* Apply Level 2 specific DTS properties only for Standard level */                \
            if (cfg->level == 2) {                                                              \
                cfg->cfg.level2.speed_threshold = ACCEL_CLAMP(DT_INST_PROP_OR(inst, speed_threshold, cfg->cfg.level2.speed_threshold), 100, 2000); \
                cfg->cfg.level2.speed_max = ACCEL_CLAMP(DT_INST_PROP_OR(inst, speed_max, cfg->cfg.level2.speed_max), 1000, 8000); \
                cfg->cfg.level2.min_factor = ACCEL_CLAMP(DT_INST_PROP_OR(inst, min_factor, cfg->cfg.level2.min_factor), 200, 1500); \
                cfg->cfg.level2.acceleration_exponent = ACCEL_CLAMP(DT_INST_PROP_OR(inst, acceleration_exponent, cfg->cfg.level2.acceleration_exponent), 1, 5); \
            }                                                                                    \
        }                                                                                        \
                                                                                                  \
        /* Final device initialization and validation */                                        \
        return accel_init_device(dev);                                                          \
    }

// Macro to create device instance data and config structures
#define ACCEL_DEVICE_DEFINE(inst)                                                               \
    static struct accel_data accel_data_##inst = {0};                                          \
    static struct accel_config accel_config_##inst = {0};                                      \
    ACCEL_INIT_FUNC(inst)                                                                      \
    DEVICE_DT_INST_DEFINE(inst,                                                                \
                          accel_init_##inst,                                                    \
                          NULL,                                                                 \
                          &accel_data_##inst,                                                   \
                          &accel_config_##inst,                                                 \
                          POST_KERNEL,                                                          \
                          CONFIG_INPUT_PROCESSOR_ACCELERATION_INIT_PRIORITY,                    \
                          &(const struct zmk_input_processor_driver_api){                      \
                              .handle_event = accel_handle_event                                \
                          });

// Create device instances for all enabled DT nodes
DT_INST_FOREACH_STATUS_OKAY(ACCEL_DEVICE_DEFINE)

// =============================================================================
// MAIN EVENT HANDLER
// =============================================================================

// =============================================================================
// OPTIMIZED FAST-PATH EVENT HANDLER
// =============================================================================

static inline int32_t accel_fast_calculate_level1(const struct accel_config *cfg, 
                                                  int32_t input_value, uint16_t code) {
    // Ultra-fast Level 1 calculation - minimal branching
    uint32_t sensitivity = cfg->cfg.level1.sensitivity;
    uint32_t max_factor = cfg->cfg.level1.max_factor;
    uint8_t curve_type = cfg->cfg.level1.curve_type;
    
    // DPI adjustment using lookup table
    uint16_t dpi_mult = dpi_adjustment_table[cfg->sensor_dpi_class];
    int64_t result = ((int64_t)input_value * sensitivity * dpi_mult) / (SENSITIVITY_SCALE * 1000);
    
    // Fast curve calculation using lookup table
    int32_t abs_input = (input_value < 0) ? -input_value : input_value;
    if (abs_input > 1) {
        uint32_t curve_mult = curve_multiplier_table[curve_type];
        uint32_t curve_factor = SENSITIVITY_SCALE + (abs_input * abs_input * curve_mult) / 100;
        curve_factor = (curve_factor > max_factor) ? max_factor : curve_factor;
        result = (result * curve_factor) / SENSITIVITY_SCALE;
    }
    
    // Y-axis boost
    if (code == INPUT_REL_Y) {
        uint16_t y_boost = accel_decode_y_boost(cfg->y_boost_scaled);
        result = (result * y_boost) / SENSITIVITY_SCALE;
    }
    
    // Clamp result
    return (result > 400) ? 400 : ((result < -400) ? -400 : (int32_t)result);
}

int accel_handle_event(const struct device *dev, struct input_event *event,
                      uint32_t param1, uint32_t param2,
                      struct zmk_input_processor_state *state) {
    // CRITICAL: Minimize interrupt processing time
    // Fast validation with minimal branching
    if (!dev || !event || !dev->config || !dev->data) {
        return 0; // Fast exit for invalid pointers
    }
    
    const struct accel_config *cfg = dev->config;
    struct accel_data *data = dev->data;
    
    // Fast path checks - optimized for common cases
    if (event->type != cfg->input_type || 
        event->value == 0 ||
        (event->code != INPUT_REL_X && event->code != INPUT_REL_Y)) {
        return 0; // Fast exit for non-movement events
    }
    
    // Skip expensive validation in interrupt context
    // (validation done at initialization time)
    
    // Fast input clamping
    int32_t input_value = accel_clamp_input_value(event->value);

    // OPTIMIZED: Fast-path processing with minimal overhead
    int32_t accelerated_value;
    
    // Ultra-fast calculation dispatch - branch prediction optimized
    if (cfg->level == 1) {
        // Level 1: Use optimized fast-path calculation
        accelerated_value = accel_fast_calculate_level1(cfg, input_value, event->code);
    } else {
        // Level 2: Use standard calculation (less common path)
        accelerated_value = accel_standard_calculate(cfg, data, input_value, event->code);
    }
    
    // Minimal safety check - emergency brake only
    if (__builtin_expect(abs(accelerated_value) > 500, 0)) {
        // Unlikely path - extreme values
        accelerated_value = (accelerated_value > 0) ? 400 : -400;
    }
    
    // Minimum movement guarantee - optimized
    if (__builtin_expect(input_value != 0 && accelerated_value == 0, 0)) {
        accelerated_value = (input_value > 0) ? 1 : -1;
    }
    
    // Update event value - single assignment
    event->value = accelerated_value;
    
    return 0;
}

#endif // DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT)
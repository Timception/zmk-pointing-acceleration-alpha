// input_processor_accel_main.c - ZMK Input Processor for Pointing Device Acceleration
// Refactored for better maintainability and modularity
// 
// Copyright (c) 2024 The ZMK Contributors
// Modifications (c) 2025 NUOVOTAKA
// SPDX-License-Identifier: MIT

#include <zephyr/logging/log.h>
#include <stdlib.h>
#include "../include/drivers/input_processor_accel.h"
#include "config/accel_config.h"

LOG_MODULE_REGISTER(input_processor_accel, CONFIG_ZMK_LOG_LEVEL);

#define DT_DRV_COMPAT zmk_input_processor_acceleration
#if DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT)

// =============================================================================
// DEVICE INITIALIZATION
// =============================================================================

// Forward declarations - moved to header
// void accel_config_apply_kconfig_preset(struct accel_config *cfg);

// Simplified device initialization function
static int accel_init_device(const struct device *dev) {
    const struct accel_config *cfg = dev->config;
    struct accel_data *data = dev->data;

    // Validate configuration
    int ret = accel_validate_config(cfg);
    if (ret < 0) {
        LOG_ERR("Configuration validation failed: %d", ret);
        return ret;
    }
    
    // Initialize data structures (optimized for single-threaded MCU)
    data->last_time_ms = 0;
    data->stable_speed = 0;
    data->remainder_x = 0;
    data->remainder_y = 0;
    data->last_factor = 1000;
    
    LOG_INF("Acceleration processor initialized (Level %d)", cfg->level);
    return 0;
}

// =============================================================================
// DEVICE INSTANCE CREATION USING DT_INST_FOREACH_STATUS_OKAY
// =============================================================================

// Common codes array for all instances
static const uint16_t accel_codes[] = { INPUT_REL_X, INPUT_REL_Y, INPUT_REL_WHEEL, INPUT_REL_HWHEEL };

// Macro to create device instance initialization function
#define ACCEL_INIT_FUNC(inst)                                                                     \
    static int accel_init_##inst(const struct device *dev) {                                     \
        struct accel_config *cfg = (struct accel_config *)dev->config;                          \
        LOG_INF("Accel init: instance %d", inst);                                              \
                                                                                                  \
        /* Initialize configuration based on current level */                                    \
        int ret = accel_config_init(cfg, CONFIG_INPUT_PROCESSOR_ACCEL_LEVEL, inst);             \
        if (ret < 0) {                                                                           \
            LOG_ERR("Configuration initialization failed: %d", ret);                            \
            return ret;                                                                          \
        }                                                                                        \
                                                                                                  \
        /* Set input type and codes */                                                           \
        cfg->input_type = INPUT_EV_REL;                                                          \
        cfg->codes = accel_codes;                                                                \
        cfg->codes_count = ARRAY_SIZE(accel_codes);                                             \
        cfg->track_remainders = DT_INST_NODE_HAS_PROP(inst, track_remainders);                  \
                                                                                                  \
        /* Apply Kconfig presets */                                                             \
        accel_config_apply_kconfig_preset(cfg);                                                 \
                                                                                                  \
        /* Log configuration */                                                                  \
        LOG_INF("Accel config: level=%d, max_factor=%d, sensitivity=%d",                       \
                cfg->level, cfg->max_factor, cfg->sensitivity);                                 \
                                                                                                  \
        /* Validate final configuration */                                                      \
        ret = accel_validate_config(cfg);                                                       \
        if (ret < 0) {                                                                          \
            LOG_ERR("Configuration validation failed: %d", ret);                               \
            return ret;                                                                         \
        }                                                                                       \
                                                                                                 \
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

int accel_handle_event(const struct device *dev, struct input_event *event,
                      uint32_t param1, uint32_t param2,
                      struct zmk_input_processor_state *state) {
    const struct accel_config *cfg = dev->config;
    struct accel_data *data = dev->data;

    // Input validation - critical errors should stop processing
    if (!dev || !event || !cfg || !data) {
        LOG_ERR("Critical error: Invalid parameters");
        return 1;
    }

    // Pass through if not the specified type
    if (event->type != cfg->input_type) {
        return 0;
    }

    // Pass through if not the specified code
    bool code_matched = false;
    for (uint32_t i = 0; i < cfg->codes_count; ++i) {
        if (event->code == cfg->codes[i]) {
            code_matched = true;
            break;
        }
    }
    if (!code_matched) {
        return 0;
    }

    // Pass through wheel events as-is
    if (event->code == INPUT_REL_WHEEL || event->code == INPUT_REL_HWHEEL) {
        return 0;
    }

    // Pass through zero values as-is
    if (event->value == 0) {
        return 0;
    }
    
    // Check if acceleration is effectively disabled
    if (cfg->max_factor <= 1000) {
        return 0;
    }

    // Pointing device movement event acceleration processing
    if (event->code == INPUT_REL_X || event->code == INPUT_REL_Y) {
        // Clamp input value to prevent overflow
        int32_t input_value = accel_clamp_input_value(event->value);
        int32_t accelerated_value;

        // Handle extreme input values
        if (abs(event->value) > MAX_SAFE_INPUT_VALUE * 10) {
            LOG_ERR("Abnormal input value %d", event->value);
            return 1;
        } else if (abs(event->value) > MAX_SAFE_INPUT_VALUE) {
            LOG_WRN("Input value %d clamped to %d", event->value, input_value);
        }

        // Call appropriate calculation function based on configuration level
        switch (cfg->level) {
            case 1:
                accelerated_value = accel_simple_calculate(cfg, input_value, event->code);
                break;
            case 2:
                accelerated_value = accel_standard_calculate(cfg, data, input_value, event->code);
                break;
            default:
                LOG_ERR("Invalid configuration level: %u", cfg->level);
                return 1;
        }

        // Check for calculation errors
        if (abs(accelerated_value) > INT16_MAX) {
            LOG_ERR("Calculation error: result %d exceeds safe range", accelerated_value);
            return 1;
        }

        // Final safety check
        accelerated_value = ACCEL_CLAMP(accelerated_value, INT16_MIN, INT16_MAX);

        // Update event value
        event->value = accelerated_value;
        
        // Log only significant changes for debugging
        if (abs(input_value - accelerated_value) > 5) {
            LOG_DBG("Accel: %s %d->%d", event->code == INPUT_REL_X ? "X" : "Y", input_value, accelerated_value);
        }
        
        return 0;
    }

    // Pass through other events as-is
    return 0;
}

#endif // DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT)
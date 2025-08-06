// input_processor_accel_main.c - ZMK Input Processor for Mouse Acceleration
// Refactored for better maintainability and modularity
// 
// Copyright (c) 2024 The ZMK Contributors
// Modifications (c) 2025 NUOVOTAKA
// SPDX-License-Identifier: MIT

#include <zephyr/logging/log.h>
#include <stdlib.h>
#include "../include/drivers/input_processor_accel.h"
#include "config/accel_config.h"

// Debug macro
#define STRINGIFY(x) #x

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
    
    // Initialize data structures (minimal for MCU efficiency)
    atomic_set(&data->last_time_ms, 0);
    atomic_set(&data->stable_speed, 0);
    atomic_set(&data->remainder_x, 0);
    atomic_set(&data->remainder_y, 0);
    atomic_set(&data->last_factor, 1000);
    
    LOG_INF("Acceleration processor initialized (Level %d)", cfg->level);
    return 0;
}

// Device instance creation
static const uint16_t accel_codes_0[] = { INPUT_REL_X, INPUT_REL_Y, INPUT_REL_WHEEL, INPUT_REL_HWHEEL };

static struct accel_config accel_config_0 = {
    .input_type = INPUT_EV_REL,  // Use Zephyr's standard definition (0x02)
    .codes = accel_codes_0,
    .codes_count = 4,
    .track_remainders = DT_INST_NODE_HAS_PROP(0, track_remainders),
};

// Debug: Check compile-time value
#if defined(INPUT_EV_REL)
#pragma message "INPUT_EV_REL is defined as: " STRINGIFY(INPUT_EV_REL)
#else
#pragma message "INPUT_EV_REL is NOT defined"
#endif

static struct accel_data accel_data_0 = {0};

// Device initialization wrapper
static int accel_init_0(const struct device *dev) {
    LOG_INF("*** ACCEL INIT STARTED ***");
    LOG_INF("Initial input_type before config: %d", accel_config_0.input_type);
    
    // Initialize configuration based on current level
    int ret = accel_config_init(&accel_config_0, CONFIG_INPUT_PROCESSOR_ACCEL_LEVEL, 0);
    if (ret < 0) {
        LOG_ERR("Configuration initialization failed: %d", ret);
        return ret;
    }
    

    // Apply Kconfig presets if any (now supports both Level 1 and Level 2)
    accel_config_apply_kconfig_preset(&accel_config_0);
    
    // Log final configuration for debugging
    LOG_INF("Final config: level=%d, input_type=%d, max_factor=%d, sensitivity=%d", 
            accel_config_0.level, accel_config_0.input_type, 
            accel_config_0.max_factor, accel_config_0.sensitivity);
    LOG_INF("Compile-time constants: INPUT_EV_REL=%d, INPUT_REL_X=%d, INPUT_REL_Y=%d", 
            INPUT_EV_REL, INPUT_REL_X, INPUT_REL_Y);
  
    // Validate final configuration
    ret = accel_validate_config(&accel_config_0);
    if (ret < 0) {
        LOG_ERR("Final configuration validation failed: %d", ret);
        return ret;
    }
    
    return accel_init_device(dev);
}

DEVICE_DT_INST_DEFINE(0,
                      accel_init_0,
                      NULL,
                      &accel_data_0,
                      &accel_config_0,
                      POST_KERNEL,
                      CONFIG_INPUT_PROCESSOR_ACCELERATION_INIT_PRIORITY,
                      &(const struct zmk_input_processor_driver_api){
                          .handle_event = accel_handle_event
                      });

// =============================================================================
// MAIN EVENT HANDLER
// =============================================================================

int accel_handle_event(const struct device *dev, struct input_event *event,
                      uint32_t param1, uint32_t param2,
                      struct zmk_input_processor_state *state) {
    const struct accel_config *cfg = dev->config;
    struct accel_data *data = dev->data;

    LOG_DBG("*** ACCEL HANDLER CALLED: type=%d, code=%d, value=%d", event->type, event->code, event->value);
    
    // Debug: First time only
    static bool first_call = true;
    if (first_call) {
        LOG_INF("*** FIRST EVENT: cfg->input_type=%d, INPUT_EV_REL=%d", cfg->input_type, INPUT_EV_REL);
        first_call = false;
    }

    // Input validation - critical errors should stop processing
    if (!dev || !event || !cfg || !data) {
        LOG_ERR("Critical error: Invalid parameters");
        return 1; // Stop processing on critical error
    }

    // Pass through if not the specified type
    if (event->type != cfg->input_type) {
        LOG_DBG("*** TYPE MISMATCH: event->type=%d, cfg->input_type=%d", event->type, cfg->input_type);
        return 0;
    }
    LOG_DBG("*** TYPE MATCHED: type=%d", event->type);

    // Pass through if not the specified code
    bool code_matched = false;
    for (uint32_t i = 0; i < cfg->codes_count; ++i) {
        if (event->code == cfg->codes[i]) {
            code_matched = true;
            break;
        }
    }
    if (!code_matched) {
        LOG_DBG("*** CODE NOT MATCHED: code=%d", event->code);
        return 0;
    }
    LOG_DBG("*** CODE MATCHED: code=%d", event->code);

    // Pass through wheel events as-is
    if (event->code == INPUT_REL_WHEEL || event->code == INPUT_REL_HWHEEL) {
        return 0;
    }

    // Pass through zero values as-is
    if (event->value == 0) {
        LOG_DBG("*** ZERO VALUE SKIP: value=0");
        return 0;
    }
    
    // Check if acceleration is effectively disabled
    LOG_DBG("*** CONFIG CHECK: level=%d, max_factor=%d, sensitivity=%d", cfg->level, cfg->max_factor, cfg->sensitivity);
    if (cfg->max_factor <= 1000) {
        LOG_DBG("*** ACCEL DISABLED: max_factor=%d <= 1000", cfg->max_factor);
        // Acceleration is effectively disabled - pass through unchanged
        return 0;
    }

    // Mouse movement event acceleration processing
    if (event->code == INPUT_REL_X || event->code == INPUT_REL_Y) {
        LOG_DBG("** Accel start: %s=%d", event->code == INPUT_REL_X ? "X" : "Y", event->value);
        
        // Clamp input value to prevent overflow
        int32_t input_value = accel_clamp_input_value(event->value);
        int32_t accelerated_value;

        // Handle extreme input values
        if (abs(event->value) > MAX_SAFE_INPUT_VALUE * 10) {
            // Extremely abnormal input - likely hardware malfunction
            LOG_ERR("Abnormal input value %d - stopping processing", event->value);
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
                // Stop processing on invalid configuration
                return 1;
        }

        // Check for calculation errors (extreme results indicate problems)
        if (abs(accelerated_value) > INT16_MAX) {
            LOG_ERR("Calculation error: result %d exceeds safe range", accelerated_value);
            return 1;
        }

        // Final safety check
        accelerated_value = ACCEL_CLAMP(accelerated_value, INT16_MIN, INT16_MAX);

        // Update event value
        event->value = accelerated_value;
        
        LOG_DBG("** Accel end: %s=%d->%d", event->code == INPUT_REL_X ? "X" : "Y", input_value, accelerated_value);
        return 0;
    }

    // Pass through other events as-is
    return 0;
}

#endif // DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT)
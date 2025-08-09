// input_processor_accel_main.c - ZMK Input Processor for Pointing Device Acceleration
// Refactored for better maintainability and modularity
// 
// Copyright (c) 2024 The ZMK Contributors
// Modifications (c) 2025 NUOVOTAKA
// SPDX-License-Identifier: MIT

#include <zephyr/logging/log.h>
#include <stdlib.h>
#include <string.h>
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


static int accel_init_device(const struct device *dev) {
    const struct accel_config *cfg = dev->config;
    struct accel_data *data = dev->data;

    // Validate configuration
    int ret = accel_validate_config(cfg);
    if (ret < 0) {
        LOG_ERR("Configuration validation failed: %d", ret);
        return ret;
    }
    
    // Initialize simplified data structures
    data->last_time_ms = 0;
    data->recent_speed = 0;
    data->speed_samples = 0;
    
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
        /* Read track_remainders property for compatibility but ignore it */                    \
        bool track_remainders_ignored = DT_INST_NODE_HAS_PROP(inst, track_remainders);         \
        (void)track_remainders_ignored; /* Suppress unused variable warning */                  \
                                                                                                  \
        /* Apply Kconfig presets */                                                             \
        accel_config_apply_kconfig_preset(cfg);                                                 \
                                                                                                  \
        /* CRITICAL: Ensure level remains unchanged after preset application */                \
        cfg->level = CONFIG_INPUT_PROCESSOR_ACCEL_LEVEL;                                        \
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

// Alternative: Direct node reference for &pointer_accel style definitions
#if DT_NODE_EXISTS(DT_ALIAS(pointer_accel)) && DT_NODE_HAS_STATUS(DT_ALIAS(pointer_accel), okay)
#define POINTER_ACCEL_NODE DT_ALIAS(pointer_accel)

static struct accel_data pointer_accel_data = {0};
static struct accel_config pointer_accel_config = {0};

static int pointer_accel_init(const struct device *dev) {
    struct accel_config *cfg = (struct accel_config *)dev->config;
    LOG_INF("Pointer accel init: direct node reference");
    
    // Initialize configuration
    int ret = accel_config_init(cfg, CONFIG_INPUT_PROCESSOR_ACCEL_LEVEL, 0);
    if (ret < 0) {
        LOG_ERR("Configuration initialization failed: %d", ret);
        return ret;
    }
    
    // Set input type and codes
    cfg->input_type = INPUT_EV_REL;
    cfg->codes = accel_codes;
    cfg->codes_count = ARRAY_SIZE(accel_codes);
    /* Read track_remainders property for compatibility but ignore it */
    bool track_remainders_ignored = DT_NODE_HAS_PROP(POINTER_ACCEL_NODE, track_remainders);
    (void)track_remainders_ignored; /* Suppress unused variable warning */
    
    // Apply Kconfig presets
    accel_config_apply_kconfig_preset(cfg);
    
    // CRITICAL: Ensure level remains unchanged after preset application
    cfg->level = CONFIG_INPUT_PROCESSOR_ACCEL_LEVEL;
    
    // Log configuration
    LOG_INF("Pointer accel config: level=%d, max_factor=%d, sensitivity=%d",
            cfg->level, cfg->max_factor, cfg->sensitivity);
    
    // Validate configuration
    ret = accel_validate_config(cfg);
    if (ret < 0) {
        LOG_ERR("Configuration validation failed: %d", ret);
        return ret;
    }
    
    return accel_init_device(dev);
}

DEVICE_DT_DEFINE(POINTER_ACCEL_NODE, pointer_accel_init, NULL,
                 &pointer_accel_data, &pointer_accel_config,
                 POST_KERNEL, CONFIG_INPUT_PROCESSOR_ACCELERATION_INIT_PRIORITY,
                 &(const struct zmk_input_processor_driver_api){
                     .handle_event = accel_handle_event
                 });
#endif

// =============================================================================
// MAIN EVENT HANDLER
// =============================================================================

int accel_handle_event(const struct device *dev, struct input_event *event,
                      uint32_t param1, uint32_t param2,
                      struct zmk_input_processor_state *state) {
    const struct accel_config *cfg = dev->config;
    struct accel_data *data = dev->data;


    if (!dev) {
        LOG_ERR("Critical error: Device pointer is NULL");
        return 0; // Pass through instead of blocking
    }
    if (!event) {
        LOG_ERR("Critical error: Event pointer is NULL");
        return 0; // Pass through instead of blocking
    }
    if (!cfg) {
        LOG_ERR("Critical error: Configuration pointer is NULL for device %s", dev->name);
        return 0; // Pass through instead of blocking
    }
    if (!data) {
        LOG_ERR("Critical error: Data pointer is NULL for device %s", dev->name);
        return 0; // Pass through instead of blocking
    }

    // Configuration sanity check
    if (cfg->level < 1 || cfg->level > 2) {
        LOG_ERR("Invalid configuration level %u for device %s", cfg->level, dev->name);
        return 0; // Pass through instead of blocking
    }
    
    // Initialize data structure only on first use
    if (data && data->last_time_ms == 0) {
        memset(data, 0, sizeof(struct accel_data));
    }

    // Pass through if not the specified type
    if (event->type != cfg->input_type) {
        return 0;
    }

    // Validate codes array before processing
    if (!cfg->codes || cfg->codes_count == 0) {
        LOG_ERR("Invalid codes configuration for device %s", dev->name);
        return 1;
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
        LOG_DBG("Event code %u not in configured codes for device %s", event->code, dev->name);
        return 0;
    }

    // Pass through wheel events as-is
    if (event->code == INPUT_REL_WHEEL || event->code == INPUT_REL_HWHEEL) {
        return 0;
    }

    // Pass through zero values as-is
    if (event->value == 0) {
        LOG_DBG("Zero value event: type=%u code=%u", event->type, event->code);
        return 0;
    }
    
    // Check if acceleration is effectively disabled
    if (cfg->max_factor <= 1000) {
        LOG_DBG("Acceleration disabled (max_factor=%u) for device %s", cfg->max_factor, dev->name);
        return 0;
    }

    // Pointing device movement event acceleration processing
    if (event->code == INPUT_REL_X || event->code == INPUT_REL_Y) {
        // DIAGNOSTIC: Log all input events for debugging cursor freeze
        static uint32_t event_counter = 0;
        event_counter++;
        if ((event_counter % 50) == 0) {
            LOG_INF("DIAG: Event #%u - type=%u code=%u value=%d", 
                    event_counter, event->type, event->code, event->value);
        }
        
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

        // CRITICAL: Emergency brake for extreme cursor jump prevention
        // Allow reasonable acceleration while preventing extreme jumps
        if (abs(accelerated_value) > 500) {
            LOG_ERR("EMERGENCY BRAKE: Accelerated value %d too extreme (input=%d), using conservative fallback", 
                    accelerated_value, input_value);
            // Use conservative scaling instead of minimal values
            accelerated_value = (accelerated_value > 0) ? 
                ACCEL_CLAMP(accelerated_value / 2, 1, 500) : 
                ACCEL_CLAMP(accelerated_value / 2, -500, -1);
        }

        // Check for calculation errors
        if (abs(accelerated_value) > INT16_MAX) {
            LOG_ERR("Calculation error: result %d exceeds safe range, clamping and continuing", accelerated_value);
            accelerated_value = (accelerated_value > 0) ? INT16_MAX : INT16_MIN;
            // Continue processing instead of returning error
        }

        // Enhanced safety checks
        if (abs(accelerated_value) > 32767) {
            LOG_ERR("Accelerated value %d exceeds safe range, clamping", accelerated_value);
            accelerated_value = (accelerated_value > 0) ? 32767 : -32767;
        }
        
        // Final safety check with reasonable limits - allow proper acceleration
        accelerated_value = ACCEL_CLAMP(accelerated_value, -400, 400);
        
        // Sanity check: ensure we don't have zero movement from non-zero input
        if (input_value != 0 && accelerated_value == 0) {
            accelerated_value = (input_value > 0) ? 1 : -1;
            LOG_DBG("Zero acceleration corrected to: %d", accelerated_value);
        }

        // Analysis logging disabled for performance
        // LOG_DBG("Main: input=%d -> final_output=%d (level=%u)", 
        //         input_value, accelerated_value, cfg->level);
        
        // DIAGNOSTIC: Log final output for debugging cursor freeze
        static uint32_t output_counter = 0;
        output_counter++;
        if ((output_counter % 20) == 0 || abs(accelerated_value) > 50) {
            LOG_INF("DIAG: Output #%u - Final value=%d (from input=%d)", 
                    output_counter, accelerated_value, input_value);
        }
        
        // Update event value
        event->value = accelerated_value;
        
        // Production-ready logging (minimal overhead)
        #if CONFIG_ZMK_LOG_LEVEL >= LOG_LEVEL_DBG
        static uint32_t log_counter = 0;
        static int32_t last_input = 0, last_output = 0;
        
        // Only log significant acceleration events or errors
        bool acceleration_applied = (abs(accelerated_value) > abs(input_value) * 1.1);
        bool periodic_log = (log_counter++ % 2000) == 0; // Reduced frequency
        
        if (acceleration_applied || periodic_log) {
            const char* axis = (event->code == INPUT_REL_X) ? "X" : "Y";
            if (acceleration_applied) {
                LOG_DBG("Accel: %s %d->%d (%.1fx)", 
                        axis, input_value, accelerated_value,
                        (float)accelerated_value / (float)input_value);
            }
            last_input = input_value;
            last_output = accelerated_value;
        }
        #endif
        
        return 0;
    }

    // Pass through other events as-is
    return 0;
}

#endif // DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT)
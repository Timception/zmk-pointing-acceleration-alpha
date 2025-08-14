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
// DEVICE INITIALIZATION
// =============================================================================

// Forward declarations - moved to header
// void accel_config_apply_kconfig_preset(struct accel_config *cfg);


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
            cfg->sensitivity = ACCEL_CLAMP(DT_INST_PROP_OR(inst, sensitivity, cfg->sensitivity), 200, 2000); \
            cfg->max_factor = ACCEL_CLAMP(DT_INST_PROP_OR(inst, max_factor, cfg->max_factor), 1000, 5000); \
            cfg->curve_type = ACCEL_CLAMP(DT_INST_PROP_OR(inst, curve_type, cfg->curve_type), 0, 2); \
            cfg->y_boost = ACCEL_CLAMP(DT_INST_PROP_OR(inst, y_boost, cfg->y_boost), 500, 3000); \
            cfg->sensor_dpi = ACCEL_CLAMP(DT_INST_PROP_OR(inst, sensor_dpi, cfg->sensor_dpi), 400, 8000); \
                                                                                                  \
            /* Apply Level 2 specific DTS properties only for Standard level */                \
            if (cfg->level == 2) {                                                              \
                cfg->speed_threshold = ACCEL_CLAMP(DT_INST_PROP_OR(inst, speed_threshold, cfg->speed_threshold), 100, 2000); \
                cfg->speed_max = ACCEL_CLAMP(DT_INST_PROP_OR(inst, speed_max, cfg->speed_max), 1000, 8000); \
                cfg->min_factor = ACCEL_CLAMP(DT_INST_PROP_OR(inst, min_factor, cfg->min_factor), 200, 1500); \
                cfg->acceleration_exponent = ACCEL_CLAMP(DT_INST_PROP_OR(inst, acceleration_exponent, cfg->acceleration_exponent), 1, 5); \
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
        // LOG_DBG("Event code %u not in configured codes for device %s", event->code, dev->name); // DISABLED for production
        return 0;
    }

    // Pass through wheel events as-is
    if (event->code == INPUT_REL_WHEEL || event->code == INPUT_REL_HWHEEL) {
        return 0;
    }

    // Pass through zero values as-is
    if (event->value == 0) {
        // LOG_DBG("Zero value event: type=%u code=%u", event->type, event->code); // DISABLED for production
        return 0;
    }
    
    // Check if acceleration is effectively disabled
    if (cfg->max_factor <= 1000) {
        // LOG_DBG("Acceleration disabled (max_factor=%u) for device %s", cfg->max_factor, dev->name); // DISABLED for production
        return 0;
    }

    // Pointing device movement event acceleration processing
    if (event->code == INPUT_REL_X || event->code == INPUT_REL_Y) {
        // DIAGNOSTIC: Log all input events for debugging cursor freeze (DISABLED)
        static uint32_t event_counter = 0;
        event_counter++;
        // Temporarily disabled to test if logging affects cursor movement
        // if ((event_counter % 50) == 0) {
        //     LOG_INF("DIAG: Event #%u - type=%u code=%u value=%d", 
        //             event_counter, event->type, event->code, event->value);
        // }
        
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
        
        // Enhanced sanity check: intelligent minimum movement guarantee
        if (input_value != 0 && accelerated_value == 0) {
            // Get DPI-adjusted sensitivity for threshold calculation
            uint32_t dpi_adjusted_sensitivity = calculate_dpi_adjusted_sensitivity(cfg);
            int64_t raw_result = (int64_t)input_value * (int64_t)dpi_adjusted_sensitivity;
            
            // Only force movement if the raw calculation was >= 0.5
            if (abs(raw_result) >= SENSITIVITY_SCALE / 2) {
                accelerated_value = (raw_result > 0) ? 1 : -1;
                LOG_DBG("Main: Minimum movement applied - input=%d, raw=%lld -> output=%d", 
                        input_value, raw_result, accelerated_value);
            } else {
                // Micro movement legitimately should be ignored
                accelerated_value = 0;
                LOG_DBG("Main: Micro movement ignored - input=%d, raw=%lld (< 0.5 threshold)", 
                        input_value, raw_result);
            }
        }

        // Enhanced debug logging - controlled by configuration
        #if defined(CONFIG_INPUT_PROCESSOR_ACCEL_DEBUG_LOG)
        static uint32_t debug_log_counter = 0;
        // More frequent logging for debugging: every 10th event
        uint32_t log_frequency = 10;
        
        // Always log significant movements or acceleration changes
        bool significant_movement = (abs(input_value) > 5) || (abs(accelerated_value) != abs(input_value));
        bool periodic_log = ((debug_log_counter++ % log_frequency) == 0);
        
        // Always log configuration on first event (regardless of movement size)
        static bool config_logged = false;
        if (!config_logged) {
            LOG_DBG("=== RUNTIME CONFIG CHECK ===");
            LOG_DBG("Config: L%u sens=%u max=%u curve=%u dpi=%u", 
                    cfg->level, cfg->sensitivity, cfg->max_factor, cfg->curve_type, cfg->sensor_dpi);
            LOG_DBG("Config: y_boost=%u speed_thresh=%u speed_max=%u min_factor=%u", 
                    cfg->y_boost, cfg->speed_threshold, cfg->speed_max, cfg->min_factor);
            LOG_DBG("=== END CONFIG CHECK ===");
            config_logged = true;
        }
        
        if (significant_movement || periodic_log) {
            const char* axis = (event->code == INPUT_REL_X) ? "X" : "Y";
            // Avoid floating point calculation for better performance
            int32_t accel_ratio_x10 = (input_value != 0) ? 
                (accelerated_value * 10) / input_value : 10;
            
            // Emergency debug: Log every significant movement for troubleshooting
            if (significant_movement) {
                LOG_DBG("DEBUG: input=%d, accel=%d, sens=%u, max=%u", 
                        input_value, accelerated_value, cfg->sensitivity, cfg->max_factor);
            }
            
            LOG_DBG("Accel: L%u %s %d->%d (%d.%dx)%s", 
                    cfg->level, axis, input_value, accelerated_value,
                    accel_ratio_x10 / 10, abs(accel_ratio_x10 % 10),
                    significant_movement ? " [SIG]" : "");
        }
        #endif
        
        // DIAGNOSTIC: Log final output for debugging cursor freeze (DISABLED)
        static uint32_t output_counter = 0;
        static int32_t last_output_time = 0;
        output_counter++;
        
        // Get current time for timing analysis
        int32_t current_time = k_uptime_get_32();
        // int32_t time_diff = current_time - last_output_time;  // Unused, commented out
        
        // Temporarily disabled to test if logging affects cursor movement
        // if ((output_counter % 20) == 0 || abs(accelerated_value) > 50 || time_diff > 100) {
        //     LOG_INF("DIAG: Output #%u - Final value=%d (from input=%d) [time_diff=%dms]", 
        //             output_counter, accelerated_value, input_value, time_diff);
        // }
        last_output_time = current_time;
        
        // Update event value
        event->value = accelerated_value;
        
        // Legacy debug logging removed - replaced with configurable version above
        
        return 0;
    }

    // Pass through other events as-is
    return 0;
}

#endif // DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT)
// input_processor_accel.c - ZMK Input Processor for Mouse Acceleration
// Multi-level configuration: Simple, Standard, Advanced
// Enhanced with overflow protection, thread safety, and precise timing
// Main file - core functionality only

#include <zephyr/logging/log.h>
#include <drivers/input_processor_accel.h>

LOG_MODULE_REGISTER(input_processor_accel, CONFIG_ZMK_LOG_LEVEL);

#define DT_DRV_COMPAT zmk_input_processor_acceleration
#if DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT)

// =============================================================================
// CONFIGURATION LEVELS
// =============================================================================

#ifndef CONFIG_INPUT_PROCESSOR_ACCEL_LEVEL
#define CONFIG_INPUT_PROCESSOR_ACCEL_LEVEL 1  // 1=Simple, 2=Standard, 3=Advanced
#endif

// Level 1: Simple configuration defaults
#if CONFIG_INPUT_PROCESSOR_ACCEL_LEVEL == 1

#ifndef CONFIG_INPUT_PROCESSOR_ACCEL_SENSITIVITY
#define CONFIG_INPUT_PROCESSOR_ACCEL_SENSITIVITY 1200
#endif

#ifndef CONFIG_INPUT_PROCESSOR_ACCEL_MAX_FACTOR
#define CONFIG_INPUT_PROCESSOR_ACCEL_MAX_FACTOR 2500
#endif

#ifndef CONFIG_INPUT_PROCESSOR_ACCEL_CURVE_TYPE
#define CONFIG_INPUT_PROCESSOR_ACCEL_CURVE_TYPE 1
#endif

// Preset configurations
#ifdef CONFIG_INPUT_PROCESSOR_ACCEL_PRESET_OFFICE
#undef CONFIG_INPUT_PROCESSOR_ACCEL_SENSITIVITY
#undef CONFIG_INPUT_PROCESSOR_ACCEL_MAX_FACTOR
#undef CONFIG_INPUT_PROCESSOR_ACCEL_CURVE_TYPE
#define CONFIG_INPUT_PROCESSOR_ACCEL_SENSITIVITY 1000
#define CONFIG_INPUT_PROCESSOR_ACCEL_MAX_FACTOR 2000
#define CONFIG_INPUT_PROCESSOR_ACCEL_CURVE_TYPE 0
#endif

#ifdef CONFIG_INPUT_PROCESSOR_ACCEL_PRESET_GAMING
#undef CONFIG_INPUT_PROCESSOR_ACCEL_SENSITIVITY
#undef CONFIG_INPUT_PROCESSOR_ACCEL_MAX_FACTOR
#undef CONFIG_INPUT_PROCESSOR_ACCEL_CURVE_TYPE
#define CONFIG_INPUT_PROCESSOR_ACCEL_SENSITIVITY 1500
#define CONFIG_INPUT_PROCESSOR_ACCEL_MAX_FACTOR 3500
#define CONFIG_INPUT_PROCESSOR_ACCEL_CURVE_TYPE 2
#endif

#ifdef CONFIG_INPUT_PROCESSOR_ACCEL_PRESET_4K
#undef CONFIG_INPUT_PROCESSOR_ACCEL_SENSITIVITY
#undef CONFIG_INPUT_PROCESSOR_ACCEL_MAX_FACTOR
#undef CONFIG_INPUT_PROCESSOR_ACCEL_CURVE_TYPE
#define CONFIG_INPUT_PROCESSOR_ACCEL_SENSITIVITY 1800
#define CONFIG_INPUT_PROCESSOR_ACCEL_MAX_FACTOR 4000
#define CONFIG_INPUT_PROCESSOR_ACCEL_CURVE_TYPE 1
#endif

// Level 2: Standard configuration defaults
#elif CONFIG_INPUT_PROCESSOR_ACCEL_LEVEL == 2

#ifndef CONFIG_INPUT_PROCESSOR_ACCEL_SENSITIVITY
#define CONFIG_INPUT_PROCESSOR_ACCEL_SENSITIVITY 1200
#endif

#ifndef CONFIG_INPUT_PROCESSOR_ACCEL_MAX_FACTOR
#define CONFIG_INPUT_PROCESSOR_ACCEL_MAX_FACTOR 3000
#endif

#ifndef CONFIG_INPUT_PROCESSOR_ACCEL_CURVE_TYPE
#define CONFIG_INPUT_PROCESSOR_ACCEL_CURVE_TYPE 1
#endif

#ifndef CONFIG_INPUT_PROCESSOR_ACCEL_Y_BOOST
#define CONFIG_INPUT_PROCESSOR_ACCEL_Y_BOOST 1200
#endif

#ifndef CONFIG_INPUT_PROCESSOR_ACCEL_SPEED_THRESHOLD
#define CONFIG_INPUT_PROCESSOR_ACCEL_SPEED_THRESHOLD 500
#endif

#ifndef CONFIG_INPUT_PROCESSOR_ACCEL_SPEED_MAX
#define CONFIG_INPUT_PROCESSOR_ACCEL_SPEED_MAX 3500
#endif

// Level 3: Advanced configuration defaults
#else // CONFIG_INPUT_PROCESSOR_ACCEL_LEVEL == 3

#ifndef CONFIG_INPUT_PROCESSOR_ACCEL_Y_ASPECT_SCALE
#define CONFIG_INPUT_PROCESSOR_ACCEL_Y_ASPECT_SCALE 1500
#endif

#ifndef CONFIG_INPUT_PROCESSOR_ACCEL_X_ASPECT_SCALE
#define CONFIG_INPUT_PROCESSOR_ACCEL_X_ASPECT_SCALE 1000
#endif

#ifndef CONFIG_INPUT_PROCESSOR_ACCEL_MIN_FACTOR
#define CONFIG_INPUT_PROCESSOR_ACCEL_MIN_FACTOR 1000
#endif

#ifndef CONFIG_INPUT_PROCESSOR_ACCEL_MAX_FACTOR
#define CONFIG_INPUT_PROCESSOR_ACCEL_MAX_FACTOR 4000
#endif

#ifndef CONFIG_INPUT_PROCESSOR_ACCEL_SPEED_THRESHOLD
#define CONFIG_INPUT_PROCESSOR_ACCEL_SPEED_THRESHOLD 500
#endif

#ifndef CONFIG_INPUT_PROCESSOR_ACCEL_SPEED_MAX
#define CONFIG_INPUT_PROCESSOR_ACCEL_SPEED_MAX 4000
#endif

#ifndef CONFIG_INPUT_PROCESSOR_ACCEL_EXPONENT
#define CONFIG_INPUT_PROCESSOR_ACCEL_EXPONENT 2
#endif

#ifndef CONFIG_INPUT_PROCESSOR_ACCEL_SENSOR_DPI
#define CONFIG_INPUT_PROCESSOR_ACCEL_SENSOR_DPI 1600
#endif

#ifndef CONFIG_INPUT_PROCESSOR_ACCEL_DPI_MULTIPLIER
#define CONFIG_INPUT_PROCESSOR_ACCEL_DPI_MULTIPLIER 1500
#endif

#ifndef CONFIG_INPUT_PROCESSOR_ACCEL_TARGET_DPI
#define CONFIG_INPUT_PROCESSOR_ACCEL_TARGET_DPI 1600
#endif

#ifndef CONFIG_INPUT_PROCESSOR_ACCEL_AUTO_SCALE_4K
#define CONFIG_INPUT_PROCESSOR_ACCEL_AUTO_SCALE_4K 1
#endif

#endif // CONFIG_INPUT_PROCESSOR_ACCEL_LEVEL

// =============================================================================
// DEVICE INITIALIZATION
// =============================================================================

// Configuration initialization macros with validation
#if CONFIG_INPUT_PROCESSOR_ACCEL_LEVEL == 1
#define ACCEL_CONFIG_INIT(inst) \
    .sensitivity = CLAMP(DT_INST_PROP_OR(inst, sensitivity, CONFIG_INPUT_PROCESSOR_ACCEL_SENSITIVITY), \
                        MIN_SAFE_SENSITIVITY, MAX_SAFE_SENSITIVITY), \
    .max_factor = CLAMP(DT_INST_PROP_OR(inst, max_factor, CONFIG_INPUT_PROCESSOR_ACCEL_MAX_FACTOR), \
                       1000, MAX_SAFE_FACTOR), \
    .curve_type = CLAMP(DT_INST_PROP_OR(inst, curve_type, CONFIG_INPUT_PROCESSOR_ACCEL_CURVE_TYPE), \
                       0, 2)

#elif CONFIG_INPUT_PROCESSOR_ACCEL_LEVEL == 2
#define ACCEL_CONFIG_INIT(inst) \
    .sensitivity = CLAMP(DT_INST_PROP_OR(inst, sensitivity, CONFIG_INPUT_PROCESSOR_ACCEL_SENSITIVITY), \
                        MIN_SAFE_SENSITIVITY, MAX_SAFE_SENSITIVITY), \
    .max_factor = CLAMP(DT_INST_PROP_OR(inst, max_factor, CONFIG_INPUT_PROCESSOR_ACCEL_MAX_FACTOR), \
                       1000, MAX_SAFE_FACTOR), \
    .curve_type = CLAMP(DT_INST_PROP_OR(inst, curve_type, CONFIG_INPUT_PROCESSOR_ACCEL_CURVE_TYPE), \
                       0, 2), \
    .y_boost = CLAMP(DT_INST_PROP_OR(inst, y_boost, CONFIG_INPUT_PROCESSOR_ACCEL_Y_BOOST), \
                    500, 3000), \
    .speed_threshold = DT_INST_PROP_OR(inst, speed_threshold, CONFIG_INPUT_PROCESSOR_ACCEL_SPEED_THRESHOLD), \
    .speed_max = MAX(DT_INST_PROP_OR(inst, speed_max, CONFIG_INPUT_PROCESSOR_ACCEL_SPEED_MAX), \
                    DT_INST_PROP_OR(inst, speed_threshold, CONFIG_INPUT_PROCESSOR_ACCEL_SPEED_THRESHOLD) + 100)

#else // Level 3
#define ACCEL_CONFIG_INIT(inst) \
    .min_factor = CLAMP(DT_INST_PROP_OR(inst, min_factor, CONFIG_INPUT_PROCESSOR_ACCEL_MIN_FACTOR), \
                       200, 2000), \
    .max_factor = MAX(CLAMP(DT_INST_PROP_OR(inst, max_factor, CONFIG_INPUT_PROCESSOR_ACCEL_MAX_FACTOR), \
                           1000, MAX_SAFE_FACTOR), \
                     DT_INST_PROP_OR(inst, min_factor, CONFIG_INPUT_PROCESSOR_ACCEL_MIN_FACTOR)), \
    .speed_threshold = DT_INST_PROP_OR(inst, speed_threshold, CONFIG_INPUT_PROCESSOR_ACCEL_SPEED_THRESHOLD), \
    .speed_max = MAX(DT_INST_PROP_OR(inst, speed_max, CONFIG_INPUT_PROCESSOR_ACCEL_SPEED_MAX), \
                    DT_INST_PROP_OR(inst, speed_threshold, CONFIG_INPUT_PROCESSOR_ACCEL_SPEED_THRESHOLD) + 100), \
    .acceleration_exponent = CLAMP(DT_INST_PROP_OR(inst, acceleration_exponent, CONFIG_INPUT_PROCESSOR_ACCEL_EXPONENT), \
                                  1, 13), \
    .y_aspect_scale = CLAMP(DT_INST_PROP_OR(inst, y_aspect_scale, CONFIG_INPUT_PROCESSOR_ACCEL_Y_ASPECT_SCALE), \
                           500, 3000), \
    .x_aspect_scale = CLAMP(DT_INST_PROP_OR(inst, x_aspect_scale, CONFIG_INPUT_PROCESSOR_ACCEL_X_ASPECT_SCALE), \
                           500, 3000), \
    .sensor_dpi = CLAMP(DT_INST_PROP_OR(inst, sensor_dpi, CONFIG_INPUT_PROCESSOR_ACCEL_SENSOR_DPI), \
                       400, 8000), \
    .dpi_multiplier = CLAMP(DT_INST_PROP_OR(inst, dpi_multiplier, CONFIG_INPUT_PROCESSOR_ACCEL_DPI_MULTIPLIER), \
                           500, 3000), \
    .target_dpi = CLAMP(DT_INST_PROP_OR(inst, target_dpi, CONFIG_INPUT_PROCESSOR_ACCEL_TARGET_DPI), \
                       400, 8000)
#endif

// Device instance initialization macro
#define ACCEL_INST_INIT(inst)                                                  \
static const uint16_t accel_codes_##inst[] = { INPUT_REL_X, INPUT_REL_Y, INPUT_REL_WHEEL, INPUT_REL_HWHEEL };     \
static const struct accel_config accel_config_##inst = {                       \
    .input_type = INPUT_EV_REL,                                                \
    .codes = accel_codes_##inst,                                               \
    .codes_count = 4,                                                          \
    .track_remainders = DT_INST_NODE_HAS_PROP(inst, track_remainders),         \
    ACCEL_CONFIG_INIT(inst)                                                    \
};                                                                             \
static struct accel_data accel_data_##inst = {0};                              \
static int accel_init_##inst(const struct device *dev) {                       \
    const struct accel_config *cfg = dev->config;                             \
    struct accel_data *data = dev->data;                                       \
    int ret = accel_validate_config(cfg);                                      \
    if (ret < 0) {                                                             \
        LOG_ERR("Configuration validation failed: %d", ret);                   \
        return ret;                                                            \
    }                                                                          \
    k_mutex_init(&data->mutex);                                                \
    atomic_set(&data->timing.last_time_us, 0);                                 \
    atomic_set(&data->timing.history_index, 0);                               \
    atomic_set(&data->timing.stable_speed, 0);                                \
    atomic_set(&data->timing.event_count, 0);                                 \
    atomic_set(&data->last_factor, 1000);                                      \
    for (int i = 0; i < ACCEL_MAX_CODES; i++) {                               \
        atomic_set(&data->remainders[i], 0);                                   \
    }                                                                          \
    for (int i = 0; i < SPEED_HISTORY_SIZE; i++) {                            \
        data->timing.speed_history[i].valid = false;                          \
    }                                                                          \
    LOG_INF("Enhanced acceleration processor initialized (Level %d)", CONFIG_INPUT_PROCESSOR_ACCEL_LEVEL); \
    return 0;                                                                  \
}                                                                              \
DEVICE_DT_INST_DEFINE(inst,                                                    \
                      accel_init_##inst,                                       \
                      NULL,                                                    \
                      &accel_data_##inst,                                      \
                      &accel_config_##inst,                                    \
                      POST_KERNEL,                                             \
                      CONFIG_INPUT_PROCESSOR_ACCELERATION_INIT_PRIORITY,       \
                      &(const struct zmk_input_processor_driver_api){          \
                          .handle_event = accel_handle_event                   \
                      });

// Generate device instance
ACCEL_INST_INIT(0)

// =============================================================================
// MAIN EVENT HANDLER
// =============================================================================

static int accel_handle_event(const struct device *dev, struct input_event *event,
                             uint32_t param1, uint32_t param2,
                             struct zmk_input_processor_state *state) {
    const struct accel_config *cfg = dev->config;
    struct accel_data *data = dev->data;

    // Input validation
    if (!dev || !event || !cfg || !data) {
        LOG_ERR("Invalid parameters");
        return -EINVAL;
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

    // Mouse movement event acceleration processing
    if (event->code == INPUT_REL_X || event->code == INPUT_REL_Y) {
        // Clamp input value to prevent overflow
        int32_t input_value = accel_clamp_input_value(event->value);
        int32_t accelerated_value;

        // Log extreme input values
        if (abs(event->value) > MAX_SAFE_INPUT_VALUE) {
            LOG_WRN("Input value %d clamped to %d", event->value, input_value);
        }

        // Call appropriate calculation function based on level
#if CONFIG_INPUT_PROCESSOR_ACCEL_LEVEL == 1
        accelerated_value = accel_simple_calculate(cfg, input_value, event->code);
#elif CONFIG_INPUT_PROCESSOR_ACCEL_LEVEL == 2
        accelerated_value = accel_standard_calculate(cfg, data, input_value, event->code);
#else
        accelerated_value = accel_advanced_calculate(cfg, data, input_value, event->code);
#endif

        // Final safety check
        accelerated_value = CLAMP(accelerated_value, INT16_MIN, INT16_MAX);

        // Update event value
        event->value = accelerated_value;
        
        return 0;
    }

    // Pass through other events as-is
    return 0;
}

#endif // DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT)
// input_processor_accel.c

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/input/input.h>
#include <zephyr/logging/log.h>
#include <math.h>
#include <stdlib.h>
#include <drivers/input_processor.h>

LOG_MODULE_REGISTER(input_processor_accel, CONFIG_ZMK_LOG_LEVEL);

#define DT_DRV_COMPAT zmk_input_processor_acceleration
#if DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT)

#define ACCEL_MAX_CODES 4



#ifndef CONFIG_INPUT_PROCESSOR_ACCEL_Y_ASPECT_SCALE
#define CONFIG_INPUT_PROCESSOR_ACCEL_Y_ASPECT_SCALE 1200  // Increase Y-axis sensitivity (1000 = 1.0x)
#endif

#ifndef CONFIG_INPUT_PROCESSOR_ACCEL_X_ASPECT_SCALE
#define CONFIG_INPUT_PROCESSOR_ACCEL_X_ASPECT_SCALE 1000  // X-axis sensitivity (1000 = 1.0x)
#endif

#ifndef CONFIG_INPUT_PROCESSOR_ACCEL_MIN_FACTOR
#define CONFIG_INPUT_PROCESSOR_ACCEL_MIN_FACTOR 1000  // 1.0x (base sensitivity)
#endif

#ifndef CONFIG_INPUT_PROCESSOR_ACCEL_MAX_FACTOR
#define CONFIG_INPUT_PROCESSOR_ACCEL_MAX_FACTOR 2500  // 2.5x (maximum acceleration)
#endif

#ifndef CONFIG_INPUT_PROCESSOR_ACCEL_SPEED_THRESHOLD
#define CONFIG_INPUT_PROCESSOR_ACCEL_SPEED_THRESHOLD 500  // Acceleration start threshold
#endif

#ifndef CONFIG_INPUT_PROCESSOR_ACCEL_SPEED_MAX
#define CONFIG_INPUT_PROCESSOR_ACCEL_SPEED_MAX 4000  // Maximum acceleration speed
#endif

#ifndef CONFIG_INPUT_PROCESSOR_ACCEL_EXPONENT
#define CONFIG_INPUT_PROCESSOR_ACCEL_EXPONENT 2
#endif

#ifndef CONFIG_INPUT_PROCESSOR_ACCEL_SENSOR_DPI
#define CONFIG_INPUT_PROCESSOR_ACCEL_SENSOR_DPI 1600  // Physical sensor DPI
#endif

#ifndef CONFIG_INPUT_PROCESSOR_ACCEL_DPI_MULTIPLIER
#define CONFIG_INPUT_PROCESSOR_ACCEL_DPI_MULTIPLIER 1000  // DPI multiplier (1000 = 1.0x)
#endif

#ifndef CONFIG_INPUT_PROCESSOR_ACCEL_TARGET_DPI
#define CONFIG_INPUT_PROCESSOR_ACCEL_TARGET_DPI 800  // Target DPI (reference for sensitivity adjustment)
#endif

struct accel_config {
    uint8_t input_type;
    const uint16_t *codes;
    uint32_t codes_count;
    bool track_remainders;
    uint16_t min_factor;
    uint16_t max_factor;
    uint32_t speed_threshold;
    uint32_t speed_max;
    uint8_t  acceleration_exponent;
    uint16_t y_aspect_scale;  // Y-axis aspect ratio adjustment
    uint16_t x_aspect_scale;  // X-axis aspect ratio adjustment
    uint16_t sensor_dpi;
    uint16_t dpi_multiplier;  // DPI multiplier setting
    uint16_t target_dpi;      // Target DPI
};

struct accel_data {
    int64_t last_time;
    int16_t remainders[ACCEL_MAX_CODES];
    uint16_t last_factor; // Record the last acceleration factor
};

static int accel_handle_event(const struct device *dev, struct input_event *event,
                             uint32_t param1, uint32_t param2,
                             struct zmk_input_processor_state *state);

#define ACCEL_INST_INIT(inst)                                                  \
static const uint16_t accel_codes_##inst[] = { INPUT_REL_X, INPUT_REL_Y, INPUT_REL_WHEEL, INPUT_REL_HWHEEL };     \
static const struct accel_config accel_config_##inst = {                       \
    .input_type = INPUT_EV_REL,                                                \
    .codes = accel_codes_##inst,                                               \
    .codes_count = 4,                                                          \
    .track_remainders = DT_INST_PROP_OR(inst, track_remainders, true),         \
    .min_factor = DT_INST_PROP_OR(inst, min_factor, CONFIG_INPUT_PROCESSOR_ACCEL_MIN_FACTOR),                     \
    .max_factor = DT_INST_PROP_OR(inst, max_factor, CONFIG_INPUT_PROCESSOR_ACCEL_MAX_FACTOR),                     \
    .speed_threshold = DT_INST_PROP_OR(inst, speed_threshold, CONFIG_INPUT_PROCESSOR_ACCEL_SPEED_THRESHOLD),            \
    .speed_max = DT_INST_PROP_OR(inst, speed_max, CONFIG_INPUT_PROCESSOR_ACCEL_SPEED_MAX),                       \
    .acceleration_exponent = DT_INST_PROP_OR(inst, acceleration_exponent, CONFIG_INPUT_PROCESSOR_ACCEL_EXPONENT),  \
    .y_aspect_scale = DT_INST_PROP_OR(inst, y_aspect_scale, CONFIG_INPUT_PROCESSOR_ACCEL_Y_ASPECT_SCALE), \
    .x_aspect_scale = DT_INST_PROP_OR(inst, x_aspect_scale, CONFIG_INPUT_PROCESSOR_ACCEL_X_ASPECT_SCALE), \
    .sensor_dpi = DT_INST_PROP_OR(inst, sensor_dpi, CONFIG_INPUT_PROCESSOR_ACCEL_SENSOR_DPI), \
    .dpi_multiplier = DT_INST_PROP_OR(inst, dpi_multiplier, CONFIG_INPUT_PROCESSOR_ACCEL_DPI_MULTIPLIER), \
    .target_dpi = DT_INST_PROP_OR(inst, target_dpi, CONFIG_INPUT_PROCESSOR_ACCEL_TARGET_DPI) \
};                                                                             \
static struct accel_data accel_data_##inst = {0};                              \
static int accel_init_##inst(const struct device *dev) {                       \
    const struct accel_config *cfg = dev->config;                             \
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

// Generate only one instance
ACCEL_INST_INIT(0)

static int accel_handle_event(const struct device *dev, struct input_event *event,
                             uint32_t param1, uint32_t param2,
                             struct zmk_input_processor_state *state) {
    const struct accel_config *cfg = dev->config;
    struct accel_data *data = dev->data;

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

    // Pass through zero values as-is (important: zero values are meaningful)
    if (event->value == 0) {
        return 0;
    }


    // Mouse movement event acceleration processing
    if (event->code == INPUT_REL_X || event->code == INPUT_REL_Y) {
        int64_t current_time = k_uptime_get();
        int32_t input_value = event->value;
        
        // Time delta for speed calculation
        int64_t time_delta = current_time - data->last_time;
        if (time_delta <= 0) time_delta = 1;
        if (time_delta > 100) time_delta = 100;
        
        // Speed calculation (single-axis based)
        uint32_t speed = (abs(input_value) * 1000) / time_delta;
        
        // Acceleration factor calculation
        uint16_t factor = cfg->min_factor;
        
        if (speed > cfg->speed_threshold) {
            if (speed >= cfg->speed_max) {
                factor = cfg->max_factor;
            } else {
                uint32_t speed_range = cfg->speed_max - cfg->speed_threshold;
                uint32_t speed_offset = speed - cfg->speed_threshold;
                uint32_t t_int = (speed_offset * 1000) / speed_range;

                // Apply acceleration curve based on exponent
                uint32_t curve;
                switch (cfg->acceleration_exponent) {
                    case 1:
                        // Linear curve: f(t) = t
                        curve = t_int;
                        break;
                    case 2:
                        // Exponential curve (mild): f(t) = e^(2*t) - 1
                        // Approximation using Taylor series: e^x ≈ 1 + x + x²/2 + x³/6
                        {
                            uint32_t x = (t_int * 2000) / 1000; // 2*t scaled by 1000
                            uint32_t x2 = (x * x) / 1000;
                            uint32_t x3 = (x2 * x) / 1000;
                            curve = x + (x2 / 2) + (x3 / 6);
                            if (curve > 1000) curve = 1000; // Clamp to prevent overflow
                        }
                        break;
                    case 3:
                        // Exponential curve (moderate): f(t) = e^(3*t) - 1
                        {
                            uint32_t x = (t_int * 3000) / 1000; // 3*t scaled by 1000
                            uint32_t x2 = (x * x) / 1000;
                            uint32_t x3 = (x2 * x) / 1000;
                            curve = x + (x2 / 2) + (x3 / 6);
                            if (curve > 1000) curve = 1000; // Clamp to prevent overflow
                        }
                        break;
                    case 4:
                        // Exponential curve (strong): f(t) = e^(4*t) - 1
                        {
                            uint32_t x = (t_int * 4000) / 1000; // 4*t scaled by 1000
                            uint32_t x2 = (x * x) / 1000;
                            uint32_t x3 = (x2 * x) / 1000;
                            curve = x + (x2 / 2) + (x3 / 6);
                            if (curve > 1000) curve = 1000; // Clamp to prevent overflow
                        }
                        break;
                    case 5:
                        // Exponential curve (aggressive): f(t) = e^(5*t) - 1
                        {
                            uint32_t x = (t_int * 5000) / 1000; // 5*t scaled by 1000
                            uint32_t x2 = (x * x) / 1000;
                            uint32_t x3 = (x2 * x) / 1000;
                            curve = x + (x2 / 2) + (x3 / 6);
                            if (curve > 1000) curve = 1000; // Clamp to prevent overflow
                        }
                        break;
                    case 10:
                        // Quadratic curve: f(t) = t^2
                        curve = (t_int * t_int) / 1000;
                        break;
                    case 11:
                        // Cubic curve: f(t) = t^3
                        curve = (t_int * t_int) / 1000;
                        curve = (curve * t_int) / 1000;
                        break;
                    case 12:
                        // Quartic curve: f(t) = t^4
                        curve = (t_int * t_int) / 1000;
                        curve = (curve * t_int) / 1000;
                        curve = (curve * t_int) / 1000;
                        break;
                    case 13:
                        // Quintic curve: f(t) = t^5
                        curve = (t_int * t_int) / 1000;
                        curve = (curve * t_int) / 1000;
                        curve = (curve * t_int) / 1000;
                        curve = (curve * t_int) / 1000;
                        break;
                    default:
                        // Default to mild exponential curve
                        {
                            uint32_t x = (t_int * 2000) / 1000; // 2*t scaled by 1000
                            uint32_t x2 = (x * x) / 1000;
                            uint32_t x3 = (x2 * x) / 1000;
                            curve = x + (x2 / 2) + (x3 / 6);
                            if (curve > 1000) curve = 1000; // Clamp to prevent overflow
                        }
                        break;
                }
                factor = cfg->min_factor + (uint16_t)(((cfg->max_factor - cfg->min_factor) * curve) / 1000);
                if (factor > cfg->max_factor) factor = cfg->max_factor;
            }
        }

        // DPI adjustment factor
        uint32_t dpi_factor = ((uint32_t)cfg->target_dpi * cfg->dpi_multiplier) / cfg->sensor_dpi;
        
        // Aspect ratio adjustment
        uint16_t aspect_scale = (event->code == INPUT_REL_X) ? cfg->x_aspect_scale : cfg->y_aspect_scale;
        
        // Precise calculation
        int64_t precise_value = ((int64_t)input_value * factor * dpi_factor * aspect_scale);
        int32_t accelerated_value = precise_value / (1000 * 1000 * 1000);
        
        // Remainder processing
        if (cfg->track_remainders) {
            uint8_t remainder_idx = (event->code == INPUT_REL_X) ? 0 : 1;
            int32_t remainder = (precise_value % (1000 * 1000 * 1000)) / (1000 * 1000);
            
            data->remainders[remainder_idx] += remainder;
            
            if (abs(data->remainders[remainder_idx]) >= 1000) {
                int32_t carry = data->remainders[remainder_idx] / 1000;
                accelerated_value += carry;
                data->remainders[remainder_idx] -= carry * 1000;
            }
        }
        
        // Minimum movement guarantee
        if (input_value != 0 && accelerated_value == 0) {
            accelerated_value = (input_value > 0) ? 1 : -1;
        }

        // Update event
        event->value = accelerated_value;
        data->last_time = current_time;
        data->last_factor = factor;
        
        // Continue processing
        return 0;
    }

    // Pass through other events as-is
    return 0;
}

#endif // DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT)

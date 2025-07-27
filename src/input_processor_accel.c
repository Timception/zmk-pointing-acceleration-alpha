// input_processor_accel.c

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/input/input.h>
#include <zephyr/sys/printk.h>
#include <zephyr/logging/log.h>
#include <math.h>
#include <stdlib.h>
#include <drivers/input_processor.h>

LOG_MODULE_REGISTER(input_processor_accel, CONFIG_ZMK_LOG_LEVEL);

#define DT_DRV_COMPAT zmk_input_processor_acceleration
#if DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT)

#define ACCEL_MAX_CODES 4

#ifndef CONFIG_INPUT_PROCESSOR_ACCEL_PAIR_WINDOW_MS
#define CONFIG_INPUT_PROCESSOR_ACCEL_PAIR_WINDOW_MS 8
#endif

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
#define CONFIG_INPUT_PROCESSOR_ACCEL_SPEED_MAX 2000  // Maximum acceleration speed
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

typedef enum {
    ACCEL_CURVE_LINEAR = 0,
    ACCEL_CURVE_EXPONENTIAL = 1,
} accel_curve_t;

struct accel_config {
    uint8_t input_type;
    const uint16_t *codes;
    uint32_t codes_count;
    bool track_remainders;
    accel_curve_t curve_type;   // 0: linear, 1: exponential
    uint16_t min_factor;
    uint16_t max_factor;
    uint32_t speed_threshold;
    uint32_t speed_max;
    uint8_t  acceleration_exponent;
    uint8_t  pair_window_ms;
    uint16_t y_aspect_scale;  // Y-axis aspect ratio adjustment
    uint16_t x_aspect_scale;  // X-axis aspect ratio adjustment
    uint16_t sensor_dpi;
    uint16_t dpi_multiplier;  // DPI multiplier setting
    uint16_t target_dpi;      // Target DPI
};

struct accel_data {
    int64_t last_time;
    int16_t remainders[ACCEL_MAX_CODES];
    
    // Vector buffer system
    int32_t vector_x;  // Accumulated X-axis movement
    int32_t vector_y;  // Accumulated Y-axis movement
    int64_t last_flush_time; // Last time the vector was output
    
    uint16_t last_factor; // Record the last acceleration factor
};

static int accel_handle_event(const struct device *dev, struct input_event *event,
                             uint32_t param1, uint32_t param2,
                             struct zmk_input_processor_state *state) {
                                return 1;
                             }

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
    .pair_window_ms = DT_INST_PROP_OR(inst, pair_window_ms, CONFIG_INPUT_PROCESSOR_ACCEL_PAIR_WINDOW_MS), \
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

// Example: Generate only one instance
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
    
    // Check for code match
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

    // Forward if value is 0 (important: 0 value is meaningful)
    if (event->value == 0) {
        return 0;
    }

    // Pass through wheel events as is
    if (event->code == INPUT_REL_WHEEL || event->code == INPUT_REL_HWHEEL) {
        return 0;
    }

    // Add basic acceleration processing
    if (event->code == INPUT_REL_X || event->code == INPUT_REL_Y) {
        int64_t current_time = k_uptime_get();
        
        // Accumulate in vector buffer
        if (event->code == INPUT_REL_X) {
            data->vector_x += event->value;
        } else if (event->code == INPUT_REL_Y) {
            data->vector_y += event->value;
        }
        
        // Flush check (after a certain time or enough movement is accumulated)
        bool should_flush = false;
        int64_t time_since_flush = current_time - data->last_flush_time;
        
        if (time_since_flush >= cfg->pair_window_ms ||
            abs(data->vector_x) + abs(data->vector_y) >= 3) {
            should_flush = true;
        }
        
        if (!should_flush) {
            // If not flushing yet, accumulate event and exit
            return 1;
        }

        // Execute vector processing
        int32_t dx = data->vector_x;
        int32_t dy = data->vector_y;
        
        // Clear vector buffer
        data->vector_x = 0;
        data->vector_y = 0;
        data->last_flush_time = current_time;
        
        // Do nothing if there is no movement
        if (dx == 0 && dy == 0) {
            return 1;
        }
        
        // Calculate speed (vector-based)
        int64_t time_delta = current_time - data->last_time;
        if (time_delta <= 0) time_delta = 1;
        if (time_delta > 100) time_delta = 100;
        
        uint32_t magnitude = (uint32_t)sqrtf((float)dx * dx + (float)dy * dy);
        uint32_t speed = (magnitude * 1000) / time_delta;
        
        uint16_t factor = cfg->min_factor;
        
        if (speed > cfg->speed_threshold) {
            if (speed >= cfg->speed_max) {
                factor = cfg->max_factor;
            } else {
                uint32_t speed_range = cfg->speed_max - cfg->speed_threshold;
                uint32_t speed_offset = speed - cfg->speed_threshold;
                uint32_t t_int = (speed_offset * 1000) / speed_range;

                uint32_t curve = t_int;
                switch (cfg->acceleration_exponent) {
                    case 1:
                        break;
                    case 2:
                        curve = (curve * t_int) / 1000;
                        break;
                    case 3:
                        curve = (curve * t_int) / 1000;
                        curve = (curve * t_int) / 1000;
                        break;
                    case 4:
                        curve = (curve * t_int) / 1000;
                        curve = (curve * t_int) / 1000;
                        curve = (curve * t_int) / 1000;
                        break;
                    case 5:
                        curve = (curve * t_int) / 1000;
                        curve = (curve * t_int) / 1000;
                        curve = (curve * t_int) / 1000;
                        curve = (curve * t_int) / 1000;
                        break;
                    default:
                        curve = (curve * t_int) / 1000;
                        curve = (curve * t_int) / 1000;
                        curve = (curve * t_int) / 1000;
                        curve = (curve * t_int) / 1000;
                        break;
                }
                factor = cfg->min_factor + (uint16_t)(((cfg->max_factor - cfg->min_factor) * curve) / 1000);
                if (factor > cfg->max_factor) factor = cfg->max_factor;
            }
        }

        uint32_t dpi_factor = ((uint32_t)cfg->target_dpi * cfg->dpi_multiplier) / cfg->sensor_dpi;
        
        int64_t precise_x = ((int64_t)dx * factor * dpi_factor * cfg->x_aspect_scale);
        int64_t precise_y = ((int64_t)dy * factor * dpi_factor * cfg->y_aspect_scale);
        
        int32_t accelerated_x = precise_x / (1000 * 1000 * 1000);
        int32_t accelerated_y = precise_y / (1000 * 1000 * 1000);
        
        if (cfg->track_remainders) {
            int32_t remainder_x = (precise_x % (1000 * 1000 * 1000)) / (1000 * 1000);
            int32_t remainder_y = (precise_y % (1000 * 1000 * 1000)) / (1000 * 1000);
            
            data->remainders[0] += remainder_x;
            data->remainders[1] += remainder_y;
            
            if (abs(data->remainders[0]) >= 1000) {
                int32_t carry = data->remainders[0] / 1000;
                accelerated_x += carry;
                data->remainders[0] -= carry * 1000;
            }
            
            if (abs(data->remainders[1]) >= 1000) {
                int32_t carry = data->remainders[1] / 1000;
                accelerated_y += carry;
                data->remainders[1] -= carry * 1000;
            }
        }
        
        if (dx != 0 && accelerated_x == 0) {
            accelerated_x = (dx > 0) ? 1 : -1;
        }
        if (dy != 0 && accelerated_y == 0) {
            accelerated_y = (dy > 0) ? 1 : -1;
        }

        // X軸イベント
        if (accelerated_x != 0) {
            event->code = INPUT_REL_X;
            event->value = accelerated_x;
            event->sync = (accelerated_y == 0); // Yがなければsync
            // ここでreturnすることで、ZMKに加速済みイベントを渡す
            data->last_time = current_time;
            data->last_factor = factor;
            return 1;
        }
        // Y軸イベント
        if (accelerated_y != 0) {
            event->code = INPUT_REL_Y;
            event->value = accelerated_y;
            event->sync = true;
            data->last_time = current_time;
            data->last_factor = factor;
            return 1;
        }
        // どちらも0なら何も送らない
        return 1;
    }

    // Pass through other events as is
    return 0;
}

#endif // DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT)

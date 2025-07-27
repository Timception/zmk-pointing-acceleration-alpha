// input_processor_accel.c - ZMK Input Processor for Mouse Acceleration
// Multi-level configuration: Simple, Standard, Advanced

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

// =============================================================================
// CONFIGURATION LEVELS: Choose your complexity level
// =============================================================================

// Level selection (only one should be defined)
#ifndef CONFIG_INPUT_PROCESSOR_ACCEL_LEVEL
#define CONFIG_INPUT_PROCESSOR_ACCEL_LEVEL 1  // 1=Simple, 2=Standard, 3=Advanced
#endif

// =============================================================================
// LEVEL 1: SIMPLE (Recommended for most users)
// =============================================================================
#if CONFIG_INPUT_PROCESSOR_ACCEL_LEVEL == 1

#ifndef CONFIG_INPUT_PROCESSOR_ACCEL_SENSITIVITY
#define CONFIG_INPUT_PROCESSOR_ACCEL_SENSITIVITY 1200  // Base sensitivity (1000 = 1.0x)
#endif

#ifndef CONFIG_INPUT_PROCESSOR_ACCEL_MAX_FACTOR
#define CONFIG_INPUT_PROCESSOR_ACCEL_MAX_FACTOR 2500  // Maximum acceleration (2500 = 2.5x)
#endif

#ifndef CONFIG_INPUT_PROCESSOR_ACCEL_CURVE_TYPE
#define CONFIG_INPUT_PROCESSOR_ACCEL_CURVE_TYPE 1  // 0=linear, 1=mild, 2=strong
#endif

// Preset-based configuration
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

// =============================================================================
// LEVEL 2: STANDARD (Balanced features)
// =============================================================================
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
#define CONFIG_INPUT_PROCESSOR_ACCEL_Y_BOOST 1200  // Y-axis sensitivity boost (1000 = same as X)
#endif

#ifndef CONFIG_INPUT_PROCESSOR_ACCEL_SPEED_THRESHOLD
#define CONFIG_INPUT_PROCESSOR_ACCEL_SPEED_THRESHOLD 500
#endif

#ifndef CONFIG_INPUT_PROCESSOR_ACCEL_SPEED_MAX
#define CONFIG_INPUT_PROCESSOR_ACCEL_SPEED_MAX 3500
#endif

// =============================================================================
// LEVEL 3: ADVANCED (All features)
// =============================================================================
#elif CONFIG_INPUT_PROCESSOR_ACCEL_LEVEL == 3

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
// CONFIGURATION STRUCTURES
// =============================================================================

struct accel_config {
    uint8_t input_type;
    const uint16_t *codes;
    uint32_t codes_count;
    bool track_remainders;
    
#if CONFIG_INPUT_PROCESSOR_ACCEL_LEVEL == 1
    // Simple configuration
    uint16_t sensitivity;
    uint16_t max_factor;
    uint8_t curve_type;
#elif CONFIG_INPUT_PROCESSOR_ACCEL_LEVEL == 2
    // Standard configuration
    uint16_t sensitivity;
    uint16_t max_factor;
    uint8_t curve_type;
    uint16_t y_boost;
    uint32_t speed_threshold;
    uint32_t speed_max;
#else
    // Advanced configuration (Level 3)
    uint16_t min_factor;
    uint16_t max_factor;
    uint32_t speed_threshold;
    uint32_t speed_max;
    uint8_t  acceleration_exponent;
    uint16_t y_aspect_scale;
    uint16_t x_aspect_scale;
    uint16_t sensor_dpi;
    uint16_t dpi_multiplier;
    uint16_t target_dpi;
#endif
};

struct accel_data {
    int64_t last_time;
    int16_t remainders[ACCEL_MAX_CODES];
    uint16_t last_factor;
};

// =============================================================================
// FUNCTION DECLARATIONS
// =============================================================================

static int accel_handle_event(const struct device *dev, struct input_event *event,
                             uint32_t param1, uint32_t param2,
                             struct zmk_input_processor_state *state);

#if CONFIG_INPUT_PROCESSOR_ACCEL_LEVEL == 1
static int32_t accel_simple_calculate(const struct accel_config *cfg, int32_t input_value, uint16_t code);
#elif CONFIG_INPUT_PROCESSOR_ACCEL_LEVEL == 2
static int32_t accel_standard_calculate(const struct accel_config *cfg, struct accel_data *data, 
                                       int32_t input_value, uint16_t code);
#else
static int32_t accel_advanced_calculate(const struct accel_config *cfg, struct accel_data *data, 
                                       int32_t input_value, uint16_t code);
#endif

// =============================================================================
// DEVICE INITIALIZATION
// =============================================================================

// Configuration initialization based on level
#if CONFIG_INPUT_PROCESSOR_ACCEL_LEVEL == 1
#define ACCEL_CONFIG_INIT(inst) \
    .sensitivity = DT_INST_PROP_OR(inst, sensitivity, CONFIG_INPUT_PROCESSOR_ACCEL_SENSITIVITY), \
    .max_factor = DT_INST_PROP_OR(inst, max_factor, CONFIG_INPUT_PROCESSOR_ACCEL_MAX_FACTOR), \
    .curve_type = DT_INST_PROP_OR(inst, curve_type, CONFIG_INPUT_PROCESSOR_ACCEL_CURVE_TYPE)

#elif CONFIG_INPUT_PROCESSOR_ACCEL_LEVEL == 2
#define ACCEL_CONFIG_INIT(inst) \
    .sensitivity = DT_INST_PROP_OR(inst, sensitivity, CONFIG_INPUT_PROCESSOR_ACCEL_SENSITIVITY), \
    .max_factor = DT_INST_PROP_OR(inst, max_factor, CONFIG_INPUT_PROCESSOR_ACCEL_MAX_FACTOR), \
    .curve_type = DT_INST_PROP_OR(inst, curve_type, CONFIG_INPUT_PROCESSOR_ACCEL_CURVE_TYPE), \
    .y_boost = DT_INST_PROP_OR(inst, y_boost, CONFIG_INPUT_PROCESSOR_ACCEL_Y_BOOST), \
    .speed_threshold = DT_INST_PROP_OR(inst, speed_threshold, CONFIG_INPUT_PROCESSOR_ACCEL_SPEED_THRESHOLD), \
    .speed_max = DT_INST_PROP_OR(inst, speed_max, CONFIG_INPUT_PROCESSOR_ACCEL_SPEED_MAX)

#else // Level 3
#define ACCEL_CONFIG_INIT(inst) \
    .min_factor = DT_INST_PROP_OR(inst, min_factor, CONFIG_INPUT_PROCESSOR_ACCEL_MIN_FACTOR), \
    .max_factor = DT_INST_PROP_OR(inst, max_factor, CONFIG_INPUT_PROCESSOR_ACCEL_MAX_FACTOR), \
    .speed_threshold = DT_INST_PROP_OR(inst, speed_threshold, CONFIG_INPUT_PROCESSOR_ACCEL_SPEED_THRESHOLD), \
    .speed_max = DT_INST_PROP_OR(inst, speed_max, CONFIG_INPUT_PROCESSOR_ACCEL_SPEED_MAX), \
    .acceleration_exponent = DT_INST_PROP_OR(inst, acceleration_exponent, CONFIG_INPUT_PROCESSOR_ACCEL_EXPONENT), \
    .y_aspect_scale = DT_INST_PROP_OR(inst, y_aspect_scale, CONFIG_INPUT_PROCESSOR_ACCEL_Y_ASPECT_SCALE), \
    .x_aspect_scale = DT_INST_PROP_OR(inst, x_aspect_scale, CONFIG_INPUT_PROCESSOR_ACCEL_X_ASPECT_SCALE), \
    .sensor_dpi = DT_INST_PROP_OR(inst, sensor_dpi, CONFIG_INPUT_PROCESSOR_ACCEL_SENSOR_DPI), \
    .dpi_multiplier = DT_INST_PROP_OR(inst, dpi_multiplier, CONFIG_INPUT_PROCESSOR_ACCEL_DPI_MULTIPLIER), \
    .target_dpi = DT_INST_PROP_OR(inst, target_dpi, CONFIG_INPUT_PROCESSOR_ACCEL_TARGET_DPI)
#endif

#define ACCEL_INST_INIT(inst)                                                  \
static const uint16_t accel_codes_##inst[] = { INPUT_REL_X, INPUT_REL_Y, INPUT_REL_WHEEL, INPUT_REL_HWHEEL };     \
static const struct accel_config accel_config_##inst = {                       \
    .input_type = INPUT_EV_REL,                                                \
    .codes = accel_codes_##inst,                                               \
    .codes_count = 4,                                                          \
    .track_remainders = DT_INST_PROP_OR(inst, track_remainders, true),         \
    ACCEL_CONFIG_INIT(inst)                                                    \
};                                                                             \
static struct accel_data accel_data_##inst = {0};                              \
static int accel_init_##inst(const struct device *dev) {                       \
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

// =============================================================================
// MAIN EVENT HANDLER
// =============================================================================

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

    // Pass through zero values as-is
    if (event->value == 0) {
        return 0;
    }

    // Mouse movement event acceleration processing
    if (event->code == INPUT_REL_X || event->code == INPUT_REL_Y) {
        int32_t input_value = event->value;
        int32_t accelerated_value;

#if CONFIG_INPUT_PROCESSOR_ACCEL_LEVEL == 1
        // SIMPLE LEVEL: Basic acceleration with presets
        accelerated_value = accel_simple_calculate(cfg, input_value, event->code);
        
#elif CONFIG_INPUT_PROCESSOR_ACCEL_LEVEL == 2
        // STANDARD LEVEL: Speed-based acceleration with Y-boost
        accelerated_value = accel_standard_calculate(cfg, data, input_value, event->code);
        
#else
        // ADVANCED LEVEL: Full-featured acceleration
        accelerated_value = accel_advanced_calculate(cfg, data, input_value, event->code);
#endif

        // Update event value
        event->value = accelerated_value;
        data->last_time = k_uptime_get();
        
        return 0;
    }

    // Pass through other events as-is
    return 0;
}

// =============================================================================
// LEVEL-SPECIFIC CALCULATION FUNCTIONS
// =============================================================================

#if CONFIG_INPUT_PROCESSOR_ACCEL_LEVEL == 1
// Simple acceleration: Just apply sensitivity and basic curve
static int32_t accel_simple_calculate(const struct accel_config *cfg, int32_t input_value, uint16_t code) {
    // Apply base sensitivity
    int64_t result = (int64_t)input_value * cfg->sensitivity / 1000;
    
    // Apply simple curve based on input magnitude
    int32_t abs_input = abs(input_value);
    if (abs_input > 1) {
        uint32_t curve_factor = 1000;
        
        switch (cfg->curve_type) {
            case 0: // Linear
                curve_factor = 1000 + (abs_input * 100); // Gradual increase
                break;
            case 1: // Mild
                curve_factor = 1000 + (abs_input * abs_input * 10); // Quadratic
                break;
            case 2: // Strong
                curve_factor = 1000 + (abs_input * abs_input * 20); // Stronger quadratic
                break;
        }
        
        if (curve_factor > cfg->max_factor) curve_factor = cfg->max_factor;
        result = (result * curve_factor) / 1000;
    }
    
    // Clamp result
    if (result > 32767) result = 32767;
    if (result < -32767) result = -32767;
    
    return (int32_t)result;
}

#elif CONFIG_INPUT_PROCESSOR_ACCEL_LEVEL == 2
// Standard acceleration: Speed-based with Y-axis boost
static int32_t accel_standard_calculate(const struct accel_config *cfg, struct accel_data *data, 
                                       int32_t input_value, uint16_t code) {
    int64_t current_time = k_uptime_get();
    
    // Time delta for speed calculation
    int64_t time_delta = current_time - data->last_time;
    if (time_delta <= 0) time_delta = 1;
    if (time_delta > 100) time_delta = 100;
    
    // Speed calculation
    uint32_t speed = (abs(input_value) * 1000) / time_delta;
    
    // Base sensitivity
    int64_t result = (int64_t)input_value * cfg->sensitivity / 1000;
    
    // Speed-based acceleration
    if (speed > cfg->speed_threshold) {
        uint32_t factor = 1000;
        if (speed >= cfg->speed_max) {
            factor = cfg->max_factor;
        } else {
            uint32_t speed_range = cfg->speed_max - cfg->speed_threshold;
            uint32_t speed_offset = speed - cfg->speed_threshold;
            uint32_t t = (speed_offset * 1000) / speed_range;
            
            // Apply curve
            uint32_t curve;
            switch (cfg->curve_type) {
                case 0: curve = t; break; // Linear
                case 1: curve = (t * t) / 1000; break; // Quadratic
                case 2: curve = (t * t * t) / (1000 * 1000); break; // Cubic
                default: curve = (t * t) / 1000; break;
            }
            
            factor = 1000 + (((cfg->max_factor - 1000) * curve) / 1000);
        }
        
        result = (result * factor) / 1000;
    }
    
    // Y-axis boost
    if (code == INPUT_REL_Y) {
        result = (result * cfg->y_boost) / 1000;
    }
    
    // Clamp result
    if (result > 32767) result = 32767;
    if (result < -32767) result = -32767;
    
    return (int32_t)result;
}

#else
// Advanced acceleration: Full original logic
static int32_t accel_advanced_calculate(const struct accel_config *cfg, struct accel_data *data, 
                                       int32_t input_value, uint16_t code) {
    int64_t current_time = k_uptime_get();
    
    // Time delta for speed calculation
    int64_t time_delta = current_time - data->last_time;
    if (time_delta <= 0) time_delta = 1;
    if (time_delta > 100) time_delta = 100;
    
    // Speed calculation
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
                    curve = t_int;
                    break;
                case 2:
                    {
                        if (t_int > 500) t_int = 500;
                        uint32_t x = (t_int * 2000) / 1000;
                        uint32_t x2 = (x * x) / 1000;
                        uint32_t x3 = (x2 * x) / 1000;
                        curve = x + (x2 / 2) + (x3 / 6);
                        if (curve > 1000) curve = 1000;
                    }
                    break;
                case 10:
                    curve = (t_int * t_int) / 1000;
                    break;
                default:
                    curve = (t_int * t_int) / 1000;
                    break;
            }
            factor = cfg->min_factor + (uint16_t)(((cfg->max_factor - cfg->min_factor) * curve) / 1000);
            if (factor > cfg->max_factor) factor = cfg->max_factor;
        }
    }

    // DPI adjustment factor
    uint32_t dpi_factor = ((uint32_t)cfg->target_dpi * cfg->dpi_multiplier) / cfg->sensor_dpi;
    
    // Auto-scale for high-resolution displays
    #if CONFIG_INPUT_PROCESSOR_ACCEL_AUTO_SCALE_4K
    if (dpi_factor < 1000) {
        dpi_factor = (dpi_factor * 1500) / 1000;
    }
    if (dpi_factor > 5000) dpi_factor = 5000;
    #endif
    
    // Aspect ratio adjustment with Y-axis acceleration boost
    uint16_t aspect_scale = (code == INPUT_REL_X) ? cfg->x_aspect_scale : cfg->y_aspect_scale;
    
    // Additional Y-axis acceleration boost
    if (code == INPUT_REL_Y && factor > cfg->min_factor) {
        uint16_t y_boost = (factor - cfg->min_factor) * 200 / (cfg->max_factor - cfg->min_factor);
        aspect_scale = (aspect_scale * (1000 + y_boost)) / 1000;
    }
    
    // Precise calculation
    int64_t precise_value = ((int64_t)input_value * factor * dpi_factor * aspect_scale);
    int32_t accelerated_value = precise_value / (1000 * 1000 * 1000);
    
    // Clamp to prevent extreme values
    if (accelerated_value > 32767) accelerated_value = 32767;
    if (accelerated_value < -32767) accelerated_value = -32767;
    
    // Remainder processing
    if (cfg->track_remainders) {
        uint8_t remainder_idx = (code == INPUT_REL_X) ? 0 : 1;
        int32_t remainder = (precise_value % (1000 * 1000 * 1000)) / (1000 * 1000);
        
        if (remainder > 10000) remainder = 10000;
        if (remainder < -10000) remainder = -10000;
        
        int32_t new_remainder = data->remainders[remainder_idx] + remainder;
        if (new_remainder > 32767) {
            data->remainders[remainder_idx] = 32767;
        } else if (new_remainder < -32767) {
            data->remainders[remainder_idx] = -32767;
        } else {
            data->remainders[remainder_idx] = new_remainder;
        }
        
        if (abs(data->remainders[remainder_idx]) >= 1000) {
            int32_t carry = data->remainders[remainder_idx] / 1000;
            if (carry > 10) carry = 10;
            if (carry < -10) carry = -10;
            
            accelerated_value += carry;
            data->remainders[remainder_idx] -= carry * 1000;
        }
    }
    
    // Minimum movement guarantee
    if (input_value != 0 && accelerated_value == 0) {
        accelerated_value = (input_value > 0) ? 1 : -1;
    }
    
    // Smooth large jumps
    if (data->last_factor > 0) {
        int32_t factor_diff = abs((int32_t)factor - (int32_t)data->last_factor);
        if (factor_diff > 500) {
            uint16_t smooth_factor = data->last_factor + ((factor > data->last_factor) ? 250 : -250);
            int64_t smooth_value = ((int64_t)input_value * smooth_factor * dpi_factor * aspect_scale);
            int32_t smooth_accelerated = smooth_value / (1000 * 1000 * 1000);
            
            if (abs(smooth_accelerated - accelerated_value) > abs(accelerated_value) / 4) {
                accelerated_value = smooth_accelerated;
                factor = smooth_factor;
            }
        }
    }
    
    data->last_factor = factor;
    return accelerated_value;
}
#endif

#endif // DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT)
// accel_presets.c - Preset configuration management
// Centralizes all preset definitions and application logic

#include <zephyr/logging/log.h>
#include <string.h>
#include "../../include/drivers/input_processor_accel.h"
#include "../config/accel_config.h"

LOG_MODULE_DECLARE(input_processor_accel);

// =============================================================================
// PRESET DEFINITIONS
// =============================================================================

typedef struct {
    const char *name;
    uint16_t sensitivity;
    uint16_t max_factor;
    uint8_t curve_type;
    uint16_t y_boost;
    uint16_t speed_threshold;
    uint16_t speed_max;
    uint16_t min_factor;
} preset_config_t;

static const preset_config_t presets[] = {
    {
        .name = "office",
        .sensitivity = 700,
        .max_factor = 1400,
        .curve_type = 0,  // Linear
        .y_boost = 1000,
        .speed_threshold = 800,
        .speed_max = 2500,
        .min_factor = 800
    },
    {
        .name = "gaming",
        .sensitivity = 1000,
        .max_factor = 2200,
        .curve_type = 1,    // Mild
        .y_boost = 1100,
        .speed_threshold = 600,
        .speed_max = 2500,
        .min_factor = 1000
    },
    {
        .name = "high_sens",
        .sensitivity = 1000,
        .max_factor = 6000,
        .curve_type = 1,    // Mild
        .y_boost = 1300,
        .speed_threshold = 400,
        .speed_max = 1500,
        .min_factor = 1200
    }
};

#define NUM_PRESETS (sizeof(presets) / sizeof(presets[0]))

// =============================================================================
// PRESET APPLICATION FUNCTIONS
// =============================================================================

int accel_config_apply_preset(struct accel_config *cfg, const char *preset_name) {
    if (!cfg || !preset_name) {
        return -EINVAL;
    }

    // Find matching preset
    const preset_config_t *preset = NULL;
    for (size_t i = 0; i < NUM_PRESETS; i++) {
        if (strcmp(presets[i].name, preset_name) == 0) {
            preset = &presets[i];
            break;
        }
    }

    if (!preset) {
        LOG_ERR("Unknown preset: %s", preset_name);
        return -ENOENT;
    }

    // Apply preset values (works for both level 1 and 2)
    cfg->sensitivity = preset->sensitivity;
    cfg->max_factor = preset->max_factor;
    cfg->curve_type = preset->curve_type;
    cfg->y_boost = preset->y_boost;
    
    // Level 2 specific settings
    if (cfg->level == 2) {
        cfg->speed_threshold = preset->speed_threshold;
        cfg->speed_max = preset->speed_max;
        cfg->min_factor = preset->min_factor;
    }

    LOG_INF("Applied preset '%s': sens=%u, max=%u, curve=%u", 
            preset_name, preset->sensitivity, preset->max_factor, preset->curve_type);

    return 0;
}

// =============================================================================
// PRESET DETECTION FROM KCONFIG
// =============================================================================

void accel_config_apply_kconfig_preset(struct accel_config *cfg) {
    #ifdef CONFIG_INPUT_PROCESSOR_ACCEL_PRESET_OFFICE
    accel_config_apply_preset(cfg, "office");
    #elif defined(CONFIG_INPUT_PROCESSOR_ACCEL_PRESET_GAMING)
    accel_config_apply_preset(cfg, "gaming");
    #elif defined(CONFIG_INPUT_PROCESSOR_ACCEL_PRESET_HIGH_SENS)
    accel_config_apply_preset(cfg, "high_sens");
    #endif
}
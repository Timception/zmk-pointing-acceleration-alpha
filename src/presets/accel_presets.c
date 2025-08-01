// accel_presets.c - Preset configuration management
// Centralizes all preset definitions and application logic

#include <zephyr/logging/log.h>
#include <string.h>
#include <drivers/input_processor_accel.h>
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
} preset_config_t;

static const preset_config_t presets[] = {
    {
        .name = "office",
        .sensitivity = 1000,
        .max_factor = 2000,
        .curve_type = 0  // Linear
    },
    {
        .name = "gaming",
        .sensitivity = 1500,
        .max_factor = 3500,
        .curve_type = 2  // Strong
    },
    {
        .name = "high_sens",
        .sensitivity = 1800,
        .max_factor = 4000,
        .curve_type = 1  // Mild
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

    // Apply preset values (only affects level 1 settings)
    cfg->sensitivity = preset->sensitivity;
    cfg->max_factor = preset->max_factor;
    cfg->curve_type = preset->curve_type;

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
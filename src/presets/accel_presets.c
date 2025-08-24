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
    uint8_t curve_type;          // Simple curve type (0-2) used for both levels in presets
    uint16_t y_boost;
    uint16_t speed_threshold;    // Level 2 only
    uint16_t speed_max;          // Level 2 only
    uint16_t min_factor;         // Level 2 only
    uint16_t sensor_dpi;         // Sensor DPI setting
} preset_config_t;

static const preset_config_t presets[] = {
    // Office presets
    {
        .name = "office_optical",
        .sensitivity = 1000,      // 1.0x (800 DPI baseline)
        .max_factor = 2200,       // 2.2x moderate acceleration
        .curve_type = 1,          // Mild
        .y_boost = 1080,          // 1.08x Y-axis enhancement
        .speed_threshold = 700,   // Early acceleration start
        .speed_max = 2600,        // Moderate maximum speed
        .min_factor = 980,        // 0.98x precision control

        .sensor_dpi = 800         // Standard optical sensor
    },
    {
        .name = "office_laser",
        .sensitivity = 1000,      // 1.0x (conservative DPI adjustment)
        .max_factor = 1500,       // 1.5x conservative acceleration (prevent cursor freeze)
        .curve_type = 0,          // Linear (most predictable)
        .y_boost = 1000,          // 1.0x Y-axis standard (avoid issues)
        .speed_threshold = 1000,  // High threshold for stability
        .speed_max = 2000,        // Low max speed for stability
        .min_factor = 1000,       // 1.0x standard (avoid issues)
        .sensor_dpi = 1600        // High precision laser
    },
    {
        .name = "office_trackball",
        .sensitivity = 1500,      // 1.5x (low DPI compensation)
        .max_factor = 2000,       // 2.0x conservative acceleration
        .curve_type = 1,          // Mild
        .y_boost = 1100,          // 1.1x Y-axis enhancement
        .speed_threshold = 800,   // Stable response
        .speed_max = 2400,        // Moderate maximum speed
        .min_factor = 950,        // 0.95x precision focus
        .sensor_dpi = 400         // Low DPI trackball
    },
    
    // Gaming presets
    {
        .name = "gaming_optical",
        .sensitivity = 1000,      // 1.0x (1200 DPI baseline adjustment)
        .max_factor = 2500,       // 2.5x aggressive acceleration
        .curve_type = 2,          // Strong
        .y_boost = 1120,          // 1.12x balance adjustment
        .speed_threshold = 550,   // Quick response
        .speed_max = 2800,        // High-speed movement support
        .min_factor = 950,        // 0.95x precision aiming
        .sensor_dpi = 1200        // Gaming optical sensor
    },
    {
        .name = "gaming_laser",
        .sensitivity = 600,       // 0.6x (ultra-high DPI compensation)
        .max_factor = 2500,       // 2.5x aggressive acceleration
        .curve_type = 2,          // Strong
        .y_boost = 1120,          // 1.12x balance adjustment
        .speed_threshold = 550,   // Quick response
        .speed_max = 2800,        // High-speed movement support
        .min_factor = 950,        // 0.95x precision aiming
        .sensor_dpi = 3200        // High DPI laser
    },
    {
        .name = "gaming_trackball",
        .sensitivity = 1200,      // 1.2x (800 DPI adjustment)
        .max_factor = 2300,       // 2.3x moderate acceleration
        .curve_type = 2,          // Strong
        .y_boost = 1150,          // 1.15x Y-axis enhancement
        .speed_threshold = 600,   // Balanced response
        .speed_max = 2700,        // High-speed movement
        .min_factor = 940,        // 0.94x precision control
        .sensor_dpi = 800         // High precision trackball
    },
    
    // High sensitivity presets
    {
        .name = "high_sens_optical",
        .sensitivity = 1100,      // 1.1x (1600 DPI adjustment)
        .max_factor = 2800,       // 2.8x strong acceleration
        .curve_type = 1,          // Mild (easier to control)
        .y_boost = 1150,          // 1.15x Y-axis enhancement
        .speed_threshold = 450,   // Immediate response
        .speed_max = 2400,        // Moderate adjustment
        .min_factor = 900,        // 0.9x precision focus
        .sensor_dpi = 1600        // High precision optical sensor
    },
    {
        .name = "high_sens_laser",
        .sensitivity = 500,       // 0.5x (ultra-high DPI compensation)
        .max_factor = 2800,       // 2.8x strong acceleration
        .curve_type = 1,          // Mild (easier to control)
        .y_boost = 1150,          // 1.15x Y-axis enhancement
        .speed_threshold = 450,   // Immediate response
        .speed_max = 2400,        // Moderate adjustment
        .min_factor = 900,        // 0.9x precision focus
        .sensor_dpi = 6400        // Ultra-high DPI laser
    },
    {
        .name = "high_sens_trackball",
        .sensitivity = 1400,      // 1.4x (800 DPI adjustment)
        .max_factor = 2600,       // 2.6x strong acceleration
        .curve_type = 1,          // Mild (easier to control)
        .y_boost = 1200,          // 1.2x Y-axis enhancement
        .speed_threshold = 500,   // Quick response
        .speed_max = 2500,        // Moderate adjustment
        .min_factor = 880,        // 0.88x precision focus
        .sensor_dpi = 800         // High precision trackball
    },
    
    // Trackpad/Touchpad presets
    {
        .name = "office_trackpad",
        .sensitivity = 1200,      // 1.2x (trackpad adjustment)
        .max_factor = 1800,       // 1.8x conservative acceleration
        .curve_type = 0,          // Linear (natural feel)
        .y_boost = 1000,          // 1.0x Y-axis standard
        .speed_threshold = 600,   // Moderate response
        .speed_max = 2200,        // Conservative max speed
        .min_factor = 900,        // 0.9x precision focus
        .sensor_dpi = 1000        // Standard trackpad
    },
    {
        .name = "gaming_trackpad",
        .sensitivity = 1100,      // 1.1x (gaming adjustment)
        .max_factor = 2200,       // 2.2x moderate acceleration
        .curve_type = 1,          // Mild (easier to control)
        .y_boost = 1050,          // 1.05x Y-axis slight enhancement
        .speed_threshold = 500,   // Quick response
        .speed_max = 2400,        // Moderate max speed
        .min_factor = 920,        // 0.92x precision control
        .sensor_dpi = 1200        // High resolution trackpad
    },
    {
        .name = "high_sens_trackpad",
        .sensitivity = 1300,      // 1.3x (high sensitivity adjustment)
        .max_factor = 2400,       // 2.4x strong acceleration
        .curve_type = 1,          // Mild (easier to control)
        .y_boost = 1100,          // 1.1x Y-axis enhancement
        .speed_threshold = 400,   // Immediate response
        .speed_max = 2300,        // Moderate adjustment
        .min_factor = 850,        // 0.85x precision focus
        .sensor_dpi = 1200        // High resolution trackpad
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

    LOG_DBG("Found preset: %s", preset_name);
    LOG_DBG("Preset values: sens=%u, max=%u, curve=%u, dpi=%u", 
            preset->sensitivity, preset->max_factor, preset->curve_type, preset->sensor_dpi);
    
    // Apply preset values based on level
    if (cfg->level == 1) {
        cfg->cfg.level1.sensitivity = preset->sensitivity;
        cfg->cfg.level1.max_factor = preset->max_factor;
        cfg->cfg.level1.curve_type = preset->curve_type;
    } else if (cfg->level == 2) {
        cfg->cfg.level2.sensitivity = preset->sensitivity;
        cfg->cfg.level2.max_factor = preset->max_factor;
        cfg->cfg.level2.speed_threshold = preset->speed_threshold;
        cfg->cfg.level2.speed_max = preset->speed_max;
        cfg->cfg.level2.min_factor = preset->min_factor;
        // For presets, use default acceleration_exponent (2 = mild exponential)
        // Advanced curve customization is only available in custom configuration
        cfg->cfg.level2.acceleration_exponent = 2;
    }
    
    // Common settings (encoded format)
    cfg->y_boost_scaled = accel_encode_y_boost(preset->y_boost);
    cfg->sensor_dpi_class = accel_encode_sensor_dpi(preset->sensor_dpi);
    
    LOG_DBG("Applied preset values to config");
    }

    LOG_INF("Applied preset '%s': sens=%u, max=%u, curve=%u", 
            preset_name, preset->sensitivity, preset->max_factor, preset->curve_type);

    return 0;
}

// =============================================================================
// PRESET DETECTION FROM KCONFIG
// =============================================================================

void accel_config_apply_kconfig_preset(struct accel_config *cfg) {
    if (!cfg) {
        LOG_ERR("Configuration pointer is NULL");
        return;
    }

    int ret = 0;
    
    LOG_DBG("Applying Kconfig preset...");
    
    // Office presets
    #ifdef CONFIG_INPUT_PROCESSOR_ACCEL_PRESET_OFFICE_OPTICAL
    LOG_DBG("Applying office_optical preset");
    ret = accel_config_apply_preset(cfg, "office_optical");
    #elif defined(CONFIG_INPUT_PROCESSOR_ACCEL_PRESET_OFFICE_LASER)
    ret = accel_config_apply_preset(cfg, "office_laser");
    #elif defined(CONFIG_INPUT_PROCESSOR_ACCEL_PRESET_OFFICE_TRACKBALL)
    ret = accel_config_apply_preset(cfg, "office_trackball");
    
    // Gaming presets
    #elif defined(CONFIG_INPUT_PROCESSOR_ACCEL_PRESET_GAMING_OPTICAL)
    ret = accel_config_apply_preset(cfg, "gaming_optical");
    #elif defined(CONFIG_INPUT_PROCESSOR_ACCEL_PRESET_GAMING_LASER)
    ret = accel_config_apply_preset(cfg, "gaming_laser");
    #elif defined(CONFIG_INPUT_PROCESSOR_ACCEL_PRESET_GAMING_TRACKBALL)
    ret = accel_config_apply_preset(cfg, "gaming_trackball");
    
    // High sensitivity presets
    #elif defined(CONFIG_INPUT_PROCESSOR_ACCEL_PRESET_HIGH_SENS_OPTICAL)
    ret = accel_config_apply_preset(cfg, "high_sens_optical");
    #elif defined(CONFIG_INPUT_PROCESSOR_ACCEL_PRESET_HIGH_SENS_LASER)
    ret = accel_config_apply_preset(cfg, "high_sens_laser");
    #elif defined(CONFIG_INPUT_PROCESSOR_ACCEL_PRESET_HIGH_SENS_TRACKBALL)
    ret = accel_config_apply_preset(cfg, "high_sens_trackball");
    
    // Trackpad/Touchpad presets
    #elif defined(CONFIG_INPUT_PROCESSOR_ACCEL_PRESET_OFFICE_TRACKPAD)
    ret = accel_config_apply_preset(cfg, "office_trackpad");
    #elif defined(CONFIG_INPUT_PROCESSOR_ACCEL_PRESET_GAMING_TRACKPAD)
    ret = accel_config_apply_preset(cfg, "gaming_trackpad");
    #elif defined(CONFIG_INPUT_PROCESSOR_ACCEL_PRESET_HIGH_SENS_TRACKPAD)
    ret = accel_config_apply_preset(cfg, "high_sens_trackpad");
    #elif defined(CONFIG_INPUT_PROCESSOR_ACCEL_PRESET_CUSTOM)
    // Custom configuration - use device tree values
    LOG_INF("Using custom configuration from device tree");
    ret = 0; // Custom configuration is valid
    #else
    LOG_WRN("No preset configuration selected, using defaults");
    ret = 0; // Use default settings
    #endif
    
    if (ret < 0) {
        LOG_ERR("Failed to apply preset configuration: %d", ret);
    }
}
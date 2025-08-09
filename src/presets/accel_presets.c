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
    uint16_t sensor_dpi;      // 追加：センサーDPI
} preset_config_t;

static const preset_config_t presets[] = {
    // オフィス用プリセット
    {
        .name = "office_optical",
        .sensitivity = 1000,      // 1.0x (800 DPI基準)
        .max_factor = 2200,       // 2.2x 適度な加速度
        .curve_type = 1,          // Mild
        .y_boost = 1080,          // 1.08x Y軸改善
        .speed_threshold = 700,   // 早めの加速度開始
        .speed_max = 2600,        // 適度な最大速度
        .min_factor = 980,        // 0.98x 精密操作
        .sensor_dpi = 800         // 標準光学センサー
    },
    {
        .name = "office_laser",
        .sensitivity = 1000,      // 1.0x (DPI調整を保守的に変更したため基準値に戻す)
        .max_factor = 1500,       // 1.5x 保守的な加速度（カーソル停止防止）
        .curve_type = 0,          // Linear（最も予測可能）
        .y_boost = 1000,          // 1.0x Y軸標準（問題を避けるため）
        .speed_threshold = 1000,  // 高い閾値で安定性重視
        .speed_max = 2000,        // 低い最大速度で安定性重視
        .min_factor = 1000,       // 1.0x 標準（問題を避けるため）
        .sensor_dpi = 1600        // 高精度レーザー
    },
    {
        .name = "office_trackball",
        .sensitivity = 1500,      // 1.5x (低DPI補正)
        .max_factor = 2000,       // 2.0x 控えめな加速度
        .curve_type = 1,          // Mild
        .y_boost = 1100,          // 1.1x Y軸強化
        .speed_threshold = 800,   // 安定した反応
        .speed_max = 2400,        // 適度な最大速度
        .min_factor = 950,        // 0.95x 精密操作重視
        .sensor_dpi = 400         // 低DPIトラックボール
    },
    
    // ゲーミング用プリセット
    {
        .name = "gaming_optical",
        .sensitivity = 1000,      // 1.0x (1200 DPI基準調整)
        .max_factor = 2500,       // 2.5x 積極的な加速度
        .curve_type = 2,          // Strong
        .y_boost = 1120,          // 1.12x バランス調整
        .speed_threshold = 550,   // 素早い反応
        .speed_max = 2800,        // 高速移動対応
        .min_factor = 950,        // 0.95x 精密エイム
        .sensor_dpi = 1200        // ゲーミング光学センサー
    },
    {
        .name = "gaming_laser",
        .sensitivity = 600,       // 0.6x (超高DPI補正)
        .max_factor = 2500,       // 2.5x 積極的な加速度
        .curve_type = 2,          // Strong
        .y_boost = 1120,          // 1.12x バランス調整
        .speed_threshold = 550,   // 素早い反応
        .speed_max = 2800,        // 高速移動対応
        .min_factor = 950,        // 0.95x 精密エイム
        .sensor_dpi = 3200        // 高DPIレーザー
    },
    {
        .name = "gaming_trackball",
        .sensitivity = 1200,      // 1.2x (800 DPI調整)
        .max_factor = 2300,       // 2.3x 適度な加速度
        .curve_type = 2,          // Strong
        .y_boost = 1150,          // 1.15x Y軸強化
        .speed_threshold = 600,   // バランス反応
        .speed_max = 2700,        // 高速移動
        .min_factor = 940,        // 0.94x 精密操作
        .sensor_dpi = 800         // 高精度トラックボール
    },
    
    // 高感度用プリセット
    {
        .name = "high_sens_optical",
        .sensitivity = 1100,      // 1.1x (1600 DPI調整)
        .max_factor = 2800,       // 2.8x 強力な加速度
        .curve_type = 1,          // Mild (制御しやすく)
        .y_boost = 1150,          // 1.15x Y軸強化
        .speed_threshold = 450,   // 即座に反応
        .speed_max = 2400,        // 適度な調整
        .min_factor = 900,        // 0.9x 精密操作重視
        .sensor_dpi = 1600        // 高精度光学センサー
    },
    {
        .name = "high_sens_laser",
        .sensitivity = 500,       // 0.5x (超高DPI補正)
        .max_factor = 2800,       // 2.8x 強力な加速度
        .curve_type = 1,          // Mild (制御しやすく)
        .y_boost = 1150,          // 1.15x Y軸強化
        .speed_threshold = 450,   // 即座に反応
        .speed_max = 2400,        // 適度な調整
        .min_factor = 900,        // 0.9x 精密操作重視
        .sensor_dpi = 6400        // 超高DPIレーザー
    },
    {
        .name = "high_sens_trackball",
        .sensitivity = 1400,      // 1.4x (800 DPI調整)
        .max_factor = 2600,       // 2.6x 強力な加速度
        .curve_type = 1,          // Mild (制御しやすく)
        .y_boost = 1200,          // 1.2x Y軸強化
        .speed_threshold = 500,   // 早い反応
        .speed_max = 2500,        // 適度な調整
        .min_factor = 880,        // 0.88x 精密操作重視
        .sensor_dpi = 800         // 高精度トラックボール
    },
    
    // トラックパッド/タッチパッド用プリセット
    {
        .name = "office_trackpad",
        .sensitivity = 1200,      // 1.2x (タッチパッド用調整)
        .max_factor = 1800,       // 1.8x 控えめな加速度
        .curve_type = 0,          // Linear (自然な操作感)
        .y_boost = 1000,          // 1.0x Y軸標準
        .speed_threshold = 600,   // 適度な反応
        .speed_max = 2200,        // 控えめな最大速度
        .min_factor = 900,        // 0.9x 精密操作重視
        .sensor_dpi = 1000        // 標準タッチパッド
    },
    {
        .name = "gaming_trackpad",
        .sensitivity = 1100,      // 1.1x (ゲーミング調整)
        .max_factor = 2200,       // 2.2x 適度な加速度
        .curve_type = 1,          // Mild (制御しやすく)
        .y_boost = 1050,          // 1.05x Y軸軽微強化
        .speed_threshold = 500,   // 素早い反応
        .speed_max = 2400,        // 適度な最大速度
        .min_factor = 920,        // 0.92x 精密操作
        .sensor_dpi = 1200        // 高解像度タッチパッド
    },
    {
        .name = "high_sens_trackpad",
        .sensitivity = 1300,      // 1.3x (高感度調整)
        .max_factor = 2400,       // 2.4x 強めの加速度
        .curve_type = 1,          // Mild (制御しやすく)
        .y_boost = 1100,          // 1.1x Y軸強化
        .speed_threshold = 400,   // 即座に反応
        .speed_max = 2300,        // 適度な調整
        .min_factor = 850,        // 0.85x 精密操作重視
        .sensor_dpi = 1200        // 高解像度タッチパッド
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
    cfg->sensor_dpi = preset->sensor_dpi;  // センサーDPI設定
    
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
    // オフィス用プリセット
    #ifdef CONFIG_INPUT_PROCESSOR_ACCEL_PRESET_OFFICE_OPTICAL
    accel_config_apply_preset(cfg, "office_optical");
    #elif defined(CONFIG_INPUT_PROCESSOR_ACCEL_PRESET_OFFICE_LASER)
    accel_config_apply_preset(cfg, "office_laser");
    #elif defined(CONFIG_INPUT_PROCESSOR_ACCEL_PRESET_OFFICE_TRACKBALL)
    accel_config_apply_preset(cfg, "office_trackball");
    
    // ゲーミング用プリセット
    #elif defined(CONFIG_INPUT_PROCESSOR_ACCEL_PRESET_GAMING_OPTICAL)
    accel_config_apply_preset(cfg, "gaming_optical");
    #elif defined(CONFIG_INPUT_PROCESSOR_ACCEL_PRESET_GAMING_LASER)
    accel_config_apply_preset(cfg, "gaming_laser");
    #elif defined(CONFIG_INPUT_PROCESSOR_ACCEL_PRESET_GAMING_TRACKBALL)
    accel_config_apply_preset(cfg, "gaming_trackball");
    
    // 高感度用プリセット
    #elif defined(CONFIG_INPUT_PROCESSOR_ACCEL_PRESET_HIGH_SENS_OPTICAL)
    accel_config_apply_preset(cfg, "high_sens_optical");
    #elif defined(CONFIG_INPUT_PROCESSOR_ACCEL_PRESET_HIGH_SENS_LASER)
    accel_config_apply_preset(cfg, "high_sens_laser");
    #elif defined(CONFIG_INPUT_PROCESSOR_ACCEL_PRESET_HIGH_SENS_TRACKBALL)
    accel_config_apply_preset(cfg, "high_sens_trackball");
    
    // トラックパッド/タッチパッド用プリセット
    #elif defined(CONFIG_INPUT_PROCESSOR_ACCEL_PRESET_OFFICE_TRACKPAD)
    accel_config_apply_preset(cfg, "office_trackpad");
    #elif defined(CONFIG_INPUT_PROCESSOR_ACCEL_PRESET_GAMING_TRACKPAD)
    accel_config_apply_preset(cfg, "gaming_trackpad");
    #elif defined(CONFIG_INPUT_PROCESSOR_ACCEL_PRESET_HIGH_SENS_TRACKPAD)
    accel_config_apply_preset(cfg, "high_sens_trackpad");
    #endif
}
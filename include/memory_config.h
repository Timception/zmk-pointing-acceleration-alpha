#pragma once

// メモリ最適化設定
#ifdef CONFIG_INPUT_PROCESSOR_ACCEL_LEVEL_SIMPLE
    #define SPEED_HISTORY_SIZE 4
    #define ACCEL_MAX_CODES 2
    #define ENABLE_PRESETS 1
    #define ENABLE_ADVANCED_CALC 0
#elif CONFIG_INPUT_PROCESSOR_ACCEL_LEVEL_STANDARD
    #define SPEED_HISTORY_SIZE 8
    #define ACCEL_MAX_CODES 4
    #define ENABLE_PRESETS 0
    #define ENABLE_ADVANCED_CALC 1
#else // ADVANCED
    #define SPEED_HISTORY_SIZE 16
    #define ACCEL_MAX_CODES 8
    #define ENABLE_PRESETS 0
    #define ENABLE_ADVANCED_CALC 1
#endif

// デバッグ機能の条件付き無効化
#ifndef CONFIG_DEBUG
    #define LOG_DBG(...)  // デバッグレベルは無効化
    // LOG_INF は残す（デバッグ時に必要）
    #define ASSERT(...)
#endif
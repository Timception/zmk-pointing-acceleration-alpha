// input_processor_accel_utils.c - Utility functions for acceleration processor
// Separated for better code organization and maintainability

#include <zephyr/logging/log.h>
#include <stdlib.h>
#include <drivers/input_processor_accel.h>

LOG_MODULE_DECLARE(input_processor_accel_main);



// =============================================================================
// MATHEMATICAL UTILITY FUNCTIONS
// =============================================================================

uint32_t accel_safe_quadratic_curve(int32_t abs_input, uint32_t multiplier) {
    // Prevent overflow: if abs_input > sqrt(UINT32_MAX / multiplier), clamp it
    const uint32_t max_safe_input = 1000;  // Conservative limit
    
    if (abs_input > max_safe_input) {
        abs_input = max_safe_input;
    }
    
    // Safe calculation with overflow check
    uint64_t temp = (uint64_t)abs_input * abs_input * multiplier;
    if (temp > UINT32_MAX - 1000) {
        return UINT32_MAX;
    }
    
    return 1000 + (uint32_t)temp;
}

// =============================================================================
// ENHANCED TIMING FUNCTIONS
// =============================================================================

uint32_t accel_calculate_smoothed_speed(struct timing_data *timing, uint32_t current_speed) {
    uint32_t history_idx = atomic_get(&timing->history_index);
    
    // Add current speed to history
    timing->speed_history[history_idx].speed = current_speed;
    timing->speed_history[history_idx].timestamp_us = accel_get_precise_time_us();
    timing->speed_history[history_idx].valid = true;
    
    // Update history index (circular buffer)
    atomic_set(&timing->history_index, (history_idx + 1) % SPEED_HISTORY_SIZE);
    
    // Calculate weighted average of recent speeds
    uint64_t weighted_sum = 0;
    uint32_t total_weight = 0;
    int64_t current_time = accel_get_precise_time_us();
    
    for (int i = 0; i < SPEED_HISTORY_SIZE; i++) {
        if (!timing->speed_history[i].valid) continue;
        
        // Calculate age-based weight (newer samples have higher weight)
        int64_t age_us = current_time - timing->speed_history[i].timestamp_us;
        if (age_us > 50000) { // Ignore samples older than 50ms
            timing->speed_history[i].valid = false;
            continue;
        }
        
        // Weight decreases with age: weight = 100 - (age_ms * 2)
        uint32_t weight = 100 - ACCEL_CLAMP(age_us / 500, 0, 90); // age_us/500 = age_ms/0.5
        
        weighted_sum += timing->speed_history[i].speed * weight;
        total_weight += weight;
    }
    
    if (total_weight == 0) {
        return current_speed;
    }
    
    uint32_t smoothed_speed = (uint32_t)(weighted_sum / total_weight);
    atomic_set(&timing->stable_speed, smoothed_speed);
    
    return smoothed_speed;
}

// Simplified speed calculation for MCU efficiency
uint32_t accel_calculate_enhanced_speed(struct timing_data *timing, int32_t input_value) {
    uint32_t current_time_ms = k_uptime_get_32();
    uint32_t last_time_ms = atomic_get(&timing->last_time_us); // Reuse as ms storage
    
    uint32_t time_delta_ms = current_time_ms - last_time_ms;
    
    // Simple speed calculation: input magnitude per time
    uint32_t abs_input = abs(input_value);
    uint32_t speed;
    
    if (time_delta_ms > 0 && time_delta_ms < 500) {
        // Normal case: speed = counts per second
        speed = (abs_input * 1000) / time_delta_ms;
    } else {
        // Edge case: use previous speed or estimate
        speed = atomic_get(&timing->stable_speed);
        if (speed == 0) {
            speed = abs_input * 100; // Simple estimate
        }
    }
    
    // Simple smoothing: 75% old + 25% new
    uint32_t old_speed = atomic_get(&timing->stable_speed);
    speed = (old_speed * 3 + speed) / 4;
    
    // Update state
    atomic_set(&timing->last_time_us, current_time_ms);
    atomic_set(&timing->stable_speed, speed);
    
    return speed;
}
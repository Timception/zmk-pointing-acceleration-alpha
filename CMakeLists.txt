# CMakeLists.txt

zephyr_library()

# Core source files (always included)
zephyr_library_sources(
  src/input_processor_accel_main.c
  src/input_processor_accel_utils.c
  src/input_processor_accel_calc_common.c
  src/config/accel_config.c
  src/validation/accel_validation.c
  src/presets/accel_presets.c
)

# Level-specific source files (conditionally included)
# Note: Both levels can be compiled simultaneously for runtime selection

# Always include Level 1 (Simple) - it's used as fallback
zephyr_library_sources(
  src/input_processor_accel_calc_level1.c
)

# Include Level 2 (Standard) only if enabled
zephyr_library_sources_ifdef(CONFIG_INPUT_PROCESSOR_ACCEL_LEVEL_STANDARD
  src/input_processor_accel_calc_level2.c
)

# Include directories
zephyr_library_include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# Add ZMK app include directory - use all possible paths
zephyr_include_directories(${APPLICATION_SOURCE_DIR}/include)

# Compiler optimizations for size
zephyr_compile_options(-Os -ffunction-sections -fdata-sections)
zephyr_link_libraries(-Wl,--gc-sections)

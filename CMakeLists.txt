# CMakeLists.txt

zephyr_library()

# Core source files (always included)
zephyr_library_sources(
  src/input_processor_accel_main.c
  src/input_processor_accel_utils.c
  src/config/accel_config.c
  src/validation/accel_validation.c
)

# Conditional compilation based on configuration level
zephyr_library_sources_ifdef(CONFIG_INPUT_PROCESSOR_ACCEL_LEVEL_SIMPLE
  src/presets/accel_presets.c
)

zephyr_library_sources_ifdef(CONFIG_INPUT_PROCESSOR_ACCEL_LEVEL_STANDARD
  src/input_processor_accel_calc.c
)

zephyr_library_sources_ifdef(CONFIG_INPUT_PROCESSOR_ACCEL_LEVEL_ADVANCED
  src/input_processor_accel_calc.c
)

# Include directories
zephyr_library_include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Compiler optimizations for size
zephyr_compile_options(-Os -ffunction-sections -fdata-sections)
zephyr_link_libraries(-Wl,--gc-sections)

# CMakeLists.txt

zephyr_library()

# Core source files (always included)
zephyr_library_sources(
  src/input_processor_accel_main.c
  src/input_processor_accel_utils.c
  src/input_processor_accel_calc.c
  src/config/accel_config.c
  src/validation/accel_validation.c
  src/presets/accel_presets.c
)

# Include directories
zephyr_library_include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Compiler optimizations for size
zephyr_compile_options(-Os -ffunction-sections -fdata-sections)
zephyr_link_libraries(-Wl,--gc-sections)

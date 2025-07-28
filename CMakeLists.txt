# CMakeLists.txt

zephyr_library()

# Main source files
zephyr_library_sources(
  src/input_processor_accel_main.c
  src/input_processor_accel_utils.c
  src/input_processor_accel_calc.c
  src/config/accel_config.c
  src/presets/accel_presets.c
  src/validation/accel_validation.c
)

# Include directories
zephyr_library_include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${CMAKE_CURRENT_SOURCE_DIR}/dts/bindings
  ${CMAKE_CURRENT_SOURCE_DIR}/dts
  ${ZMK_BASE}/app/include
)

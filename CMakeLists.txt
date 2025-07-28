
zephyr_library()

# Main source files
zephyr_library_sources(src/input_processor_accel_main.c)
zephyr_library_sources(src/input_processor_accel_utils.c)
zephyr_library_sources(src/input_processor_accel_calc.c)

# Include directories
zephyr_library_include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
zephyr_library_include_directories(${CMAKE_CURRENT_SOURCE_DIR}/dts/bindings)
zephyr_library_include_directories(${CMAKE_CURRENT_SOURCE_DIR}/dts/)
zephyr_library_include_directories(${ZMK_BASE}/app/include)
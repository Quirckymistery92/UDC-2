# The following five lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.16)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)

# Suppress noisy warnings from LVGL and dependencies
add_compile_options(-Wno-format)
add_compile_options(-Wno-ignored-qualifiers)
add_compile_options(-Wno-attributes)
add_compile_options(-Wno-unused-function)

project(UDC-2)

# Generate SPIFFS image from spiffs_data/ and flash it to the "storage" partition
spiffs_create_partition_image(storage ${CMAKE_SOURCE_DIR}/spiffs_data FLASH_IN_PROJECT)

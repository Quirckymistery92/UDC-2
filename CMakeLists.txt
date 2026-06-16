# The following five lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.16)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)

# Script de contador de compilaciones
include(${CMAKE_SOURCE_DIR}/build_counter.cmake)

# Suppress noisy warnings from LVGL and dependencies
add_compile_options(-Wno-format)
add_compile_options(-Wno-ignored-qualifiers)
add_compile_options(-Wno-attributes)
add_compile_options(-Wno-unused-function)

project(UDC-2)

# La función correcta para imágenes compatibles con Wear Leveling:
fatfs_create_spiflash_image(storage ${CMAKE_SOURCE_DIR}/fatfs_data FLASH_IN_PROJECT)

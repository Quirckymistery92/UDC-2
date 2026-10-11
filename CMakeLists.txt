# The following five lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.16)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)

# Suppress noisy warnings from LVGL and dependencies
add_compile_options(-Wno-format)
add_compile_options(-Wno-ignored-qualifiers)
add_compile_options(-Wno-attributes)
add_compile_options(-Wno-unused-function)
add_compile_options("-I${CMAKE_CURRENT_SOURCE_DIR}/components/desc_apps/include")

project(UDC-2)

littlefs_create_partition_image(storage ${CMAKE_SOURCE_DIR}/storage_files FLASH_IN_PROJECT)

set(GENERATED_INCLUDE_DIR "${CMAKE_BINARY_DIR}/include")
file(MAKE_DIRECTORY ${GENERATED_INCLUDE_DIR})

add_custom_target(update_version_info ALL
    COMMAND ${CMAKE_COMMAND} -E echo "Actualizando datos de compilación..."
    COMMAND python "${CMAKE_SOURCE_DIR}/generate_build_info.py" "${CMAKE_BINARY_DIR}" "${CMAKE_SOURCE_DIR}"
    BYPRODUCTS "${GENERATED_INCLUDE_DIR}/version_info.h" "${GENERATED_INCLUDE_DIR}/build_timestamp.h"
    VERBATIM
)

idf_build_set_property(COMPILE_OPTIONS "-I${GENERATED_INCLUDE_DIR}" APPEND)

add_dependencies(__idf_main update_version_info)

string(TIMESTAMP BUILD_DATETIME "%H%M%S")
target_compile_options(__idf_main PRIVATE "-D__BUILD_NONCE__=${BUILD_DATETIME}")
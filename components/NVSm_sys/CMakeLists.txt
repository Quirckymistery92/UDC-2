set(COMPONENT_DIR ${CMAKE_CURRENT_SOURCE_DIR})

# Incluye las carpetas de core y módulos como includes públicos del componente
set(COMPONENT_INCLUDE_DIRS
    core
    modules
)

# Detecta automáticamente todos los sources de módulos bajo modules/
file(GLOB_RECURSE NVSM_MODULE_SOURCES RELATIVE ${COMPONENT_DIR} modules/*.c)

# Detecta los sources de core
file(GLOB_RECURSE NVSM_CORE_SOURCES RELATIVE ${COMPONENT_DIR} core/*.c)

set(COMPONENTS_SRCS
    ${NVSM_CORE_SOURCES}
    ${NVSM_MODULE_SOURCES}
)

# Avisa a CMake que reconfigure si cambian los archivos del componente
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS
    ${COMPONENT_DIR}/modules/*.c
    ${COMPONENT_DIR}/core/*.c
    ${COMPONENT_DIR}/modules/*
)

# Agrega automáticamente los includes de cada módulo si existen
file(GLOB MODULE_DIRS RELATIVE ${COMPONENT_DIR} modules/*)
foreach(_mod_dir ${MODULE_DIRS})
    if(IS_DIRECTORY "${COMPONENT_DIR}/${_mod_dir}")
        list(APPEND COMPONENT_INCLUDE_DIRS "${_mod_dir}")
    endif()
endforeach()

idf_component_register(
    SRCS ${COMPONENTS_SRCS}
    INCLUDE_DIRS include ${COMPONENT_INCLUDE_DIRS}
    REQUIRES nvs_flash freertos esp_common
)
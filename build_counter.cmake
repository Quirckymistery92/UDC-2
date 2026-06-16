# Script de contador de compilaciones
set(BUILD_COUNTER_FILE "${CMAKE_SOURCE_DIR}/build_counter.txt")
set(BUILD_LOG_FILE "${CMAKE_SOURCE_DIR}/compilation_log.txt")

# Inicializar contador si no existe
if(NOT EXISTS "${BUILD_COUNTER_FILE}")
    file(WRITE "${BUILD_COUNTER_FILE}" "0")
endif()

# Leer el contador actual
file(READ "${BUILD_COUNTER_FILE}" CURRENT_COUNT)
string(STRIP "${CURRENT_COUNT}" CURRENT_COUNT)

# Incrementar el contador
math(EXPR NEW_COUNT "${CURRENT_COUNT} + 1")

# Guardar el nuevo contador
file(WRITE "${BUILD_COUNTER_FILE}" "${NEW_COUNT}")

# Obtener timestamp
string(TIMESTAMP CURRENT_TIME "%Y-%m-%d %H:%M:%S")

# Agregar al archivo de log
file(APPEND "${BUILD_LOG_FILE}" "[${CURRENT_TIME}] Build #${NEW_COUNT}\n")

# Mostrar mensaje en consola
message(STATUS "Build #${NEW_COUNT} - ${CURRENT_TIME}")

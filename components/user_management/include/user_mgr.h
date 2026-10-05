/**
 * @file user_mgr.h
 * @brief API pública del User Manager con soporte de métricas de PIN.
 * @details Interfaz puramente en C compatible con LVGL 9 y EEZ Studio.
 * Procesamiento asíncrono (Active Object) y persistencia FATFS (C++ interno).
 */

#pragma once

/* ESP */
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "esp_err.h"

/* Módulos */
#include "ui_manager.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ==========================================================================
 * MACROS Y CONSTANTES DE CONFIGURACIÓN GLOBALES
 * ========================================================================== */

#define USER_MGR_USER_GUEST     0xFFFF  /**< ID especial reservado para el usuario invitado */
#define USER_MGR_USER_GUEST_PIN "0000"
#define USER_MGR_MAX_PIN_LEN    6      /**< Longitud máxima del búfer del PIN (incluye \0) */
#define USER_MGR_MIN_PIN_VAL    4       /**< Longitud mínima permitida para un PIN válido */
#define USER_MGR_MAX_PIN_VAL    6       /**< Longitud máxima permitida para un PIN válido */
#define USER_MGR_MAX_NAME_LEN   32      /**< Longitud máxima del nombre (incluye \0) */
#define USER_MGR_MAX_USERS      6       /**< Cantidad máxima de usuarios soportados */
#define USER_MGR_MSG_LOCK_L1 "30 segundos"
#define USER_MGR_MSG_LOCK_L2 "5 minutos"
#define USER_MGR_MSG_LOCK_L3 "15 minutos"
#define USER_MGR_MSG_LOCK_PERMA "Permanente"

/* ==========================================================================
 * TIPOS DE DATOS Y EVENTOS
 * ========================================================================== */

typedef enum {
    /* Eventos de Peticiones Asíncronas (1:1) */
    USER_MGR_EVT_SUCCESS = 0,
    USER_MGR_EVT_ERR_WRONG_PIN,
    USER_MGR_EVT_ERR_NOT_FOUND,
    USER_MGR_EVT_ERR_DISABLED,
    USER_MGR_EVT_ERR_LOCKED,
    USER_MGR_EVT_ERR_DB_FULL,
    USER_MGR_EVT_ERR_TEMP_LOCKED,
    USER_MGR_EVT_ERR_INTERNAL,
    
    /* Eventos Globales (Broadcast para el Bridge Gráfico) */
    USER_MGR_EVT_GLOBAL_DB_CHANGED,
    USER_MGR_EVT_GLOBAL_LOGOUT,
    USER_MGR_EVT_GLOBAL_LOGIN
} user_mgr_event_t;

typedef enum {
    USER_MGR_PERM_GUEST = 0,
    USER_MGR_PERM_USER,
    USER_MGR_PERM_ADMIN
} user_mgr_perm_t;

typedef struct {
    uint32_t lockout_remaining_ms;
    uint8_t failed_attempts;
    uint8_t max_attempts;
} user_mgr_event_info_t;

/**
 * @brief Placeholder para futura integración con ESP-DL (Reconocimiento facial)
 */
typedef struct {
    uint8_t *feature_tensor;
    size_t size;
} user_mgr_face_data_t;

/**
 * @brief Estructura de visualización pública de un usuario.
 */
typedef struct {
    uint16_t id;
    char name[USER_MGR_MAX_NAME_LEN];
    user_mgr_perm_t permission;
    ui_icon_id_t avatar_id;
    size_t pin_length;
    const char* role_str;
    bool is_enabled;
} user_mgr_user_view_t;

/**
 * @brief Estructura de creación para registrar nuevos usuarios.
 */
typedef struct {
    char name[USER_MGR_MAX_NAME_LEN];
    char pin[USER_MGR_MAX_PIN_LEN];
    user_mgr_perm_t permission;
    ui_icon_id_t avatar_id;
    user_mgr_face_data_t *face_data; /**< Preparado para ESP-DL, puede ser NULL */
} user_mgr_user_create_t;

/* ==========================================================================
 * FIRMAS DE CALLBACKS (Delegados)
 * ========================================================================== */

typedef void (*user_mgr_cb_t)(user_mgr_event_t event, uint16_t user_id, void *ctx);
typedef void (*user_mgr_view_cb_t)(user_mgr_user_view_t **sorted_array, uint16_t count, void *ctx);
typedef void (*user_mgr_global_cb_t)(user_mgr_event_t event);

/* ==========================================================================
 * 1. INICIALIZACIÓN DEL SISTEMA
 * ========================================================================== */

esp_err_t user_mgr_init(user_mgr_global_cb_t ui_bridge_listener);

/* ==========================================================================
 * 2. LECTURAS SÍNCRONAS (Protegidas por Mutex, rápidas para UI)
 * ========================================================================== */

uint16_t user_mgr_sess_get_active(void);
esp_err_t user_mgr_sess_has_active_user_sync(bool *out_is_active);
uint16_t user_mgr_sess_get_last(void);
bool user_mgr_user_guest_is_enabled(void);
esp_err_t user_mgr_user_get_data(uint16_t user_id, user_mgr_user_view_t *out_data);
const char* user_mgr_auth_get_lock_msg(uint16_t user_id);
bool user_mgr_auth_get_lock_temp_isLocked(uint16_t user_id);
uint32_t user_mgr_auth_get_lock_remaining_ms(uint16_t user_id);

/* ==========================================================================
 * 3. GESTIÓN ASÍNCRONA DE SESIÓN Y AUTENTICACIÓN (Colas FreeRTOS)
 * ========================================================================== */

esp_err_t user_mgr_sess_req_login(uint16_t user_id, const char *pin, user_mgr_cb_t cb, void *ctx);
esp_err_t user_mgr_sess_req_login_face(uint16_t user_id, user_mgr_face_data_t *face, user_mgr_cb_t cb, void *ctx);
esp_err_t user_mgr_auth_req_validate_pin(uint16_t user_id, const char *pin, user_mgr_cb_t cb, void *ctx);
esp_err_t user_mgr_sess_req_logout(user_mgr_cb_t cb, void *ctx);

/* ==========================================================================
 * 4. OPERACIONES ASÍNCRONAS DE BASE DE DATOS (CRUD)
 * ========================================================================== */

esp_err_t user_mgr_user_req_create(const user_mgr_user_create_t *data, user_mgr_cb_t cb, void *ctx);
esp_err_t user_mgr_user_req_delete(uint16_t user_id, user_mgr_cb_t cb, void *ctx);
esp_err_t user_mgr_user_req_guest_toggle(bool enable, user_mgr_cb_t cb, void *ctx);
esp_err_t user_mgr_user_req_sorted_view(user_mgr_view_cb_t cb, void *ctx);

#ifdef __cplusplus
}
#endif
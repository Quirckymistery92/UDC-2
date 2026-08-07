/**
 * @file sys_identity.h
 * @brief API Pública del Identity Manager con soporte de métricas de PIN.
 * @details Interfaz puramente en C compatible con LVGL 9 y EEZ Studio. 
 * Procesamiento asíncrono (Active Object) y persistencia FATFS (C++ interno).
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "esp_err.h"
#include "ui_manager.h"
#ifdef __cplusplus
extern "C" {
#endif

/* ==========================================================================
 * MACROS Y CONSTANTES DE CONFIGURACIÓN GLOBALES
 * ========================================================================== */

#define SYS_ID_USER_GUEST     0xFFFF  /**< ID especial reservado para el usuario invitado */
#define SYS_ID_USER_GUEST_PIN "0000"
#define SYS_ID_MAX_PIN_LEN    6      /**< Longitud máxima del búfer del PIN (incluye \0) */
#define SYS_ID_MIN_PIN_VAL    4       /**< Longitud mínima permitida para un PIN válido */
#define SYS_ID_MAX_PIN_VAL    6       /**< Longitud máxima permitida para un PIN válido */
#define SYS_ID_MAX_NAME_LEN   32      /**< Longitud máxima del nombre (incluye \0) */
#define SYS_ID_MAX_USERS      6       /**< Cantidad máxima de usuarios soportados */
#define SYS_ID_MSG_LOCK_L1 "30 segundos"
#define SYS_ID_MSG_LOCK_L2 "5 minutos"
#define SYS_ID_MSG_LOCK_L3 "15 minutos"
#define SYS_ID_MSG_LOCK_PERMA "Permanente"

/* ==========================================================================
 * TIPOS DE DATOS Y EVENTOS
 * ========================================================================== */

typedef enum {
    /* Eventos de Peticiones Asíncronas (1:1) */
    SYS_ID_EVT_SUCCESS = 0,         
    SYS_ID_EVT_ERR_WRONG_PIN,       
    SYS_ID_EVT_ERR_NOT_FOUND,       
    SYS_ID_EVT_ERR_DISABLED,        
    SYS_ID_EVT_ERR_LOCKED,
    SYS_ID_EVT_ERR_DB_FULL,
    SYS_ID_EVT_ERR_TEMP_LOCKED,
    SYS_ID_EVT_ERR_INTERNAL,
    
    /* Eventos Globales (Broadcast para el Bridge Gráfico) */
    SYS_ID_EVT_GLOBAL_DB_CHANGED,   
    SYS_ID_EVT_GLOBAL_LOGOUT,
    SYS_ID_EVT_GLOBAL_LOGIN        
} sys_id_event_t;

typedef enum {
    SYS_ID_PERM_GUEST = 0,
    SYS_ID_PERM_USER,
    SYS_ID_PERM_ADMIN
} sys_id_perm_t;

typedef struct {
    uint32_t lockout_remaining_ms;
    uint8_t failed_attempts;
    uint8_t max_attempts;
} sys_id_event_info_t;

/**
 * @brief Placeholder para futura integración con ESP-DL (Reconocimiento facial)
 */
typedef struct {
    uint8_t *feature_tensor; 
    size_t size;             
} sys_id_face_data_t;

/**
 * @brief Estructura de visualización pública de un usuario.
 */
typedef struct {
    uint16_t id;                        
    char name[SYS_ID_MAX_NAME_LEN];     
    sys_id_perm_t permission;           
    ui_icon_id_t avatar_id;                 
    size_t pin_length;                 /**< Longitud real del PIN (4 a 6 caracteres. Guest es 0) */
    const char* role_str;
    bool is_enabled;                    
} sys_id_user_view_t;

/**
 * @brief Estructura de creación para registrar nuevos usuarios.
 */
typedef struct {
    char name[SYS_ID_MAX_NAME_LEN];
    char pin[SYS_ID_MAX_PIN_LEN];
    sys_id_perm_t permission;
    ui_icon_id_t avatar_id;
    sys_id_face_data_t *face_data; /**< Preparado para ESP-DL, puede ser NULL */
} sys_id_user_create_t;

/* ==========================================================================
 * FIRMAS DE CALLBACKS (Delegados)
 * ========================================================================== */

typedef void (*sys_id_cb_t)(sys_id_event_t event, uint16_t user_id, void *ctx);
typedef void (*sys_id_view_cb_t)(sys_id_user_view_t **sorted_array, uint16_t count, void *ctx);
typedef void (*sys_id_global_cb_t)(sys_id_event_t event);

/* ==========================================================================
 * 1. INICIALIZACIÓN DEL SISTEMA
 * ========================================================================== */

esp_err_t sys_id_init(sys_id_global_cb_t ui_bridge_listener);

/* ==========================================================================
 * 2. LECTURAS SÍNCRONAS (Protegidas por Mutex, rápidas para UI)
 * ========================================================================== */

uint16_t sys_id_sess_get_active(void);
uint16_t sys_id_sess_get_last(void);
bool sys_id_user_guest_is_enabled(void);
esp_err_t sys_id_user_get_data(uint16_t user_id, sys_id_user_view_t *out_data);
const char* sys_id_auth_get_lock_msg(uint16_t user_id);
bool sys_id_auth_get_lock_temp_isLocked(uint16_t user_id);
uint32_t sys_id_auth_get_lock_remaining_ms(uint16_t user_id);

/* ==========================================================================
 * 3. GESTIÓN ASÍNCRONA DE SESIÓN Y AUTENTICACIÓN (Colas FreeRTOS)
 * ========================================================================== */

esp_err_t sys_id_sess_req_login(uint16_t user_id, const char *pin, sys_id_cb_t cb, void *ctx);
esp_err_t sys_id_sess_req_login_face(uint16_t user_id, sys_id_face_data_t *face, sys_id_cb_t cb, void *ctx);
esp_err_t sys_id_auth_req_validate_pin(uint16_t user_id, const char *pin, sys_id_cb_t cb, void *ctx);
esp_err_t sys_id_sess_req_logout(sys_id_cb_t cb, void *ctx);

/* ==========================================================================
 * 4. OPERACIONES ASÍNCRONAS DE BASE DE DATOS (CRUD)
 * ========================================================================== */

esp_err_t sys_id_user_req_create(const sys_id_user_create_t *data, sys_id_cb_t cb, void *ctx);
esp_err_t sys_id_user_req_delete(uint16_t user_id, sys_id_cb_t cb, void *ctx);
esp_err_t sys_id_user_req_guest_toggle(bool enable, sys_id_cb_t cb, void *ctx);
esp_err_t sys_id_user_req_sorted_view(sys_id_view_cb_t cb, void *ctx);

#ifdef __cplusplus
}
#endif
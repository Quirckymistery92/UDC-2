#include "user_mgr.h"

/* ESP */
#include <cstring>
#include <cstdio>
#include <sys/stat.h>
#include <unistd.h>
#include <algorithm>
#include "esp_timer.h"
#include "esp_check.h"
#include "esp_log.h"
#include "esp_rom_crc.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"

/* Módulos */
#include "ui_manager.h"

// ============================================================================
// ESTRUCTURAS INTERNAS Y ALINEACIÓN DE MEMORIA (ANTI-UB)
// ============================================================================

enum class CmdType {
    REQ_LOGIN,
    REQ_LOGIN_FACE,
    REQ_LOGOUT,
    REQ_VALIDATE,
    REQ_CREATE,
    REQ_DELETE,
    REQ_GUEST_TOGGLE,
    REQ_SORTED_VIEW
};

struct SysCmdMsg {
    CmdType type;
    uint16_t user_id;
    user_mgr_cb_t cb;
    void *ctx;
    union {
        char pin[USER_MGR_MAX_PIN_LEN];
        user_mgr_user_create_t create_data;
        user_mgr_face_data_t face_data;
        bool toggle_state;
        user_mgr_view_cb_t view_cb;
    } payload;
};

struct __attribute__((packed)) UserMgrRecord {
    user_mgr_user_view_t public_data; 
    char pin[USER_MGR_MAX_PIN_LEN];
};

struct UserMgrNode {
    bool is_used;
    UserMgrRecord record;
};

struct UserMgrAuthState {
    uint8_t failed_attempts;
    uint8_t lock_layer;
    uint64_t lockout_until_ms;
};

struct __attribute__((packed)) DbHeader {
    uint32_t magic_word;
    uint32_t crc32;
    uint16_t version;
    uint16_t user_count;
};

// ============================================================================
// CLASE MANAGER (SINGLETON - ACTIVE OBJECT)
// ============================================================================

class UserMgrManager {
private:
    static constexpr const char* TAG = "USER_MGR";
    static constexpr const char* DB_FILE_PATH = "/fatfs/users.dat";
    static constexpr const char* DB_TEMP_PATH = "/fatfs/users.tmp";
    static constexpr uint32_t DB_MAGIC_WORD = 0x1A2B3C4E; 
    static constexpr uint16_t DB_VERSION = 2;
    static constexpr uint8_t CMD_QUEUE_SIZE = 15;
    static constexpr uint32_t LOCKOUT_TIME_MS = 30000;
    static constexpr uint8_t MAX_FAILED_ATTEMPTS = 3;
    static constexpr uint16_t USER_MGR_USER_NONE = 0; 
    static constexpr uint32_t LOCK_TIME_L1_MS = 30 * 1000;
    static constexpr uint32_t LOCK_TIME_L2_MS = 5 * 60 * 1000;
    static constexpr uint32_t LOCK_TIME_L3_MS = 15 * 60 * 1000;

    UserMgrNode m_user_db[USER_MGR_MAX_USERS]{};
    UserMgrAuthState m_auth_state[USER_MGR_MAX_USERS]{};
    
    // La sesión inicia vacía, no en Guest.
    uint16_t m_active_user = USER_MGR_USER_NONE;
    uint16_t m_last_user = USER_MGR_USER_NONE;
    
    // Los IDs de usuario reales comienzan en 1.
    uint16_t m_next_id = 1;

    user_mgr_global_cb_t m_global_bridge_cb = nullptr;
    QueueHandle_t m_queue = nullptr;
    SemaphoreHandle_t m_mutex = nullptr;
    bool m_initialized = false;

    UserMgrManager() {
        m_next_id = 1;
    }
    static UserMgrManager* s_instance;

    uint64_t getCurrentMs() const {
        return static_cast<uint64_t>(esp_timer_get_time() / 1000);
    }

    int16_t findIndexById(uint16_t user_id) const {
        if (user_id == USER_MGR_USER_NONE) return -1;
        for (int16_t i = 0; i < USER_MGR_MAX_USERS; i++) {
            if (m_user_db[i].is_used && m_user_db[i].record.public_data.id == user_id) {
                return i;
            }
        }
        return -1;
    }

    int16_t findFreeSlot() const {
        for (int16_t i = 0; i < USER_MGR_MAX_USERS; i++) {
            if (!m_user_db[i].is_used) return i;
        }
        return -1;
    }

    static const char* getPermissionString(uint8_t permission) {
        switch(permission) {
            case USER_MGR_PERM_ADMIN: return "Admin";
            case USER_MGR_PERM_USER:  return "Usuario";
            case USER_MGR_PERM_GUEST: return "Invitado";
            default: return "Desconocido";
        }
    }

    void injectGuest() {
        if (findIndexById(USER_MGR_USER_GUEST) >= 0) return;

        int16_t slot = findFreeSlot();
        if (slot >= 0) {
            m_user_db[slot].is_used = true;
            m_user_db[slot].record.public_data.id = USER_MGR_USER_GUEST;
            strlcpy(m_user_db[slot].record.public_data.name, "Invitado", USER_MGR_MAX_NAME_LEN);
            m_user_db[slot].record.public_data.permission = USER_MGR_PERM_GUEST;
            m_user_db[slot].record.public_data.avatar_id = UI_ICON_USER_LOGO_GUEST;
            strlcpy(m_user_db[slot].record.pin, USER_MGR_USER_GUEST_PIN, USER_MGR_MAX_PIN_LEN);
            m_user_db[slot].record.public_data.pin_length = strlen(USER_MGR_USER_GUEST_PIN); 
            m_user_db[slot].record.public_data.is_enabled = true;
            ESP_LOGI(TAG, "Usuario Guest agregado automaticamente.");
        }
    }

    bool enforceMinimumUsers() {
        bool has_active = false;
        for (int i = 0; i < USER_MGR_MAX_USERS; i++) {
            if (m_user_db[i].is_used && m_user_db[i].record.public_data.is_enabled) {
                has_active = true;
                break;
            }
        }
        
        if (!has_active) {
            ESP_LOGW(TAG, "0 usuarios activos en sistema. Forzando inyeccion de Invitado.");
            injectGuest();
            return true;
        }
        return false;
    }

    void saveDb() {
        UserMgrRecord io_buffer[USER_MGR_MAX_USERS];
        uint16_t active_count = 0;

        for (int i = 0; i < USER_MGR_MAX_USERS; i++) {
            if (m_user_db[i].is_used) {
                io_buffer[active_count++] = m_user_db[i].record;
            }
        }

        uint32_t crc = esp_rom_crc32_le(0, reinterpret_cast<const uint8_t*>(io_buffer), sizeof(UserMgrRecord) * active_count);
        DbHeader header = { DB_MAGIC_WORD, crc, DB_VERSION, active_count };

        FILE* f = fopen(DB_TEMP_PATH, "wb");
        if (f) {
            fwrite(&header, sizeof(DbHeader), 1, f);
            if (active_count > 0) fwrite(io_buffer, sizeof(UserMgrRecord), active_count, f);
            fflush(f); fsync(fileno(f)); fclose(f);
            unlink(DB_FILE_PATH);
            rename(DB_TEMP_PATH, DB_FILE_PATH);
        } else {
            ESP_LOGE(TAG, "Fallo al escribir FATFS.");
        }
    }

    bool loadDb() {
        bool db_changed = false;
        FILE* f = fopen(DB_FILE_PATH, "rb");
        
        if (!f) {
            ESP_LOGW(TAG, "Database no encontrada. Creando nueva.");
            enforceMinimumUsers();
            saveDb();
            return true;
        }

        DbHeader header;
        if (fread(&header, sizeof(DbHeader), 1, f) != 1 || header.magic_word != DB_MAGIC_WORD) {
            fclose(f); unlink(DB_FILE_PATH); 
            enforceMinimumUsers();
            saveDb(); 
            return true;
        }

        if (header.user_count == 0) {
            db_changed = true;
        } else {
            UserMgrRecord io_buffer[USER_MGR_MAX_USERS];
            if (fread(io_buffer, sizeof(UserMgrRecord), header.user_count, f) == header.user_count) {
                uint32_t calc_crc = esp_rom_crc32_le(0, reinterpret_cast<const uint8_t*>(io_buffer), sizeof(UserMgrRecord) * header.user_count);
                if (calc_crc == header.crc32) {
                    for (uint16_t i = 0; i < header.user_count; i++) {
                        if (i < USER_MGR_MAX_USERS) {
                            m_user_db[i].record = io_buffer[i];
                            m_user_db[i].is_used = true;
                            // Prevenimos que el iterador pise IDs reservados.
                            if (m_user_db[i].record.public_data.id >= m_next_id) {
                                m_next_id = m_user_db[i].record.public_data.id + 1;

                                if (m_next_id == USER_MGR_USER_NONE) {
                                    m_next_id = 1;
                                }
                            }
                        }
                    }
                } else {
                    ESP_LOGE(TAG, "CRC DB corrupto. Recreando.");
                    db_changed = true;
                }
            } else {
                ESP_LOGE(TAG, "Error lectura de FATFS. Recreando.");
                db_changed = true;
            }
        }
        fclose(f);

        if (enforceMinimumUsers()) db_changed = true;
        if (db_changed) saveDb();
        
        return db_changed;
    }

    static void taskWrapper(void* arg) {
        static_cast<UserMgrManager*>(arg)->runCoreLoop();
    }

    void runCoreLoop() {
      SysCmdMsg cmd;
      bool trigger_db_save;
      bool trigger_ui_update;

      while (true) {
        if (xQueueReceive(m_queue, &cmd, portMAX_DELAY) == pdTRUE) {
          xSemaphoreTakeRecursive(m_mutex, portMAX_DELAY);
          trigger_db_save = false;
          trigger_ui_update = false;

          switch (cmd.type) {
          case CmdType::REQ_LOGIN:
          case CmdType::REQ_VALIDATE: {
            uint64_t now_ms = getCurrentMs();
            int16_t idx = findIndexById(cmd.user_id);

            if (idx < 0) {
              if (cmd.cb) cmd.cb(USER_MGR_EVT_ERR_NOT_FOUND, cmd.user_id, cmd.ctx);
            } else if (!m_user_db[idx].record.public_data.is_enabled || m_auth_state[idx].lock_layer >= 4) {
              if (cmd.cb) cmd.cb(USER_MGR_EVT_ERR_LOCKED, cmd.user_id, cmd.ctx);
            } else if (m_auth_state[idx].lock_layer > 0 && now_ms < m_auth_state[idx].lockout_until_ms) {
              if (cmd.cb) cmd.cb(USER_MGR_EVT_ERR_TEMP_LOCKED, cmd.user_id, cmd.ctx);
            } else {
              bool pin_ok = (strcmp(m_user_db[idx].record.pin, cmd.payload.pin) == 0);

              if (pin_ok) {
                m_auth_state[idx].failed_attempts = 0;
                m_auth_state[idx].lock_layer = 0;
                m_auth_state[idx].lockout_until_ms = 0;

                if (cmd.type == CmdType::REQ_LOGIN) {
                  m_active_user = cmd.user_id;
                  if (m_global_bridge_cb) m_global_bridge_cb(USER_MGR_EVT_GLOBAL_LOGIN);
                } 
                if (cmd.cb) cmd.cb(USER_MGR_EVT_SUCCESS, cmd.user_id, cmd.ctx);
                
              } else {
                m_auth_state[idx].failed_attempts++;

                if (m_auth_state[idx].failed_attempts >= MAX_FAILED_ATTEMPTS) {
                  m_auth_state[idx].failed_attempts = 0;
                  m_auth_state[idx].lock_layer++;

                  if (m_auth_state[idx].lock_layer == 1) {
                    m_auth_state[idx].lockout_until_ms = now_ms + LOCK_TIME_L1_MS;
                    if (cmd.cb) cmd.cb(USER_MGR_EVT_ERR_TEMP_LOCKED, cmd.user_id, cmd.ctx);
                  } else if (m_auth_state[idx].lock_layer == 2) {
                    m_auth_state[idx].lockout_until_ms = now_ms + LOCK_TIME_L2_MS;
                    if (cmd.cb) cmd.cb(USER_MGR_EVT_ERR_TEMP_LOCKED, cmd.user_id, cmd.ctx);
                  } else if (m_auth_state[idx].lock_layer == 3) {
                    m_auth_state[idx].lockout_until_ms = now_ms + LOCK_TIME_L3_MS;
                    if (cmd.cb) cmd.cb(USER_MGR_EVT_ERR_TEMP_LOCKED, cmd.user_id, cmd.ctx);
                  } else {
                    m_user_db[idx].record.public_data.is_enabled = false;
                    m_auth_state[idx].lock_layer = 4;

                    trigger_db_save = true; 
                    trigger_ui_update = true;
                    if (cmd.cb) cmd.cb(USER_MGR_EVT_ERR_LOCKED, cmd.user_id, cmd.ctx);
                  }
                } else {
                  if (cmd.cb) cmd.cb(USER_MGR_EVT_ERR_WRONG_PIN, cmd.user_id, cmd.ctx);
                }
              }
            }
            break;
          }
          case CmdType::REQ_LOGIN_FACE: {
            if (cmd.cb)
              cmd.cb(USER_MGR_EVT_ERR_INTERNAL, cmd.user_id, cmd.ctx);
            break;
          }

          case CmdType::REQ_LOGOUT: {
            if (m_active_user != USER_MGR_USER_NONE) {
              m_last_user = m_active_user;
              m_active_user = USER_MGR_USER_NONE;
              if (m_global_bridge_cb)
                m_global_bridge_cb(USER_MGR_EVT_GLOBAL_LOGOUT);
            }
            if (cmd.cb)
              cmd.cb(USER_MGR_EVT_SUCCESS, 0, cmd.ctx);
            break;
          }

          case CmdType::REQ_CREATE: {
            size_t input_pin_len = strlen(cmd.payload.create_data.pin);
            if (input_pin_len < USER_MGR_MIN_PIN_VAL ||
                input_pin_len > USER_MGR_MAX_PIN_VAL) {
              ESP_LOGW(TAG, "Rechazado: PIN de longitud invalida (%d)",
                       input_pin_len);
              if (cmd.cb)
                cmd.cb(USER_MGR_EVT_ERR_WRONG_PIN, 0, cmd.ctx);
              break;
            }

            int16_t slot = findFreeSlot();
            if (slot >= 0) {
              if (m_next_id == USER_MGR_USER_NONE) {
                  m_next_id = 1;
              }
              uint16_t new_id = m_next_id++;
              if (new_id == USER_MGR_USER_GUEST)
                new_id = m_next_id++;

              m_user_db[slot].record.public_data.id = new_id;
              strlcpy(m_user_db[slot].record.public_data.name,
                      cmd.payload.create_data.name, USER_MGR_MAX_NAME_LEN);
              strlcpy(m_user_db[slot].record.pin, cmd.payload.create_data.pin,
                      USER_MGR_MAX_PIN_LEN);
              m_user_db[slot].record.public_data.permission =
                  cmd.payload.create_data.permission;
              m_user_db[slot].record.public_data.avatar_id =
                  cmd.payload.create_data.avatar_id;
              m_user_db[slot].record.public_data.pin_length = input_pin_len;
              m_user_db[slot].record.public_data.is_enabled = true;
              m_user_db[slot].is_used = true;
              memset(&m_auth_state[slot], 0, sizeof(UserMgrAuthState));

              trigger_db_save = true;
              trigger_ui_update = true;
              if (cmd.cb)
                cmd.cb(USER_MGR_EVT_SUCCESS,
                       m_user_db[slot].record.public_data.id, cmd.ctx);
            } else {
              if (cmd.cb)
                cmd.cb(USER_MGR_EVT_ERR_DB_FULL, 0, cmd.ctx);
            }
            break;
          }

          case CmdType::REQ_DELETE: {
            // PROTECCIÓN: No se puede eliminar a sí mismo estando logueado.
            if (cmd.user_id == m_active_user) {
              ESP_LOGE(TAG,
                       "Operacion rechazada: No se puede eliminar el usuario "
                       "activo (ID: %d). Debe cerrar sesion primero.",
                       cmd.user_id);
              // Se asume un evento de error, pero la base de datos queda
              // intacta.
              if (cmd.cb)
                cmd.cb(USER_MGR_EVT_ERR_LOCKED, cmd.user_id, cmd.ctx);
              break;
            }

            int16_t idx = findIndexById(cmd.user_id);
            if (idx >= 0) {
              m_user_db[idx].is_used = false;

              trigger_db_save = true;
              trigger_ui_update = true;
              enforceMinimumUsers();

              if (cmd.cb)
                cmd.cb(USER_MGR_EVT_SUCCESS, cmd.user_id, cmd.ctx);
            } else {
              if (cmd.cb)
                cmd.cb(USER_MGR_EVT_ERR_NOT_FOUND, cmd.user_id, cmd.ctx);
            }
            break;
          }

          case CmdType::REQ_GUEST_TOGGLE: {
            int16_t idx = findIndexById(USER_MGR_USER_GUEST);
            if (idx >= 0) {
              m_user_db[idx].record.public_data.is_enabled =
                  cmd.payload.toggle_state;
            } else if (cmd.payload.toggle_state) {
              injectGuest();
            }

            trigger_db_save = true;
            trigger_ui_update = true;
            if (!cmd.payload.toggle_state && enforceMinimumUsers()) {
              ESP_LOGW(TAG, "Revertido: El sistema no puede quedar vacio.");
            }

            if (cmd.cb)
              cmd.cb(USER_MGR_EVT_SUCCESS, USER_MGR_USER_GUEST, cmd.ctx);
            break;
          }

          case CmdType::REQ_SORTED_VIEW: {
            if (cmd.payload.view_cb) {
              // Buffer de copias alineadas (Soluciona el
              // -Waddress-of-packed-member)
              user_mgr_user_view_t unpacked_views[USER_MGR_MAX_USERS];
              user_mgr_user_view_t *view_array[USER_MGR_MAX_USERS];
              uint16_t count = 0;

              for (int i = 0; i < USER_MGR_MAX_USERS; i++) {
                if (m_user_db[i].is_used) {
                  // 1. Copiamos por valor el struct empaquetado al normal
                  unpacked_views[count] = m_user_db[i].record.public_data;
                  // 2. Asignamos el string relativo
                  unpacked_views[count].role_str =
                      getPermissionString(unpacked_views[count].permission);
                  // 3. Guardamos el puntero SEGURO
                  view_array[count] = &unpacked_views[count];
                  count++;
                }
              }

              // Ordenamiento simple para N pequeño (Soluciona el
              // -Werror=array-bounds de std::sort)
              for (uint16_t i = 0; i < count; i++) {
                for (uint16_t j = i + 1; j < count; j++) {
                  if (view_array[j]->id < view_array[i]->id) {
                    user_mgr_user_view_t *temp = view_array[i];
                    view_array[i] = view_array[j];
                    view_array[j] = temp;
                  }
                }
              }

              cmd.payload.view_cb(view_array, count, cmd.ctx);
            }
            break;
          }
          }

          xSemaphoreGiveRecursive(m_mutex);

          if (trigger_db_save)
            saveDb();
          if (trigger_ui_update && m_global_bridge_cb)
            m_global_bridge_cb(USER_MGR_EVT_GLOBAL_DB_CHANGED);
        }
      }
    }

public:
    UserMgrManager(const UserMgrManager&) = delete;
    UserMgrManager& operator=(const UserMgrManager&) = delete;

    static UserMgrManager& getInstance() {
        if (!s_instance) s_instance = new UserMgrManager();
        return *s_instance;
    }

    esp_err_t init(user_mgr_global_cb_t bridge_cb) {
        if (m_initialized) return ESP_OK;
        
        m_global_bridge_cb = bridge_cb;
        m_mutex = xSemaphoreCreateRecursiveMutex();
        m_queue = xQueueCreate(CMD_QUEUE_SIZE, sizeof(SysCmdMsg));
        m_initialized = true; 

        loadDb();
    
        BaseType_t err = xTaskCreate(taskWrapper, "user_mgr_core", 4096, this, 5, nullptr);
        ESP_RETURN_ON_FALSE(err == pdPASS, ESP_FAIL, TAG, "Falló la inicialización async task background commander");

        if (m_global_bridge_cb) {
            m_global_bridge_cb(USER_MGR_EVT_GLOBAL_DB_CHANGED);
        }
        ESP_LOGI(TAG, "User manager initialized successfully");
        return ESP_OK;
    }

    uint16_t getActiveUser() const {
        if (!m_initialized) return USER_MGR_USER_NONE;
        xSemaphoreTakeRecursive(m_mutex, portMAX_DELAY);
        uint16_t id = m_active_user;
        xSemaphoreGiveRecursive(m_mutex);
        return id;
    }

    esp_err_t hasActiveUserSync(bool *out_is_active) const {
        if (!out_is_active || !m_initialized) return ESP_ERR_INVALID_STATE;

        xSemaphoreTakeRecursive(m_mutex, portMAX_DELAY);
        *out_is_active = (m_active_user != USER_MGR_USER_NONE);
        xSemaphoreGiveRecursive(m_mutex);
        return ESP_OK;
    }

    uint16_t getLastUser() const {
        if (!m_initialized) return USER_MGR_USER_NONE;
        xSemaphoreTakeRecursive(m_mutex, portMAX_DELAY);
        // Si nadie está activo ahora, retornamos el último que lo estuvo (o NONE si nunca hubo nadie)
        uint16_t id = (m_active_user != USER_MGR_USER_NONE) ? m_active_user : m_last_user;
        xSemaphoreGiveRecursive(m_mutex);
        return id;
    }

    bool isGuestEnabled() const {
        if (!m_initialized) return false;
        bool enabled = false;
        xSemaphoreTakeRecursive(m_mutex, portMAX_DELAY);
        int16_t idx = findIndexById(USER_MGR_USER_GUEST);
        if (idx >= 0) enabled = m_user_db[idx].record.public_data.is_enabled;
        xSemaphoreGiveRecursive(m_mutex);
        return enabled;
    }

    esp_err_t getUserData(uint16_t user_id, user_mgr_user_view_t* out_data) const {
        if (!m_initialized || !out_data || user_id == USER_MGR_USER_NONE) return ESP_ERR_INVALID_STATE;
        esp_err_t ret = ESP_ERR_NOT_FOUND;
        xSemaphoreTakeRecursive(m_mutex, portMAX_DELAY);
        int16_t idx = findIndexById(user_id);
        if (idx >= 0) {
            *out_data = m_user_db[idx].record.public_data;
            out_data->role_str = getPermissionString(out_data->permission);
            ret = ESP_OK;
        }
        xSemaphoreGiveRecursive(m_mutex);
        return ret;
    }

    esp_err_t enqueueCommand(SysCmdMsg &cmd) {
      if (!m_initialized)
        return ESP_ERR_INVALID_STATE;
      return (xQueueSend(m_queue, &cmd, 0) == pdTRUE) ? ESP_OK : ESP_FAIL;
    }

    const char *getLockMessage(uint16_t user_id) const {
        if (!m_initialized) return "";
        const char *msg = "";

        xSemaphoreTakeRecursive(m_mutex, portMAX_DELAY);
        int16_t idx = findIndexById(user_id);
        
        if (idx >= 0) {
            if (!m_user_db[idx].record.public_data.is_enabled || m_auth_state[idx].lock_layer >= 4) {
                msg = USER_MGR_MSG_LOCK_PERMA; 
            } 
            else if (m_auth_state[idx].lock_layer > 0) {
                if (getCurrentMs() < m_auth_state[idx].lockout_until_ms) {
                    switch (m_auth_state[idx].lock_layer) {
                        case 1: msg = USER_MGR_MSG_LOCK_L1; break;
                        case 2: msg = USER_MGR_MSG_LOCK_L2; break;
                        case 3: msg = USER_MGR_MSG_LOCK_L3; break;
                    }
                }
            }
        }
        xSemaphoreGiveRecursive(m_mutex);
        return msg;
    }

    bool getLockState_isLock(uint16_t user_id) {
        if (!m_initialized) return false;
        
        bool is_locked = false;
        xSemaphoreTakeRecursive(m_mutex, portMAX_DELAY);
        int16_t idx = findIndexById(user_id);
        
        if (idx >= 0) {
            if (!m_user_db[idx].record.public_data.is_enabled || m_auth_state[idx].lock_layer >= 4) {
                is_locked = true;
            } 
            else if (m_auth_state[idx].lock_layer > 0) {
                if (getCurrentMs() < m_auth_state[idx].lockout_until_ms) {
                    is_locked = true;
                } else {
                    is_locked = false;
                }
            }
        }
        xSemaphoreGiveRecursive(m_mutex);
        return is_locked;
    }

    uint32_t getLockRemainingMs(uint16_t user_id) {
        if (!m_initialized) return 0;
        uint32_t remaining = 0;

        xSemaphoreTakeRecursive(m_mutex, portMAX_DELAY);
        
        int16_t idx = findIndexById(user_id);
        if (idx >= 0 && m_auth_state[idx].lock_layer > 0 && m_auth_state[idx].lock_layer < 4) {
            uint64_t now = getCurrentMs();
            if (now < m_auth_state[idx].lockout_until_ms) {
                remaining = (uint32_t)(m_auth_state[idx].lockout_until_ms - now);
            }
        }
        xSemaphoreGiveRecursive(m_mutex);
        return remaining;
    }
};

UserMgrManager* UserMgrManager::s_instance = nullptr;

// ============================================================================
// FACHADA DE API EN C PARA LVGL (Wrapper C)
// ============================================================================

extern "C" {

esp_err_t user_mgr_init(user_mgr_global_cb_t ui_bridge_listener) {
    return UserMgrManager::getInstance().init(ui_bridge_listener);
}

uint16_t user_mgr_sess_get_active(void) { return UserMgrManager::getInstance().getActiveUser(); }
esp_err_t user_mgr_sess_has_active_user_sync(bool *out_is_active) {
    return UserMgrManager::getInstance().hasActiveUserSync(out_is_active);
}
const char* user_mgr_auth_get_lock_msg(uint16_t user_id) { return UserMgrManager::getInstance().getLockMessage(user_id); }
bool user_mgr_auth_get_lock_temp_isLocked(uint16_t user_id) { return UserMgrManager::getInstance().getLockState_isLock(user_id); }
uint32_t user_mgr_auth_get_lock_remaining_ms(uint16_t user_id) { return UserMgrManager::getInstance().getLockRemainingMs(user_id); }
uint16_t user_mgr_sess_get_last(void) { return UserMgrManager::getInstance().getLastUser(); }
bool user_mgr_user_guest_is_enabled(void) { return UserMgrManager::getInstance().isGuestEnabled(); }

esp_err_t user_mgr_user_get_data(uint16_t user_id, user_mgr_user_view_t *out_data) {
    return UserMgrManager::getInstance().getUserData(user_id, out_data);
}

esp_err_t user_mgr_sess_req_login(uint16_t user_id, const char *pin, user_mgr_cb_t cb, void *ctx) {
    SysCmdMsg cmd{};
    cmd.type = CmdType::REQ_LOGIN;
    cmd.user_id = user_id;
    cmd.cb = cb;
    cmd.ctx = ctx;
    if(pin) strlcpy(cmd.payload.pin, pin, USER_MGR_MAX_PIN_LEN);
    return UserMgrManager::getInstance().enqueueCommand(cmd);
}

esp_err_t user_mgr_sess_req_login_face(uint16_t user_id, user_mgr_face_data_t *face, user_mgr_cb_t cb, void *ctx) {
    SysCmdMsg cmd{};
    cmd.type = CmdType::REQ_LOGIN_FACE;
    cmd.user_id = user_id;
    cmd.cb = cb;
    cmd.ctx = ctx;
    if(face) cmd.payload.face_data = *face;
    return UserMgrManager::getInstance().enqueueCommand(cmd);
}

esp_err_t user_mgr_auth_req_validate_pin(uint16_t user_id, const char *pin, user_mgr_cb_t cb, void *ctx) {
    SysCmdMsg cmd{};
    cmd.type = CmdType::REQ_VALIDATE;
    cmd.user_id = user_id;
    cmd.cb = cb;
    cmd.ctx = ctx;
    if(pin) strlcpy(cmd.payload.pin, pin, USER_MGR_MAX_PIN_LEN);
    return UserMgrManager::getInstance().enqueueCommand(cmd);
}

esp_err_t user_mgr_sess_req_logout(user_mgr_cb_t cb, void *ctx) {
    SysCmdMsg cmd{};
    cmd.type = CmdType::REQ_LOGOUT;
    cmd.cb = cb;
    cmd.ctx = ctx;
    return UserMgrManager::getInstance().enqueueCommand(cmd);
}

esp_err_t user_mgr_user_req_create(const user_mgr_user_create_t *data, user_mgr_cb_t cb, void *ctx) {
    if(!data) return ESP_ERR_INVALID_ARG;
    SysCmdMsg cmd{};
    cmd.type = CmdType::REQ_CREATE;
    cmd.cb = cb;
    cmd.ctx = ctx;
    cmd.payload.create_data = *data;
    return UserMgrManager::getInstance().enqueueCommand(cmd);
}

esp_err_t user_mgr_user_req_delete(uint16_t user_id, user_mgr_cb_t cb, void *ctx) {
    SysCmdMsg cmd{};
    cmd.type = CmdType::REQ_DELETE;
    cmd.user_id = user_id;
    cmd.cb = cb;
    cmd.ctx = ctx;
    return UserMgrManager::getInstance().enqueueCommand(cmd);
}

esp_err_t user_mgr_user_req_guest_toggle(bool enable, user_mgr_cb_t cb, void *ctx) {
    SysCmdMsg cmd{};
    cmd.type = CmdType::REQ_GUEST_TOGGLE;
    // IMPORTANTE: Asegurate que USER_MGR_USER_GUEST exista en el .h y no sea 0
    cmd.user_id = USER_MGR_USER_GUEST; 
    cmd.cb = cb;
    cmd.ctx = ctx;
    cmd.payload.toggle_state = enable;
    return UserMgrManager::getInstance().enqueueCommand(cmd);
}

esp_err_t user_mgr_user_req_sorted_view(user_mgr_view_cb_t cb, void *ctx) {
    if(!cb) return ESP_ERR_INVALID_ARG;
    SysCmdMsg cmd{};
    cmd.type = CmdType::REQ_SORTED_VIEW;
    cmd.ctx = ctx;
    cmd.payload.view_cb = cb;
    return UserMgrManager::getInstance().enqueueCommand(cmd);
}

} // extern "C"
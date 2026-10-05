/**
 * @file jpegc_element.h
 * @brief Interfaz del decodificador JPEG asíncrono para imágenes LVGL.
 *
 * Declara la configuración del decodificador, los formatos de salida, los
 * callbacks de finalización y las funciones para crear handles y encolar
 * trabajos de decodificación. El procesamiento se ejecuta en una tarea
 * interna y los buffers de salida se gestionan en PSRAM.
 */

#pragma once

/* ESP */
#include <stdint.h>
#include "esp_err.h"

/* Módulos */
#include "lvgl.h"
#include "desc_apps.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ── Task / queue hardcoded defaults ─────────────────────────────────────── */
#define JPEGC_ELEMENT_TASK_STACK_SIZE   (8192U)   /**< FreeRTOS task stack size in bytes  */
#define JPEGC_ELEMENT_TASK_PRIORITY     (5U)       /**< FreeRTOS task priority             */
#define JPEGC_ELEMENT_QUEUE_DEPTH       (8U)       /**< Max jobs pending in the FIFO queue */
#define JPEGC_ELEMENT_MAX_HANDLES       (8U)       /**< Max simultaneous handles allowed   */

/* ── Forward declaration / opaque handle ─────────────────────────────────── */
typedef struct jpegc_element_t *jpegc_element_handle_t;

/* ── Callbacks ────────────────────────────────────────────────────────────── */

/**
 * @brief Called from the decode task immediately BEFORE decoding starts.
 *
 * CRITICAL: Use this callback to signal LVGL to release the previous image.
 * For example, call lv_image_set_src(obj, NULL) to clear the old descriptor
 * from the display before the new decode begins. This prevents LVGL from
 * reading stale buffer data while this handle is reused for a new decode.
 *
 * @note If this is the first decode on the handle, no previous image exists.
 */
typedef void (*jpegc_element_cb_pre_t)(lv_obj_t *target);

/** Payload delivered to the post-decode callback. */
typedef struct {
    lv_img_dsc_t *img_dsc;  /**< Ready-to-use LVGL descriptor (points to internal PSRAM buf). */
    esp_err_t     result;   /**< ESP_OK on success, error code otherwise.                      */
    lv_obj_t     *target_obj; /**< Optional LVGL object passed in process_data                  */
} jpegc_element_cb_post_data_t;

/**
 * @brief Called from the decode task once decoding is complete (or failed).
 *
 * CRITICAL: On success (result == ESP_OK), call lv_image_set_src(target_obj, img_dsc)
 * to attach the newly decoded image to the LVGL object. The img_dsc pointer is stable
 * and owned by this handle for its entire lifetime. LVGL will hold a reference to it
 * until you call the pre_cb on the next decode (or cleanup the handle).
 *
 * @note On failure, do not use img_dsc; check the result code first.
 */
typedef void (*jpegc_element_cb_post_t)(jpegc_element_cb_post_data_t *data);

/* ── Output format ────────────────────────────────────────────────────────── */

/**
 * @brief Desired pixel format of the decoded image.
 *
 * Maps internally to jpeg_dec_output_format_t from esp_driver_jpeg.
 */
typedef enum {
    JPEGC_OUTPUT_FORMAT_RGB565   = 0, /**< 2 bytes/px  – direct LVGL RGB565          */
    JPEGC_OUTPUT_FORMAT_RGB888   = 1, /**< 3 bytes/px  – direct LVGL RGB888          */
    JPEGC_OUTPUT_FORMAT_YCbCr422 = 2, /**< 2 bytes/px  – YUV422, LVGL RAW            */
    JPEGC_OUTPUT_FORMAT_GRAY     = 3, /**< 1 byte/px   – LVGL L8 (luminance)         */
} jpegc_element_output_format_t;

/* ── Process data (per-call) ──────────────────────────────────────────────── */

/**
 * @brief Per-process call data.
 *
 * Only the filename (or relative sub-path) is needed here; the full VFS path
 * is assembled internally from fs_letter + fs_default_dir + input_path.
 */
typedef struct {
    const char *input_path; /**< File name, e.g. "splash.jpg". No leading slash needed. */
    lv_obj_t  *target_obj; /**< Optional LVGL object to apply the image to (nullable). */
} jpegc_element_process_data_t;

/* ── Handle configuration ─────────────────────────────────────────────────── */

/**
 * @brief Configuration applied once at handle creation.
 *
 * buff_kb rules:
 *   -1  → output buffer is allocated automatically on the first decode, using
 *          the actual image dimensions. The size is then fixed for the life of
 *          the handle (no realloc). A larger image later → log error + cb error.
 *   >0  → output buffer is allocated immediately with buff_kb * 1024 bytes.
 *
 * extra_kb:
 *   Added on top of the computed/requested size in both auto and fixed modes.
 *   0 has no effect.
 *
 * All output buffers are allocated in PSRAM, 16-byte aligned (DMA-safe).
 */
typedef struct {
    int32_t  buff_kb;    /**< Output buffer size in KB. -1 = auto-size on first decode.   */
    uint32_t extra_kb;   /**< Extra KB added to the output buffer. 0 = no effect.          */

    jpegc_element_output_format_t output_format; /**< Decoded pixel format.               */

    int intr_priority;   /**< JPEG HW interrupt priority (0 = driver default 1-3).         */
    int timeout_ms;      /**< Decode timeout in ms. -1 = wait forever.                     */

    const char *fs_letter;      /**< VFS mount point, e.g. "/spiffs".                      */
    const char *fs_default_dir; /**< Sub-directory, e.g. "images". Empty string for root.  */

    jpegc_element_cb_pre_t  pre_cb;  /**< Pre-decode callback  (nullable).                 */
    jpegc_element_cb_post_t post_cb; /**< Post-decode callback (nullable).                 */
} jpegc_element_config_t;

/* ── Public API ───────────────────────────────────────────────────────────── */

/**
 * @brief Initialise the global JPEG element subsystem.
 *
 * Creates the internal FreeRTOS FIFO task and queue.
 * Must be called once before any handle is created.
 *
 */
app_desc_status_t* jpegc_element_init(void);

/**
 * @brief Create and configure a new decoder handle.
 *
 * Allocates the internal struct, acquires a dedicated JPEG HW engine instance,
 * and (if buff_kb != -1) pre-allocates the output buffer in PSRAM.
 *
 * @param[out] handle  Receives the newly created handle.
 * @param[in]  config  Handle configuration (copied internally).
 * @return ESP_OK | ESP_ERR_INVALID_ARG | ESP_ERR_INVALID_STATE |
 *         ESP_ERR_NO_MEM | ESP_FAIL
 */
esp_err_t jpegc_element_handle_create(jpegc_element_handle_t *handle,
                                       jpegc_element_config_t  *config);

/**
 * @brief Enqueue a JPEG decode job for the given handle.
 *
 * The job is processed asynchronously by the internal FIFO task.
 * Results are delivered via the post_cb configured in the handle.
 *
 * @note The handle must have been created with jpegc_element_handle_create.
 *       Calling process on a cleaned-up handle returns ESP_ERR_INVALID_STATE.
 *
 * @param[in] handle  A configured handle.
 * @param[in] data    Process data containing the filename.
 * @return ESP_OK | ESP_ERR_INVALID_ARG | ESP_ERR_INVALID_STATE | ESP_FAIL
 */
esp_err_t jpegc_element_process(jpegc_element_handle_t       handle,
                                 jpegc_element_process_data_t *data);

/**
 * @brief Release all resources owned by a handle.
 *
 * Frees the output buffer, destroys the internal JPEG HW engine, and resets
 * the handle to an unconfigured state (equivalent to a freshly created but
 * unconfigured handle).  The handle struct itself is NOT freed; only
 * jpegc_element_deinit() frees all handle memory.
 *
 * CRITICAL PRECONDITION: Before calling cleanup, ensure LVGL no longer holds
 * a reference to the descriptor. Either:
 *   a) Call the pre_cb directly to signal LVGL to release the image, OR
 *   b) Know that no LVGL object is currently displaying the image.
 *
 * Calling cleanup while LVGL is actively rendering the buffer will cause
 * use-after-free and heap corruption.
 *
 * @note The handle cannot be reused after cleanup without calling deinit and
 *       re-initialising the subsystem.
 *
 * @param[in] handle  Handle to clean up.
 * @return ESP_OK on success, ESP_FAIL if an internal sub-operation failed.
 */
esp_err_t jpegc_element_cleanup(jpegc_element_handle_t handle);

/**
 * @brief Tear down the entire JPEG element subsystem.
 *
 * Stops and deletes the FIFO task, flushes and deletes the queue, calls
 * cleanup on every registered handle, and frees all allocated memory.
 * After this call the subsystem can be re-initialised with jpegc_element_init.
 *
 * @return ESP_OK
 */
esp_err_t jpegc_element_deinit(void);

#ifdef __cplusplus
}
#endif

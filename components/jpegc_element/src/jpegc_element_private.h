#pragma once

/*
 * jpegc_element – private internals.
 * Not to be included by any external code.
 */

#include <stdbool.h>
#include <stdint.h>

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"

#include "driver/jpeg_decode.h"

#include "jpegc_element.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ── Internal constants ───────────────────────────────────────────────────── */

/** DMA alignment for every PSRAM buffer managed by this wrapper (bytes). */
#define JPEGC_DMA_ALIGN         (16U)

/** Maximum length of the assembled VFS path (fs_letter + dir + filename). */
#define JPEGC_FILEPATH_MAX      (512U)

/* ── Per-handle internal structure ───────────────────────────────────────── */

/**
 * @brief Opaque handle implementation.
 *
 * One instance per jpegc_element_handle_create() call.
 * All resources owned exclusively by this struct; nobody outside the wrapper
 * should touch them directly.
 */
struct jpegc_element_t {
    jpeg_decoder_handle_t  decoder;       /**< HW JPEG decoder engine (one per handle).       */
    jpegc_element_config_t config;        /**< Deep copy of the user-supplied config.          */

    uint8_t               *out_buf;       /**< PSRAM output buffer (16-byte DMA aligned).      */
    size_t                 out_buf_size;  /**< Allocated size of out_buf in bytes.             */
    bool                   buf_size_fixed;/**< true once output buffer size is determined.     */

    lv_img_dsc_t           img_dsc;      /**< LVGL image descriptor reused across decodes.    */
    bool                   configured;   /**< false after cleanup, true after handle_create.  */
};

/* ── Job queued between process() and the decode task ────────────────────── */

/**
 * @brief A single decode job pushed into the FIFO queue.
 *
 * The filepath is assembled once in jpegc_element_process() so the task does
 * not need access to the config again to reconstruct it.
 */
typedef struct {
    jpegc_element_handle_t handle;
    char                   filepath[JPEGC_FILEPATH_MAX];
} jpegc_element_job_t;

/* ── Global subsystem context ─────────────────────────────────────────────── */

/**
 * @brief Singleton holding the global FreeRTOS resources and handle registry.
 *
 * Only one instance exists (static in jpegc_element.c).
 */
typedef struct {
    QueueHandle_t           queue;        /**< FreeRTOS queue (FIFO, depth JPEGC_ELEMENT_QUEUE_DEPTH). */
    TaskHandle_t            task;         /**< Single decode task consuming the queue.                 */
    bool                    initialized;  /**< Prevents double-init and guards process/create.         */

    /** Registry of every handle created since the last init.
     *  deinit() iterates this to free everything. */
    jpegc_element_handle_t  handles[JPEGC_ELEMENT_MAX_HANDLES];
    uint8_t                 handle_count;
} jpegc_element_ctx_t;

#ifdef __cplusplus
}
#endif

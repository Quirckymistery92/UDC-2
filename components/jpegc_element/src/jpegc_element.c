/*
 * jpegc_element.c
 *
 * Hardware JPEG decode wrapper for ESP32-P4.
 * Uses esp_driver_jpeg (jpeg_new_decoder_engine / jpeg_decoder_process) with
 * output delivered as a ready-to-use lv_img_dsc_t in PSRAM.
 *
 * Architecture:
 *   - One global FreeRTOS task consumes a FIFO queue → serial decode order.
 *   - Each handle owns its own JPEG HW engine instance + PSRAM output buffer.
 *   - No heap allocation is done inside the wrapper outside of init/create.
 *     (The per-job input buffer read from SPIFFS is alloc'd/free'd per decode.)
 */

#include <string.h>
#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "esp_log.h"
#include "esp_err.h"
#include "esp_heap_caps.h"

#include "driver/jpeg_decode.h"

#include "jpegc_element.h"
#include "jpegc_element_private.h"

/* ── Module tag ──────────────────────────────────────────────────────────── */
static const char *TAG = "jpegc";

/* ── Singleton context ───────────────────────────────────────────────────── */
static jpegc_element_ctx_t s_ctx = {0};

/* ═══════════════════════════════════════════════════════════════════════════
 * Internal helpers
 * ═══════════════════════════════════════════════════════════════════════════ */

/**
 * @brief Bytes-per-pixel for the selected output format.
 */
static inline uint8_t priv_bpp(jpegc_element_output_format_t fmt)
{
    switch (fmt) {
        case JPEGC_OUTPUT_FORMAT_RGB565:   return 2;
        case JPEGC_OUTPUT_FORMAT_RGB888:   return 3;
        case JPEGC_OUTPUT_FORMAT_YCbCr422: return 2;
        case JPEGC_OUTPUT_FORMAT_GRAY:     return 1;
        default:                           return 2;
    }
}

/**
 * @brief Map wrapper enum → esp_driver_jpeg decode output format.
 */
static inline jpeg_dec_output_format_t priv_map_dec_format(jpegc_element_output_format_t fmt)
{
    switch (fmt) {
        case JPEGC_OUTPUT_FORMAT_RGB565:   return JPEG_DECODE_OUT_FORMAT_RGB565;
        case JPEGC_OUTPUT_FORMAT_RGB888:   return JPEG_DECODE_OUT_FORMAT_RGB888;
        case JPEGC_OUTPUT_FORMAT_YCbCr422: return JPEG_DECODE_OUT_FORMAT_YUV422;
        case JPEGC_OUTPUT_FORMAT_GRAY:     return JPEG_DECODE_OUT_FORMAT_GRAY;
        default:                           return JPEG_DECODE_OUT_FORMAT_RGB565;
    }
}

static inline lv_color_format_t priv_map_lv_format(jpegc_element_output_format_t fmt)
{
    switch (fmt) {
        case JPEGC_OUTPUT_FORMAT_RGB565:   return LV_COLOR_FORMAT_RGB565;
        case JPEGC_OUTPUT_FORMAT_RGB888:   return LV_COLOR_FORMAT_RGB888;
        case JPEGC_OUTPUT_FORMAT_YCbCr422: return LV_COLOR_FORMAT_RAW;
        case JPEGC_OUTPUT_FORMAT_GRAY:     return LV_COLOR_FORMAT_L8;
        default:                           return LV_COLOR_FORMAT_RGB565;
    }
}

static uint8_t *priv_alloc_output_buf(size_t min_size, size_t *actual_size)
{
    jpeg_decode_memory_alloc_cfg_t cfg = {
        .buffer_direction = JPEG_DEC_ALLOC_OUTPUT_BUFFER,
    };
    void *ptr = jpeg_alloc_decoder_mem(min_size, &cfg, actual_size);
    if (ptr) {
        memset(ptr, 0, *actual_size);
    }
    return (uint8_t *)ptr;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Decode task (FIFO consumer)
 * ═══════════════════════════════════════════════════════════════════════════ */

static void priv_decode_task(void *arg)
{
    (void)arg;
    jpegc_element_job_t job;

    for (;;) {
        /* Block until a job arrives. */
        if (xQueueReceive(s_ctx.queue, &job, portMAX_DELAY) != pdTRUE) {
            continue;
        }
        
        jpegc_element_handle_t h = job.handle;

        /* ── Pre-decode callback ─────────────────────────────────────────── */
        if (h->config.pre_cb) {
            h->config.pre_cb(job.target_obj);
        }

        /* Prepare a default post_data; overwritten on success. */
        jpegc_element_cb_post_data_t post_data = {
            .img_dsc = &h->img_dsc,
            .result  = ESP_FAIL,
        };

#define DELIVER_ERROR(err_code) \
        do { \
            post_data.result = (err_code); \
            if (h->config.post_cb) { h->config.post_cb(&post_data); } \
        } while (0)

        /* ── Open file ───────────────────────────────────────────────────── */
        FILE *f = fopen(job.filepath, "rb");
        if (!f) {
            ESP_LOGE(TAG, "Cannot open file: %s", job.filepath);
            DELIVER_ERROR(ESP_FAIL);
            continue;
        }

        fseek(f, 0, SEEK_END);
        long file_size = ftell(f);
        fseek(f, 0, SEEK_SET);

        if (file_size <= 0) {
            ESP_LOGE(TAG, "Invalid file size (%ld) for: %s", file_size, job.filepath);
            fclose(f);
            DELIVER_ERROR(ESP_FAIL);
            continue;
        }

        /* ── Allocate input buffer (PSRAM, DMA-ready via driver helper) ─── */
        jpeg_decode_memory_alloc_cfg_t in_mem_cfg = {
            .buffer_direction = JPEG_DEC_ALLOC_INPUT_BUFFER,
        };
        size_t in_actual = 0;
        uint8_t *in_buf = (uint8_t *)jpeg_alloc_decoder_mem((size_t)file_size,
                                                              &in_mem_cfg,
                                                              &in_actual);
        if (!in_buf) {
            ESP_LOGE(TAG, "Cannot alloc input buffer (%ld B) for: %s", file_size, job.filepath);
            fclose(f);
            DELIVER_ERROR(ESP_ERR_NO_MEM);
            continue;
        }

        /* ── Read file into input buffer ─────────────────────────────────── */
        size_t bytes_read = fread(in_buf, 1, (size_t)file_size, f);
        fclose(f);

        if (bytes_read != (size_t)file_size) {
            ESP_LOGE(TAG, "Short read %zu/%ld for: %s", bytes_read, file_size, job.filepath);
            heap_caps_free(in_buf);
            DELIVER_ERROR(ESP_FAIL);
            continue;
        }

        /* ── Parse JPEG header (no HW needed) ───────────────────────────── */
        jpeg_decode_picture_info_t pic_info = {0};
        esp_err_t ret = jpeg_decoder_get_info(in_buf, (uint32_t)bytes_read, &pic_info);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "jpeg_decoder_get_info failed (%s) for: %s",
                     esp_err_to_name(ret), job.filepath);
            heap_caps_free(in_buf);
            DELIVER_ERROR(ret);
            continue;
        }

        /* ── Compute required output size ────────────────────────────────── */
        const uint8_t  bpp       = priv_bpp(h->config.output_format);
        const uint32_t w_aligned = (pic_info.width  + 15U) & ~15U;
        const uint32_t h_aligned = (pic_info.height + 15U) & ~15U;
        const size_t   needed    = (size_t)w_aligned * h_aligned * bpp
                                   + (size_t)h->config.extra_kb * 1024U;

        /* ── Auto-allocate output buffer on first decode (buff_kb == -1) ── */
        if (!h->buf_size_fixed) {
            h->out_buf = priv_alloc_output_buf(needed, &h->out_buf_size);
            if (!h->out_buf) {
                ESP_LOGE(TAG, "Cannot alloc auto output buffer (%zu B) in PSRAM for: %s",
                         needed, job.filepath);
                heap_caps_free(in_buf);
                DELIVER_ERROR(ESP_ERR_NO_MEM);
                continue;
            }
            h->buf_size_fixed = true;
            ESP_LOGI(TAG, "[%p] Auto output buffer: %zu B (w=%lu h=%lu bpp=%u)",
                     (void *)h, h->out_buf_size, (unsigned long)pic_info.width,
                     (unsigned long)pic_info.height, bpp);
        }

        /* ── Check the (fixed) output buffer is large enough ────────────── */
        if (needed > h->out_buf_size) {
            ESP_LOGE(TAG,
                     "[%p] Output buffer too small: need %zu B, have %zu B "
                     "(img %lux%lu, bpp %u). Buffer is fixed – no realloc.",
                     (void *)h, needed, h->out_buf_size,
                     (unsigned long)pic_info.width, (unsigned long)pic_info.height, bpp);
            heap_caps_free(in_buf);
            DELIVER_ERROR(ESP_ERR_NO_MEM);
            continue;
        }

        /* ── HW decode ───────────────────────────────────────────────────── */
        jpeg_decode_cfg_t dec_cfg = {
            .output_format = priv_map_dec_format(h->config.output_format),
            .rgb_order     = (h->config.output_format == JPEGC_OUTPUT_FORMAT_RGB565)
                               ? JPEG_DEC_RGB_ELEMENT_ORDER_BGR
                               : JPEG_DEC_RGB_ELEMENT_ORDER_RGB,
            .conv_std      = JPEG_YUV_RGB_CONV_STD_BT601,
        };

        uint32_t out_size = 0;
        ret = jpeg_decoder_process(h->decoder,
                                   &dec_cfg,
                                   in_buf,       (uint32_t)bytes_read,
                                   h->out_buf,   (uint32_t)h->out_buf_size,
                                   &out_size);

        /* Verify decoder result and avoid overruns into adjacent heap metadata. */
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "[%p] jpeg_decoder_process failed: %s for: %s",
                     (void *)h, esp_err_to_name(ret), job.filepath);
            heap_caps_free(in_buf);
            DELIVER_ERROR(ret);
            continue;
        }

        if ((size_t)out_size > h->out_buf_size) {
            ESP_LOGE(TAG, "[%p] Decoder reported out_size=%u > allocated=%zu for: %s",
                     (void *)h, out_size, h->out_buf_size, job.filepath);
            heap_caps_free(in_buf);
            DELIVER_ERROR(ESP_ERR_NO_MEM);
            continue;
        }

        heap_caps_free(in_buf);   /* Input buffer no longer needed. */

        /* ── Populate lv_img_dsc_t ───────────────────────────────────────── */
        h->img_dsc.header.magic  = LV_IMAGE_HEADER_MAGIC;
        h->img_dsc.header.cf     = (uint32_t)priv_map_lv_format(h->config.output_format);
        h->img_dsc.header.flags  = 0;
        h->img_dsc.header.w      = (uint32_t)pic_info.width;
        h->img_dsc.header.h      = (uint32_t)pic_info.height;
        h->img_dsc.header.stride = (uint32_t)pic_info.width * bpp;
        h->img_dsc.data_size     = out_size;
        h->img_dsc.data          = (const uint8_t *)h->out_buf;

        /* ── Deliver to caller ───────────────────────────────────────────── */
        post_data.result  = ESP_OK;
        post_data.img_dsc = &h->img_dsc;
        post_data.target_obj = job.target_obj;
        if (h->config.post_cb) {
            h->config.post_cb(&post_data);
        }

#undef DELIVER_ERROR
    }
    /* Unreachable – task is deleted via vTaskDelete in deinit. */
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Public API
 * ═══════════════════════════════════════════════════════════════════════════ */

esp_err_t jpegc_element_init(void)
{
    if (s_ctx.initialized) {
        ESP_LOGW(TAG, "init: already initialised");
        return ESP_OK;
    }

    s_ctx.queue = xQueueCreate(JPEGC_ELEMENT_QUEUE_DEPTH, sizeof(jpegc_element_job_t));
    if (!s_ctx.queue) {
        ESP_LOGE(TAG, "init: failed to create queue");
        return ESP_ERR_NO_MEM;
    }

    BaseType_t ok = xTaskCreate(priv_decode_task,
                                "jpegc_task",
                                JPEGC_ELEMENT_TASK_STACK_SIZE,
                                NULL,
                                JPEGC_ELEMENT_TASK_PRIORITY,
                                &s_ctx.task);
    if (ok != pdPASS) {
        ESP_LOGE(TAG, "init: failed to create task");
        vQueueDelete(s_ctx.queue);
        s_ctx.queue = NULL;
        return ESP_FAIL;
    }

    s_ctx.initialized    = true;
    s_ctx.handle_count   = 0;
    ESP_LOGI(TAG, "Initialised (queue depth=%u, task prio=%u, stack=%u B)",
             JPEGC_ELEMENT_QUEUE_DEPTH,
             JPEGC_ELEMENT_TASK_PRIORITY,
             JPEGC_ELEMENT_TASK_STACK_SIZE);
    return ESP_OK;
}

/* ─────────────────────────────────────────────────────────────────────────── */

esp_err_t jpegc_element_handle_create(jpegc_element_handle_t *handle,
                                       jpegc_element_config_t  *config)
{
    if (!handle || !config) {
        ESP_LOGE(TAG, "handle_create: null argument");
        return ESP_ERR_INVALID_ARG;
    }
    if (!s_ctx.initialized) {
        ESP_LOGE(TAG, "handle_create: subsystem not initialised – call jpegc_element_init first");
        return ESP_ERR_INVALID_STATE;
    }
    if (s_ctx.handle_count >= JPEGC_ELEMENT_MAX_HANDLES) {
        ESP_LOGE(TAG, "handle_create: reached max handle limit (%u)", JPEGC_ELEMENT_MAX_HANDLES);
        return ESP_ERR_NO_MEM;
    }

    /* Allocate handle struct in internal RAM (not PSRAM). */
    struct jpegc_element_t *h = (struct jpegc_element_t *)
        heap_caps_calloc(1, sizeof(struct jpegc_element_t), MALLOC_CAP_DEFAULT);
    if (!h) {
        ESP_LOGE(TAG, "handle_create: failed to allocate handle struct");
        return ESP_ERR_NO_MEM;
    }
    
    h->config = *config;
    jpeg_decode_engine_cfg_t eng_cfg = {
        .intr_priority = config->intr_priority,
        .timeout_ms    = config->timeout_ms,
    };
    esp_err_t ret = jpeg_new_decoder_engine(&eng_cfg, &h->decoder);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "handle_create: jpeg_new_decoder_engine failed: %s", esp_err_to_name(ret));
        heap_caps_free(h);
        return ret;
    }

    if (config->buff_kb != -1) {
        size_t req = ((size_t)(uint32_t)config->buff_kb + (size_t)config->extra_kb) * 1024U;
        h->out_buf = priv_alloc_output_buf(req, &h->out_buf_size);
        if (!h->out_buf) {
            ESP_LOGE(TAG, "handle_create: failed to alloc output buffer (%zu B) in PSRAM", req);
            jpeg_del_decoder_engine(h->decoder);
            heap_caps_free(h);
            return ESP_ERR_NO_MEM;
        }
        h->buf_size_fixed = true;
        ESP_LOGI(TAG, "[%p] Pre-allocated output buffer: %zu B", (void *)h, h->out_buf_size);
    }
    /* else: buf_size_fixed stays false → auto-alloc on first decode */

    h->configured = true;
    s_ctx.handles[s_ctx.handle_count++] = h;
    *handle = h;

    ESP_LOGI(TAG, "[%p] Handle created (buff_kb=%ld, extra_kb=%lu, fmt=%d)",
             (void *)h, (long)config->buff_kb, (unsigned long)config->extra_kb,
             (int)config->output_format);
    return ESP_OK;
}

/* ─────────────────────────────────────────────────────────────────────────── */

esp_err_t jpegc_element_process(jpegc_element_handle_t       handle,
                                 jpegc_element_process_data_t *data)
{
    if (!handle || !data || !data->input_path) {
        ESP_LOGE(TAG, "process: invalid arguments");
        return ESP_ERR_INVALID_ARG;
    }
    if (!s_ctx.initialized) {
        ESP_LOGE(TAG, "process: subsystem not initialised");
        return ESP_ERR_INVALID_STATE;
    }
    if (!handle->configured) {
        ESP_LOGE(TAG, "process: [%p] handle is not configured (already cleaned up?)", (void *)handle);
        return ESP_ERR_INVALID_STATE;
    }

    /* Build the full VFS path once here; the task receives it ready. */
    jpegc_element_job_t job;
    job.handle = handle;
    job.target_obj = data->target_obj;

    const char *letter = handle->config.fs_letter      ? handle->config.fs_letter      : "";
    const char *dir    = handle->config.fs_default_dir ? handle->config.fs_default_dir : "";

    if (dir[0] != '\0') {
        snprintf(job.filepath, JPEGC_FILEPATH_MAX, "%s/%s/%s",
                 letter, dir, data->input_path);
    } else {
        snprintf(job.filepath, JPEGC_FILEPATH_MAX, "%s/%s",
                 letter, data->input_path);
    }

    /* Enqueue with a 1-second timeout to detect backpressure. */
    if (xQueueSend(s_ctx.queue, &job, pdMS_TO_TICKS(1000)) != pdTRUE) {
        ESP_LOGE(TAG, "process: queue full – could not enqueue job for '%s'", job.filepath);
        return ESP_FAIL;
    }

    return ESP_OK;
}

/* ─────────────────────────────────────────────────────────────────────────── */

esp_err_t jpegc_element_cleanup(jpegc_element_handle_t handle)
{
    if (!handle) {
        ESP_LOGE(TAG, "cleanup: null handle");
        return ESP_FAIL;
    }

    esp_err_t ret = ESP_OK;

    /* Release output buffer. */
    if (handle->out_buf) {
        heap_caps_free(handle->out_buf);
        handle->out_buf       = NULL;
        handle->out_buf_size  = 0;
        handle->buf_size_fixed = false;
    }

    /* Destroy the HW decoder engine. */
    if (handle->decoder) {
        esp_err_t del_ret = jpeg_del_decoder_engine(handle->decoder);
        if (del_ret != ESP_OK) {
            ESP_LOGE(TAG, "cleanup: [%p] jpeg_del_decoder_engine failed: %s",
                     (void *)handle, esp_err_to_name(del_ret));
            ret = ESP_FAIL;
        }
        handle->decoder = NULL;
    }

    /* Clear the LVGL descriptor and mark handle as unconfigured. */
    memset(&handle->img_dsc, 0, sizeof(handle->img_dsc));
    handle->configured = false;

    ESP_LOGI(TAG, "[%p] Handle cleaned up", (void *)handle);
    return ret;
}

/* ─────────────────────────────────────────────────────────────────────────── */

esp_err_t jpegc_element_deinit(void)
{
    if (!s_ctx.initialized) {
        ESP_LOGW(TAG, "deinit: not initialised – nothing to do");
        return ESP_OK;
    }

    /* Stop the decode task first to prevent in-flight jobs from accessing
     * handles that are about to be freed. */
    if (s_ctx.task) {
        vTaskDelete(s_ctx.task);
        s_ctx.task = NULL;
    }

    /* Delete the queue (any pending jobs are dropped). */
    if (s_ctx.queue) {
        vQueueDelete(s_ctx.queue);
        s_ctx.queue = NULL;
    }

    /* Cleanup and free every registered handle. */
    for (uint8_t i = 0; i < s_ctx.handle_count; i++) {
        if (s_ctx.handles[i]) {
            jpegc_element_cleanup(s_ctx.handles[i]);
            heap_caps_free(s_ctx.handles[i]);
            s_ctx.handles[i] = NULL;
        }
    }
    s_ctx.handle_count = 0;
    s_ctx.initialized  = false;

    ESP_LOGI(TAG, "Deinitialised");
    return ESP_OK;
}

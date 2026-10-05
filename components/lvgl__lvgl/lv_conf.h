#ifndef LV_CONF_H
#define LV_CONF_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/*---------------------------------------------------------------------------
 * Hardware memory-alignment attribute for ALL image/font data arrays.
 *
 * LVGL decorates every generated asset array with LV_ATTRIBUTE_MEM_ALIGN.
 * If left empty (the default) the linker places arrays without alignment
 * constraints, so lv_draw_buf_init() emits:
 *   [Info] lv_draw_buf_init: Data is not aligned, ignored
 * and the PPA / DMA2D hardware cannot use those buffers as source.
 *
 * We tie this to CONFIG_LV_DRAW_BUF_ALIGN (set in sdkconfig, = 64 on
 * ESP32-P4 with PPA enabled) so the value tracks any future Kconfig change
 * without manual editing.
 *---------------------------------------------------------------------------*/
 #define LV_USE_ASSERT_OBJ 0
#define LV_USE_ASSERT_NULL 0
#define LV_USE_ASSERT_MALLOC 0
#define LV_USE_ASSERT_STYLE 0
#define LV_USE_ASSERT_MEM_INTEGRITY 0
#define LV_USE_OBSERVER 1

#define LV_ATTRIBUTE_MEM_ALIGN_DEFAULT_SIZE 128

#ifdef CONFIG_LV_DRAW_BUF_ALIGN
#  define LV_ATTRIBUTE_MEM_ALIGN  __attribute__((aligned(CONFIG_LV_DRAW_BUF_ALIGN)))
#else
/* Fallback: align to XXX bytes (L2 cache line / PPA requirement). */
#  define LV_ATTRIBUTE_MEM_ALIGN  __attribute__((aligned(LV_ATTRIBUTE_MEM_ALIGN_DEFAULT_SIZE)))
#endif

/* Activar el uso de funciones de memoria personalizadas */
#define LV_USE_STDLIB_MALLOC    LV_STDLIB_BUILTIN
#if LV_USE_STDLIB_MALLOC == LV_STDLIB_BUILTIN
    #define LV_MEM_SIZE (1024 * 1024U) 
    #define LV_MEM_POOL_EXPAND_SIZE 0
    #define LV_MEM_ADR 0
    #if LV_MEM_ADR == 0
        #undef LV_MEM_POOL_INCLUDE
        #undef LV_MEM_POOL_ALLOC
    #endif
#endif
#define LV_ATTRIBUTE_LARGE_RAM_ARRAY EXT_RAM_BSS_ATTR

#define LV_USE_STDLIB_STRING    LV_STDLIB_CLIB
#define LV_USE_STDLIB_SPRINTF  LV_STDLIB_CLIB
#define LV_USE_LOG 0
#if LV_USE_LOG
    #define LV_LOG_LEVEL LV_LOG_LEVEL_INFO
    #define LV_LOG_PRINTF 1
#endif

#define LV_SYSMON_GET_IDLE lv_timer_get_idle

/*---------------------------------------------------------------------------
 * File-system: POSIX backed by ESP-IDF VFS (FATFS mounts at /fatfs)
 *--------------------------------------------------------------------------*/
#define LV_USE_FS_POSIX  1
#if LV_USE_FS_POSIX
    #define LV_FS_POSIX_LETTER  'S'
    #define LV_FS_POSIX_PATH    "/fatfs/"
    #define LV_FS_POSIX_CACHE_SIZE 512
#endif

#endif /*LV_CONF_H*/

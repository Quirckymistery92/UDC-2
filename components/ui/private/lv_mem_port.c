#include "esp_heap_caps.h"
#include "lvgl.h"
#include "esp_log.h"
/* LVGL custom allocator – all allocations go to PSRAM. */

void lv_mem_init(void)   { /* PSRAM already initialised by ESP-IDF */ }
void lv_mem_deinit(void) { }

void * lv_malloc_core(size_t size)
{
    return heap_caps_malloc(size,
                                 MALLOC_CAP_SPIRAM |
                                 MALLOC_CAP_8BIT);
}

void * lv_realloc_core(void * ptr, size_t new_size) {
    return heap_caps_realloc(ptr, new_size, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
}

void lv_free_core(void * ptr)
{
    heap_caps_free(ptr);
}

void lv_mem_monitor_core(lv_mem_monitor_t * mon_p) {
    multi_heap_info_t info;
    heap_caps_get_info(&info, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    mon_p->total_size        = info.total_free_bytes + info.total_allocated_bytes;
    mon_p->free_size         = info.total_free_bytes;
    mon_p->free_biggest_size = info.largest_free_block;
    mon_p->used_cnt          = 0;
    mon_p->free_cnt          = 0;
    mon_p->used_pct          = mon_p->total_size
                               ? (100U * (mon_p->total_size - mon_p->free_size)) / mon_p->total_size
                               : 0;
    mon_p->frag_pct          = mon_p->free_size
                               ? (100U * (mon_p->free_size - mon_p->free_biggest_size)) / mon_p->free_size
                               : 0;
}

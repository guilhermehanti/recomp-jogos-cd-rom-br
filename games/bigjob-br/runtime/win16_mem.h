#ifndef BIGJOB_WIN16_MEM_H
#define BIGJOB_WIN16_MEM_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "cpu.h"

extern CpuState *g_active_cpu;

#ifdef __cplusplus
extern "C" {
#endif

/* Win16 protected mode huge-pointer shift: selector step = 1 << 3 = 8 */
#define WIN16_AHSHIFT 3
#define WIN16_AHINCR  (1u << WIN16_AHSHIFT)

/* Fixed selector mapping for the 49 NE segments:
 * NE segment id (1..49) -> selector (id << 3) | 7 (or simply id * 8).
 * Using sel = seg_id * 8 keeps segment selectors clean and aligned with __AHINCR=8.
 */
#define NE_SEG_TO_SEL(seg_id) ((uint16_t)((seg_id) * WIN16_AHINCR))
#define NE_SEL_TO_SEG(sel)    ((uint16_t)((sel) / WIN16_AHINCR))
#define NE_DGROUP_SEL         NE_SEG_TO_SEL(49)

typedef struct Win16SegDesc {
    uint8_t *base;
    uint32_t limit;       /* usable bytes from this selector */
    uint32_t alloc_size;  /* total allocation size (only on head selector) */
    uint16_t handle;      /* GlobalAlloc handle (0 for NE segments) */
    uint16_t lock_count;
    bool     in_use;
    bool     is_ne_seg;
    bool     is_head;
} Win16SegDesc;

/* Lifecycle */
bool win16_mem_init(void);
void win16_mem_shutdown(void);

/* Load all 49 NE segments and apply data-segment relocations from BIGJOBNG.EXE bytes */
bool win16_load_ne_image(const uint8_t *ne_bytes, size_t ne_size);

/* Global heap (Win16 KERNEL!15..21) */
uint16_t win16_global_alloc(uint16_t flags, uint32_t size);
uint16_t win16_global_realloc(uint16_t handle, uint32_t new_size, uint16_t flags);
uint16_t win16_global_free(uint16_t handle);
uint32_t win16_global_lock(uint16_t handle);   /* returns seg:off packed as (seg<<16)|0 */
uint16_t win16_global_unlock(uint16_t handle);
uint32_t win16_global_size(uint16_t handle);
uint32_t win16_global_handle(uint16_t sel);    /* returns (sel<<16)|handle */

/* Huge memory copy (KERNEL!348 hmemcpy) */
void win16_hmemcpy(uint16_t dst_seg, uint16_t dst_off,
                   uint16_t src_seg, uint16_t src_off,
                   uint32_t count);

/* Read C-string from segmented memory into host buffer */
size_t win16_read_cstr(uint16_t seg, uint16_t off, char *dst, size_t dst_cap);
void   win16_write_cstr(uint16_t seg, uint16_t off, const char *src, size_t max_len);

/* Access to NE resource table (parsed from BIGJOBNG.EXE) */
bool win16_ne_load_string(uint16_t str_id, char *dst, size_t dst_cap, size_t *out_len);
const uint8_t *win16_ne_find_resource(uint16_t rt_type, uint16_t res_id, uint32_t *out_size);

#ifdef __cplusplus
}
#endif

#endif /* BIGJOB_WIN16_MEM_H */

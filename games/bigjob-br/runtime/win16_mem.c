#include "win16_mem.h"
#include "cpu.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_SEG_SLOTS 8192  /* covers selectors 0x0000 .. 0xFFF8 with step 8 */
#define FIRST_HEAP_SLOT 64  /* slots 1..49 reserved for NE segments */

static Win16SegDesc g_segs[MAX_SEG_SLOTS];
CpuState *g_active_cpu = NULL;
static int g_inv_sel_logs = 0;
static uint8_t *g_ne_image = NULL;
static size_t   g_ne_size  = 0;
static uint32_t g_ne_hdr   = 0;
static uint16_t g_res_table_off = 0;

static uint16_t rd16(const uint8_t *p) {
    return (uint16_t)(p[0] | ((uint16_t)p[1] << 8));
}
static uint32_t rd32(const uint8_t *p) {
    return (uint32_t)p[0] | ((uint32_t)p[1] << 8) | ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
}
static void wr16(uint8_t *p, uint16_t v) {
    p[0] = (uint8_t)(v & 0xFF);
    p[1] = (uint8_t)((v >> 8) & 0xFF);
}
static void wr32(uint8_t *p, uint32_t v) {
    p[0] = (uint8_t)(v & 0xFF);
    p[1] = (uint8_t)((v >> 8) & 0xFF);
    p[2] = (uint8_t)((v >> 16) & 0xFF);
    p[3] = (uint8_t)((v >> 24) & 0xFF);
}

static inline uint32_t sel_to_slot(uint16_t sel) {
    return (uint32_t)(sel >> WIN16_AHSHIFT);
}

bool win16_mem_init(void) {
    memset(g_segs, 0, sizeof(g_segs));
    return true;
}

void win16_mem_shutdown(void) {
    for (uint32_t i = 0; i < MAX_SEG_SLOTS; ++i) {
        if (g_segs[i].in_use && g_segs[i].is_head && g_segs[i].base) {
            free(g_segs[i].base);
        }
    }
    memset(g_segs, 0, sizeof(g_segs));
    free(g_ne_image);
    g_ne_image = NULL;
    g_ne_size = 0;
}

uint8_t *win16_ptr(uint16_t seg, uint16_t off) {
    uint32_t slot = sel_to_slot(seg);
    if (slot == 0 || slot >= MAX_SEG_SLOTS || !g_segs[slot].in_use) {
        if (g_inv_sel_logs < 40) {
            g_inv_sel_logs++;
            uint16_t cs = g_active_cpu ? g_active_cpu->cs : 0;
            uint16_t ss = g_active_cpu ? g_active_cpu->ss : 0;
            uint16_t sp = g_active_cpu ? g_active_cpu->esp.w : 0;
            uint16_t bp = g_active_cpu ? g_active_cpu->ebp.w : 0;
            uint32_t ss_slot = sel_to_slot(ss);
            uint16_t r_ip = 0, r_cs = 0, bp_ip = 0, bp_cs = 0;
            if (ss_slot > 0 && ss_slot < MAX_SEG_SLOTS && g_segs[ss_slot].in_use) {
                if ((uint32_t)(sp + 4) <= g_segs[ss_slot].limit) {
                    r_ip = rd16(g_segs[ss_slot].base + sp);
                    r_cs = rd16(g_segs[ss_slot].base + sp + 2);
                }
                if ((uint32_t)(bp + 6) <= g_segs[ss_slot].limit) {
                    bp_ip = rd16(g_segs[ss_slot].base + bp + 2);
                    bp_cs = rd16(g_segs[ss_slot].base + bp + 4);
                }
            }
            fprintf(stderr, "[win16_mem] INVALID selector %04X:%04X (CS=seg%u SP_ret=seg%u:%04X BP_ret=seg%u:%04X)\n",
                    seg, off, NE_SEL_TO_SEG(cs), NE_SEL_TO_SEG(r_cs), r_ip, NE_SEL_TO_SEG(bp_cs), bp_ip);
        }
        return NULL;
    }
    if ((uint32_t)off >= g_segs[slot].limit) {
        fprintf(stderr, "[win16_mem] OOB access %04X:%04X (limit=%u)\n", seg, off, g_segs[slot].limit);
        return NULL;
    }
    return g_segs[slot].base + off;
}

uint8_t mem_r8(uint16_t seg, uint16_t off) {
    uint8_t *p = win16_ptr(seg, off);
    return p ? *p : 0;
}

uint16_t mem_r16(uint16_t seg, uint16_t off) {
    uint8_t *p = win16_ptr(seg, off);
    return p ? rd16(p) : 0;
}

uint32_t mem_r32(uint16_t seg, uint16_t off) {
    uint8_t *p = win16_ptr(seg, off);
    return p ? rd32(p) : 0;
}

void mem_w8(uint16_t seg, uint16_t off, uint8_t val) {
    uint8_t *p = win16_ptr(seg, off);
    if (p) *p = val;
}

void mem_w16(uint16_t seg, uint16_t off, uint16_t val) {
    uint8_t *p = win16_ptr(seg, off);
    if (p) wr16(p, val);
}

void mem_w32(uint16_t seg, uint16_t off, uint32_t val) {
    uint8_t *p = win16_ptr(seg, off);
    if (p) wr32(p, val);
}

/* Apply NE relocations to a loaded segment buffer (resolving internal segment selectors & offsets) */
static void apply_ne_relocs(uint16_t seg_id, uint8_t *seg_buf, uint32_t seg_cap,
                            const uint8_t *ne_bytes, size_t ne_size, uint32_t reloc_off) {
    if (reloc_off + 2 > ne_size) return;
    uint16_t count = rd16(ne_bytes + reloc_off);
    uint32_t pos = reloc_off + 2;

    for (uint16_t i = 0; i < count; ++i) {
        if (pos + 8 > ne_size) break;
        uint8_t src_type = ne_bytes[pos + 0];
        uint8_t flags    = ne_bytes[pos + 1];
        uint16_t site    = rd16(ne_bytes + pos + 2);
        uint16_t target1 = rd16(ne_bytes + pos + 4);
        uint16_t target2 = rd16(ne_bytes + pos + 6);
        pos += 8;

        uint8_t target_kind = flags & 0x03;
        bool additive = (flags & 0x04) != 0;

        /* Skip OS fixups (flags == 7 / target_kind == 3): preserve original x87 bytes */
        if (target_kind == 3) continue;

        /* Only patch internal references (target_kind == 0) and KERNEL!113 (__AHSHIFT=3) in memory.
         * External API calls in code segments are handled directly by the static lifter. */
        uint16_t patch_sel = 0;
        uint16_t patch_off = 0;
        bool do_patch = false;

        if (target_kind == 0) {
            if (target1 >= 1 && target1 <= 49) {
                patch_sel = NE_SEG_TO_SEL(target1);
                patch_off = target2;
                do_patch = true;
            }
        } else if (target_kind == 1) {
            /* Import by ordinal: patch KERNEL!113 (__AHSHIFT = 3) */
            /* Module index for KERNEL in BIGJOBNG.EXE is 2; verify via ordinal 113 & src_type 5 */
            if (src_type == 5 && target2 == 113) {
                patch_off = WIN16_AHSHIFT;
                do_patch = true;
            }
        }

        if (!do_patch) continue;

        /* Follow relocation chain unless additive */
        uint16_t cur = site;
        int guard = 0;
        while (cur != 0xFFFF && guard < 4096) {
            guard++;
            if (cur + 2 > seg_cap) break;
            uint16_t next = additive ? 0xFFFF : rd16(seg_buf + cur);

            if (src_type == 2) {
                /* 16-bit selector */
                wr16(seg_buf + cur, patch_sel);
            } else if (src_type == 3) {
                /* 32-bit far pointer (offset:16, selector:16) */
                if (cur + 4 <= seg_cap) {
                    wr16(seg_buf + cur, additive ? (uint16_t)(rd16(seg_buf + cur) + patch_off) : patch_off);
                    wr16(seg_buf + cur + 2, patch_sel);
                }
            } else if (src_type == 5) {
                /* 16-bit offset */
                wr16(seg_buf + cur, additive ? (uint16_t)(rd16(seg_buf + cur) + patch_off) : patch_off);
            }

            if (additive) break;
            cur = next;
        }
    }
    (void)seg_id;
}

bool win16_load_ne_image(const uint8_t *ne_bytes, size_t ne_size) {
    if (!ne_bytes || ne_size < 0x40 || ne_bytes[0] != 'M' || ne_bytes[1] != 'Z') {
        fprintf(stderr, "[win16_mem] Invalid MZ header\n");
        return false;
    }
    uint32_t ne_off = rd32(ne_bytes + 0x3C);
    if (ne_off + 0x40 > ne_size || ne_bytes[ne_off] != 'N' || ne_bytes[ne_off + 1] != 'E') {
        fprintf(stderr, "[win16_mem] Invalid NE header at 0x%X\n", ne_off);
        return false;
    }

    free(g_ne_image);
    g_ne_image = (uint8_t *)malloc(ne_size);
    if (!g_ne_image) return false;
    memcpy(g_ne_image, ne_bytes, ne_size);
    g_ne_size = ne_size;
    g_ne_hdr  = ne_off;

    const uint8_t *nh = g_ne_image + ne_off;
    uint16_t seg_count   = rd16(nh + 0x1C);
    uint16_t seg_tbl_off = rd16(nh + 0x22);
    g_res_table_off      = rd16(nh + 0x24);
    uint16_t align_shift = rd16(nh + 0x32);
    if (align_shift == 0) align_shift = 9;

    if (seg_count == 0 || seg_count >= FIRST_HEAP_SLOT) {
        fprintf(stderr, "[win16_mem] Unexpected NE segment count: %u\n", seg_count);
        return false;
    }

    for (uint16_t i = 0; i < seg_count; ++i) {
        uint16_t seg_id = (uint16_t)(i + 1);
        const uint8_t *se = nh + seg_tbl_off + i * 8;
        uint32_t file_off = (uint32_t)rd16(se + 0) << align_shift;
        uint32_t file_len = rd16(se + 2);
        uint16_t flags    = rd16(se + 4);
        uint32_t minalloc = rd16(se + 6);
        if (file_off != 0 && file_len == 0) file_len = 65536;
        if (minalloc == 0) minalloc = 65536;

        /* Give DGROUP (seg 49) a full 64KB so BSS + stack + local heap fit cleanly */
        uint32_t alloc_len = (seg_id == 49) ? 65536 : (minalloc > file_len ? minalloc : file_len);
        if (alloc_len < 65536 && (flags & 1)) {
            /* Data segments get at least minalloc, rounded up nicely */
            alloc_len = (seg_id == 49) ? 65536 : alloc_len;
        }

        uint8_t *buf = (uint8_t *)calloc(1, alloc_len);
        if (!buf) return false;
        if (file_off > 0 && file_off + file_len <= ne_size) {
            memcpy(buf, ne_bytes + file_off, file_len);
        }

        /* Apply NE relocations if HASRELOC (bit 8 = 0x0100) */
        if ((flags & 0x0100) && file_off > 0) {
            apply_ne_relocs(seg_id, buf, alloc_len, ne_bytes, ne_size, file_off + file_len);
        }

        g_segs[seg_id].base       = buf;
        g_segs[seg_id].limit      = alloc_len;
        g_segs[seg_id].alloc_size = alloc_len;
        g_segs[seg_id].handle     = 0;
        g_segs[seg_id].lock_count = 1;
        g_segs[seg_id].in_use     = true;
        g_segs[seg_id].is_ne_seg  = true;
        g_segs[seg_id].is_head    = true;
    }

    return true;
}

static uint32_t find_free_slots(uint32_t num_slots) {
    uint32_t run = 0;
    for (uint32_t i = FIRST_HEAP_SLOT; i < MAX_SEG_SLOTS; ++i) {
        if (!g_segs[i].in_use) {
            run++;
            if (run == num_slots) {
                return i - num_slots + 1;
            }
        } else {
            run = 0;
        }
    }
    return 0;
}

uint16_t win16_global_alloc(uint16_t flags, uint32_t size) {
    if (size == 0) size = 16;
    uint32_t num_slots = (size + 65535u) / 65536u;
    uint32_t start = find_free_slots(num_slots);
    if (start == 0) return 0;

    uint8_t *buf = (flags & 0x0040) ? (uint8_t *)calloc(1, size) : (uint8_t *)malloc(size);
    if (!buf) return 0;
    if (!(flags & 0x0040)) memset(buf, 0, size); /* deterministic zero-init */

    uint16_t handle = (uint16_t)(start * WIN16_AHINCR);
    for (uint32_t k = 0; k < num_slots; ++k) {
        uint32_t off = k * 65536u;
        g_segs[start + k].base       = buf + off;
        g_segs[start + k].limit      = size - off;
        g_segs[start + k].alloc_size = (k == 0) ? size : 0;
        g_segs[start + k].handle     = handle;
        g_segs[start + k].lock_count = 0;
        g_segs[start + k].in_use     = true;
        g_segs[start + k].is_ne_seg  = false;
        g_segs[start + k].is_head    = (k == 0);
    }
    return handle;
}

uint16_t win16_global_realloc(uint16_t handle, uint32_t new_size, uint16_t flags) {
    uint32_t old_slot = sel_to_slot(handle);
    if (old_slot < FIRST_HEAP_SLOT || old_slot >= MAX_SEG_SLOTS || !g_segs[old_slot].in_use || !g_segs[old_slot].is_head) {
        return 0;
    }
    uint32_t old_size = g_segs[old_slot].alloc_size;
    uint8_t *old_base = g_segs[old_slot].base;

    uint16_t new_h = win16_global_alloc(flags, new_size);
    if (!new_h) return 0;
    uint32_t new_slot = sel_to_slot(new_h);
    uint32_t copy_n = old_size < new_size ? old_size : new_size;
    memcpy(g_segs[new_slot].base, old_base, copy_n);
    win16_global_free(handle);
    return new_h;
}

uint16_t win16_global_free(uint16_t handle) {
    uint32_t slot = sel_to_slot(handle);
    if (slot < FIRST_HEAP_SLOT || slot >= MAX_SEG_SLOTS || !g_segs[slot].in_use || !g_segs[slot].is_head) {
        return handle; /* error: non-zero */
    }
    uint32_t num_slots = (g_segs[slot].alloc_size + 65535u) / 65536u;
    free(g_segs[slot].base);
    for (uint32_t k = 0; k < num_slots && (slot + k) < MAX_SEG_SLOTS; ++k) {
        memset(&g_segs[slot + k], 0, sizeof(Win16SegDesc));
    }
    return 0; /* Win16 GlobalFree returns NULL (0) on success */
}

uint32_t win16_global_lock(uint16_t handle) {
    uint32_t slot = sel_to_slot(handle);
    if (slot == 0 || slot >= MAX_SEG_SLOTS || !g_segs[slot].in_use) return 0;
    g_segs[slot].lock_count++;
    return ((uint32_t)handle << 16); /* sel:0000 */
}

uint16_t win16_global_unlock(uint16_t handle) {
    uint32_t slot = sel_to_slot(handle);
    if (slot == 0 || slot >= MAX_SEG_SLOTS || !g_segs[slot].in_use) return 0;
    if (g_segs[slot].lock_count > 0) g_segs[slot].lock_count--;
    return g_segs[slot].lock_count;
}

uint32_t win16_global_size(uint16_t handle) {
    uint32_t slot = sel_to_slot(handle);
    if (slot == 0 || slot >= MAX_SEG_SLOTS || !g_segs[slot].in_use) return 0;
    return g_segs[slot].alloc_size;
}

uint32_t win16_global_handle(uint16_t sel) {
    uint32_t slot = sel_to_slot(sel);
    if (slot == 0 || slot >= MAX_SEG_SLOTS || !g_segs[slot].in_use) return 0;
    uint16_t h = g_segs[slot].handle ? g_segs[slot].handle : sel;
    return ((uint32_t)sel << 16) | h;
}

void win16_hmemcpy(uint16_t dst_seg, uint16_t dst_off,
                   uint16_t src_seg, uint16_t src_off,
                   uint32_t count) {
    if (count == 0) return;
    uint8_t *dst = win16_ptr(dst_seg, dst_off);
    uint8_t *src = win16_ptr(src_seg, src_off);
    if (!dst || !src) return;
    memmove(dst, src, count);
}

size_t win16_read_cstr(uint16_t seg, uint16_t off, char *dst, size_t dst_cap) {
    if (!dst || dst_cap == 0) return 0;
    size_t i = 0;
    while (i + 1 < dst_cap) {
        uint8_t c = mem_r8(seg, (uint16_t)(off + i));
        dst[i] = (char)c;
        if (c == 0) return i;
        i++;
    }
    dst[dst_cap - 1] = '\0';
    return dst_cap - 1;
}

void win16_write_cstr(uint16_t seg, uint16_t off, const char *src, size_t max_len) {
    if (!src || max_len == 0) return;
    size_t i = 0;
    while (i + 1 < max_len && src[i] != '\0') {
        mem_w8(seg, (uint16_t)(off + i), (uint8_t)src[i]);
        i++;
    }
    mem_w8(seg, (uint16_t)(off + i), 0);
}

const uint8_t *win16_ne_find_resource(uint16_t rt_type, uint16_t res_id, uint32_t *out_size) {
    if (!g_ne_image || g_res_table_off == 0) return NULL;
    uint32_t pos = g_ne_hdr + g_res_table_off;
    if (pos + 2 > g_ne_size) return NULL;
    uint16_t shift = rd16(g_ne_image + pos);
    pos += 2;

    while (pos + 2 <= g_ne_size) {
        uint16_t type_id = rd16(g_ne_image + pos);
        if (type_id == 0) break;
        uint16_t count = rd16(g_ne_image + pos + 2);
        pos += 8;
        bool type_match = (type_id & 0x8000) && ((type_id & 0x7FFF) == rt_type);
        for (uint16_t i = 0; i < count; ++i) {
            if (pos + 12 > g_ne_size) return NULL;
            uint16_t r_off = rd16(g_ne_image + pos + 0);
            uint16_t r_len = rd16(g_ne_image + pos + 2);
            uint16_t r_id  = rd16(g_ne_image + pos + 6);
            pos += 12;
            if (type_match && (r_id & 0x8000) && ((r_id & 0x7FFF) == res_id)) {
                uint32_t abs_off = (uint32_t)r_off << shift;
                uint32_t abs_len = (uint32_t)r_len << shift;
                if (abs_off + abs_len <= g_ne_size) {
                    if (out_size) *out_size = abs_len;
                    return g_ne_image + abs_off;
                }
            }
        }
    }
    return NULL;
}

bool win16_ne_load_string(uint16_t str_id, char *dst, size_t dst_cap, size_t *out_len) {
    uint16_t block_id = (uint16_t)((str_id >> 4) + 1);
    uint16_t index    = (uint16_t)(str_id & 0x0F);
    uint32_t sz = 0;
    const uint8_t *blk = win16_ne_find_resource(6 /* RT_STRING */, block_id, &sz);
    if (!blk) return false;
    uint32_t pos = 0;
    for (uint16_t i = 0; i <= index; ++i) {
        if (pos >= sz) return false;
        uint8_t slen = blk[pos++];
        if (pos + slen > sz) return false;
        if (i == index) {
            size_t n = slen;
            if (n + 1 > dst_cap) n = dst_cap ? dst_cap - 1 : 0;
            if (dst && dst_cap > 0) {
                memcpy(dst, blk + pos, n);
                dst[n] = '\0';
            }
            if (out_len) *out_len = n;
            return slen > 0;
        }
        pos += slen;
    }
    return false;
}

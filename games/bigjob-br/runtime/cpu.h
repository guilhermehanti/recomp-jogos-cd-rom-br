#ifndef BIGJOB_CPU_H
#define BIGJOB_CPU_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct CpuState CpuState;
typedef void (*RecompFn)(CpuState *cpu);

/* 32-bit x86 register with 16-bit and 8-bit aliases (little-endian host) */
typedef union Reg32 {
    uint32_t d;
    uint16_t w;
    struct {
        uint8_t l;
        uint8_t h;
    } b;
} Reg32;

struct CpuState {
    Reg32 eax, ebx, ecx, edx;
    Reg32 esi, edi, ebp, esp;

    uint16_t es, cs, ss, ds, fs, gs;
    uint16_t ip;

    /* Flags (stored as 0 or 1 for speed and clarity) */
    uint8_t cf, pf, af, zf, sf, of, df;

    /* Minimal x87 stack for Borland CRT (seg01) */
    double fpu_st[8];
    int fpu_top;
    uint16_t fpu_cw;
    uint16_t fpu_sw;

    /* Execution control */
    bool halted;
    int exit_code;
};

/* Register accessors */
#define EAX (cpu->eax.d)
#define EBX (cpu->ebx.d)
#define ECX (cpu->ecx.d)
#define EDX (cpu->edx.d)
#define ESI (cpu->esi.d)
#define EDI (cpu->edi.d)
#define EBP (cpu->ebp.d)
#define ESP (cpu->esp.d)

#define AX  (cpu->eax.w)
#define BX  (cpu->ebx.w)
#define CX  (cpu->ecx.w)
#define DX  (cpu->edx.w)
#define SI  (cpu->esi.w)
#define DI  (cpu->edi.w)
#define BP  (cpu->ebp.w)
#define SP  (cpu->esp.w)

#define AL  (cpu->eax.b.l)
#define AH  (cpu->eax.b.h)
#define BL  (cpu->ebx.b.l)
#define BH  (cpu->ebx.b.h)
#define CL  (cpu->ecx.b.l)
#define CH  (cpu->ecx.b.h)
#define DL  (cpu->edx.b.l)
#define DH  (cpu->edx.b.h)

/* Segmented memory access (implemented in win16_mem.c) */
uint8_t *win16_ptr(uint16_t seg, uint16_t off);
uint8_t  mem_r8(uint16_t seg, uint16_t off);
uint16_t mem_r16(uint16_t seg, uint16_t off);
uint32_t mem_r32(uint16_t seg, uint16_t off);
void     mem_w8(uint16_t seg, uint16_t off, uint8_t val);
void     mem_w16(uint16_t seg, uint16_t off, uint16_t val);
void     mem_w32(uint16_t seg, uint16_t off, uint32_t val);

/* Stack helpers */
static inline void push16(CpuState *cpu, uint16_t val) {
    cpu->esp.w = (uint16_t)(cpu->esp.w - 2);
    mem_w16(cpu->ss, cpu->esp.w, val);
}

static inline uint16_t pop16(CpuState *cpu) {
    uint16_t val = mem_r16(cpu->ss, cpu->esp.w);
    cpu->esp.w = (uint16_t)(cpu->esp.w + 2);
    return val;
}

static inline void push32(CpuState *cpu, uint32_t val) {
    cpu->esp.w = (uint16_t)(cpu->esp.w - 4);
    mem_w32(cpu->ss, cpu->esp.w, val);
}

static inline uint32_t pop32(CpuState *cpu) {
    uint32_t val = mem_r32(cpu->ss, cpu->esp.w);
    cpu->esp.w = (uint16_t)(cpu->esp.w + 4);
    return val;
}

/* Flag helpers */
static inline uint8_t calc_parity8(uint8_t v) {
    v ^= v >> 4;
    v ^= v >> 2;
    v ^= v >> 1;
    return (uint8_t)((~v) & 1);
}

static inline uint16_t get_flags16(const CpuState *cpu) {
    uint16_t f = 0x0002;
    if (cpu->cf) f |= 0x0001;
    if (cpu->pf) f |= 0x0004;
    if (cpu->af) f |= 0x0010;
    if (cpu->zf) f |= 0x0040;
    if (cpu->sf) f |= 0x0080;
    if (cpu->df) f |= 0x0400;
    if (cpu->of) f |= 0x0800;
    return f;
}

static inline void set_flags16(CpuState *cpu, uint16_t f) {
    cpu->cf = (f & 0x0001) != 0;
    cpu->pf = (f & 0x0004) != 0;
    cpu->af = (f & 0x0010) != 0;
    cpu->zf = (f & 0x0040) != 0;
    cpu->sf = (f & 0x0080) != 0;
    cpu->df = (f & 0x0400) != 0;
    cpu->of = (f & 0x0800) != 0;
}

static inline void set_szp8(CpuState *cpu, uint8_t r) {
    cpu->zf = (r == 0);
    cpu->sf = (r & 0x80) != 0;
    cpu->pf = calc_parity8(r);
}

static inline void set_szp16(CpuState *cpu, uint16_t r) {
    cpu->zf = (r == 0);
    cpu->sf = (r & 0x8000) != 0;
    cpu->pf = calc_parity8((uint8_t)r);
}

static inline void set_szp32(CpuState *cpu, uint32_t r) {
    cpu->zf = (r == 0);
    cpu->sf = (r & 0x80000000u) != 0;
    cpu->pf = calc_parity8((uint8_t)r);
}

/* Indirect dispatch (implemented in dispatch_table.c / runtime) */
void recomp_dispatch_far(CpuState *cpu, uint16_t seg, uint16_t off);
void recomp_dispatch_near(CpuState *cpu, uint16_t seg, uint16_t off);

/* Optional execution trace (BIGJOB_TRACE=1) */
extern int g_trace;

/* Win16 API and DOS INT bridges */
void win16_call_import(CpuState *cpu, const char *mod, uint16_t ordinal);
void win16_int(CpuState *cpu, uint8_t int_num);

#ifdef __cplusplus
}
#endif

#endif /* BIGJOB_CPU_H */

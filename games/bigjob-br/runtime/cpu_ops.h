#ifndef BIGJOB_CPU_OPS_H
#define BIGJOB_CPU_OPS_H

#include "cpu.h"

/* ---- Condition codes ---- */
#define CC_O(c)  ((c)->of != 0)
#define CC_NO(c) ((c)->of == 0)
#define CC_B(c)  ((c)->cf != 0)
#define CC_AE(c) ((c)->cf == 0)
#define CC_E(c)  ((c)->zf != 0)
#define CC_NE(c) ((c)->zf == 0)
#define CC_BE(c) ((c)->cf != 0 || (c)->zf != 0)
#define CC_A(c)  ((c)->cf == 0 && (c)->zf == 0)
#define CC_S(c)  ((c)->sf != 0)
#define CC_NS(c) ((c)->sf == 0)
#define CC_P(c)  ((c)->pf != 0)
#define CC_NP(c) ((c)->pf == 0)
#define CC_L(c)  ((c)->sf != (c)->of)
#define CC_GE(c) ((c)->sf == (c)->of)
#define CC_LE(c) ((c)->zf != 0 || (c)->sf != (c)->of)
#define CC_G(c)  ((c)->zf == 0 && (c)->sf == (c)->of)

/* ---- ADD / ADC ---- */
static inline uint8_t op_add8(CpuState *cpu, uint8_t a, uint8_t b) {
    uint16_t r = (uint16_t)a + (uint16_t)b;
    uint8_t r8 = (uint8_t)r;
    cpu->cf = (r > 0xFFu);
    cpu->of = (((a ^ r8) & (b ^ r8) & 0x80u) != 0);
    cpu->af = (((a ^ b ^ r8) & 0x10u) != 0);
    set_szp8(cpu, r8);
    return r8;
}
static inline uint16_t op_add16(CpuState *cpu, uint16_t a, uint16_t b) {
    uint32_t r = (uint32_t)a + (uint32_t)b;
    uint16_t r16 = (uint16_t)r;
    cpu->cf = (r > 0xFFFFu);
    cpu->of = (((a ^ r16) & (b ^ r16) & 0x8000u) != 0);
    cpu->af = (((a ^ b ^ r16) & 0x10u) != 0);
    set_szp16(cpu, r16);
    return r16;
}
static inline uint32_t op_add32(CpuState *cpu, uint32_t a, uint32_t b) {
    uint64_t r = (uint64_t)a + (uint64_t)b;
    uint32_t r32 = (uint32_t)r;
    cpu->cf = (r > 0xFFFFFFFFull);
    cpu->of = (((a ^ r32) & (b ^ r32) & 0x80000000u) != 0);
    cpu->af = (((a ^ b ^ r32) & 0x10u) != 0);
    set_szp32(cpu, r32);
    return r32;
}

static inline uint8_t op_adc8(CpuState *cpu, uint8_t a, uint8_t b) {
    uint16_t c = cpu->cf;
    uint16_t r = (uint16_t)a + (uint16_t)b + c;
    uint8_t r8 = (uint8_t)r;
    cpu->cf = (r > 0xFFu);
    cpu->of = (((a ^ r8) & (b ^ r8) & 0x80u) != 0);
    cpu->af = (((a ^ b ^ r8) & 0x10u) != 0);
    set_szp8(cpu, r8);
    return r8;
}
static inline uint16_t op_adc16(CpuState *cpu, uint16_t a, uint16_t b) {
    uint32_t c = cpu->cf;
    uint32_t r = (uint32_t)a + (uint32_t)b + c;
    uint16_t r16 = (uint16_t)r;
    cpu->cf = (r > 0xFFFFu);
    cpu->of = (((a ^ r16) & (b ^ r16) & 0x8000u) != 0);
    cpu->af = (((a ^ b ^ r16) & 0x10u) != 0);
    set_szp16(cpu, r16);
    return r16;
}
static inline uint32_t op_adc32(CpuState *cpu, uint32_t a, uint32_t b) {
    uint64_t c = cpu->cf;
    uint64_t r = (uint64_t)a + (uint64_t)b + c;
    uint32_t r32 = (uint32_t)r;
    cpu->cf = (r > 0xFFFFFFFFull);
    cpu->of = (((a ^ r32) & (b ^ r32) & 0x80000000u) != 0);
    cpu->af = (((a ^ b ^ r32) & 0x10u) != 0);
    set_szp32(cpu, r32);
    return r32;
}

/* ---- SUB / SBB / CMP ---- */
static inline uint8_t op_sub8(CpuState *cpu, uint8_t a, uint8_t b) {
    uint16_t r = (uint16_t)a - (uint16_t)b;
    uint8_t r8 = (uint8_t)r;
    cpu->cf = (a < b);
    cpu->of = (((a ^ b) & (a ^ r8) & 0x80u) != 0);
    cpu->af = (((a ^ b ^ r8) & 0x10u) != 0);
    set_szp8(cpu, r8);
    return r8;
}
static inline uint16_t op_sub16(CpuState *cpu, uint16_t a, uint16_t b) {
    uint32_t r = (uint32_t)a - (uint32_t)b;
    uint16_t r16 = (uint16_t)r;
    cpu->cf = (a < b);
    cpu->of = (((a ^ b) & (a ^ r16) & 0x8000u) != 0);
    cpu->af = (((a ^ b ^ r16) & 0x10u) != 0);
    set_szp16(cpu, r16);
    return r16;
}
static inline uint32_t op_sub32(CpuState *cpu, uint32_t a, uint32_t b) {
    uint64_t r = (uint64_t)a - (uint64_t)b;
    uint32_t r32 = (uint32_t)r;
    cpu->cf = (a < b);
    cpu->of = (((a ^ b) & (a ^ r32) & 0x80000000u) != 0);
    cpu->af = (((a ^ b ^ r32) & 0x10u) != 0);
    set_szp32(cpu, r32);
    return r32;
}

static inline uint8_t op_sbb8(CpuState *cpu, uint8_t a, uint8_t b) {
    uint16_t c = cpu->cf;
    uint16_t r = (uint16_t)a - (uint16_t)b - c;
    uint8_t r8 = (uint8_t)r;
    cpu->cf = ((uint16_t)a < (uint16_t)b + c);
    cpu->of = (((a ^ b) & (a ^ r8) & 0x80u) != 0);
    cpu->af = (((a ^ b ^ r8) & 0x10u) != 0);
    set_szp8(cpu, r8);
    return r8;
}
static inline uint16_t op_sbb16(CpuState *cpu, uint16_t a, uint16_t b) {
    uint32_t c = cpu->cf;
    uint32_t r = (uint32_t)a - (uint32_t)b - c;
    uint16_t r16 = (uint16_t)r;
    cpu->cf = ((uint32_t)a < (uint32_t)b + c);
    cpu->of = (((a ^ b) & (a ^ r16) & 0x8000u) != 0);
    cpu->af = (((a ^ b ^ r16) & 0x10u) != 0);
    set_szp16(cpu, r16);
    return r16;
}
static inline uint32_t op_sbb32(CpuState *cpu, uint32_t a, uint32_t b) {
    uint64_t c = cpu->cf;
    uint64_t r = (uint64_t)a - (uint64_t)b - c;
    uint32_t r32 = (uint32_t)r;
    cpu->cf = ((uint64_t)a < (uint64_t)b + c);
    cpu->of = (((a ^ b) & (a ^ r32) & 0x80000000u) != 0);
    cpu->af = (((a ^ b ^ r32) & 0x10u) != 0);
    set_szp32(cpu, r32);
    return r32;
}

/* ---- INC / DEC (preserve CF!) ---- */
static inline uint8_t op_inc8(CpuState *cpu, uint8_t a) {
    uint8_t r = (uint8_t)(a + 1u);
    cpu->of = (a == 0x7Fu);
    cpu->af = ((a & 0x0Fu) == 0x0Fu);
    set_szp8(cpu, r);
    return r;
}
static inline uint16_t op_inc16(CpuState *cpu, uint16_t a) {
    uint16_t r = (uint16_t)(a + 1u);
    cpu->of = (a == 0x7FFFu);
    cpu->af = ((a & 0x0Fu) == 0x0Fu);
    set_szp16(cpu, r);
    return r;
}
static inline uint32_t op_inc32(CpuState *cpu, uint32_t a) {
    uint32_t r = a + 1u;
    cpu->of = (a == 0x7FFFFFFFu);
    cpu->af = ((a & 0x0Fu) == 0x0Fu);
    set_szp32(cpu, r);
    return r;
}

static inline uint8_t op_dec8(CpuState *cpu, uint8_t a) {
    uint8_t r = (uint8_t)(a - 1u);
    cpu->of = (a == 0x80u);
    cpu->af = ((a & 0x0Fu) == 0);
    set_szp8(cpu, r);
    return r;
}
static inline uint16_t op_dec16(CpuState *cpu, uint16_t a) {
    uint16_t r = (uint16_t)(a - 1u);
    cpu->of = (a == 0x8000u);
    cpu->af = ((a & 0x0Fu) == 0);
    set_szp16(cpu, r);
    return r;
}
static inline uint32_t op_dec32(CpuState *cpu, uint32_t a) {
    uint32_t r = a - 1u;
    cpu->of = (a == 0x80000000u);
    cpu->af = ((a & 0x0Fu) == 0);
    set_szp32(cpu, r);
    return r;
}

/* ---- NEG ---- */
static inline uint8_t  op_neg8(CpuState *cpu, uint8_t a)   { return op_sub8(cpu, 0, a); }
static inline uint16_t op_neg16(CpuState *cpu, uint16_t a) { return op_sub16(cpu, 0, a); }
static inline uint32_t op_neg32(CpuState *cpu, uint32_t a) { return op_sub32(cpu, 0, a); }

/* ---- Logic ---- */
static inline uint8_t op_and8(CpuState *cpu, uint8_t a, uint8_t b)   { uint8_t r = a & b; cpu->cf = 0; cpu->of = 0; set_szp8(cpu, r); return r; }
static inline uint16_t op_and16(CpuState *cpu, uint16_t a, uint16_t b) { uint16_t r = a & b; cpu->cf = 0; cpu->of = 0; set_szp16(cpu, r); return r; }
static inline uint32_t op_and32(CpuState *cpu, uint32_t a, uint32_t b) { uint32_t r = a & b; cpu->cf = 0; cpu->of = 0; set_szp32(cpu, r); return r; }

static inline uint8_t op_or8(CpuState *cpu, uint8_t a, uint8_t b)    { uint8_t r = a | b; cpu->cf = 0; cpu->of = 0; set_szp8(cpu, r); return r; }
static inline uint16_t op_or16(CpuState *cpu, uint16_t a, uint16_t b) { uint16_t r = a | b; cpu->cf = 0; cpu->of = 0; set_szp16(cpu, r); return r; }
static inline uint32_t op_or32(CpuState *cpu, uint32_t a, uint32_t b) { uint32_t r = a | b; cpu->cf = 0; cpu->of = 0; set_szp32(cpu, r); return r; }

static inline uint8_t op_xor8(CpuState *cpu, uint8_t a, uint8_t b)   { uint8_t r = a ^ b; cpu->cf = 0; cpu->of = 0; set_szp8(cpu, r); return r; }
static inline uint16_t op_xor16(CpuState *cpu, uint16_t a, uint16_t b) { uint16_t r = a ^ b; cpu->cf = 0; cpu->of = 0; set_szp16(cpu, r); return r; }
static inline uint32_t op_xor32(CpuState *cpu, uint32_t a, uint32_t b) { uint32_t r = a ^ b; cpu->cf = 0; cpu->of = 0; set_szp32(cpu, r); return r; }

/* ---- Shifts ---- */
static inline uint8_t op_shl8(CpuState *cpu, uint8_t v, uint8_t cnt) {
    cnt &= 31; if (!cnt) return v;
    cpu->cf = (cnt <= 8) ? ((v >> (8 - cnt)) & 1u) : 0;
    uint8_t r = (cnt < 8) ? (uint8_t)(v << cnt) : 0;
    if (cnt == 1) cpu->of = ((r >> 7) ^ cpu->cf) != 0;
    set_szp8(cpu, r); return r;
}
static inline uint16_t op_shl16(CpuState *cpu, uint16_t v, uint8_t cnt) {
    cnt &= 31; if (!cnt) return v;
    cpu->cf = (cnt <= 16) ? ((v >> (16 - cnt)) & 1u) : 0;
    uint16_t r = (cnt < 16) ? (uint16_t)(v << cnt) : 0;
    if (cnt == 1) cpu->of = ((r >> 15) ^ cpu->cf) != 0;
    set_szp16(cpu, r); return r;
}
static inline uint32_t op_shl32(CpuState *cpu, uint32_t v, uint8_t cnt) {
    cnt &= 31; if (!cnt) return v;
    cpu->cf = (v >> (32 - cnt)) & 1u;
    uint32_t r = v << cnt;
    if (cnt == 1) cpu->of = ((r >> 31) ^ cpu->cf) != 0;
    set_szp32(cpu, r); return r;
}

static inline uint8_t op_shr8(CpuState *cpu, uint8_t v, uint8_t cnt) {
    cnt &= 31; if (!cnt) return v;
    cpu->cf = (cnt <= 8) ? ((v >> (cnt - 1)) & 1u) : 0;
    uint8_t r = (cnt < 8) ? (uint8_t)(v >> cnt) : 0;
    if (cnt == 1) cpu->of = (v & 0x80u) != 0;
    set_szp8(cpu, r); return r;
}
static inline uint16_t op_shr16(CpuState *cpu, uint16_t v, uint8_t cnt) {
    cnt &= 31; if (!cnt) return v;
    cpu->cf = (cnt <= 16) ? ((v >> (cnt - 1)) & 1u) : 0;
    uint16_t r = (cnt < 16) ? (uint16_t)(v >> cnt) : 0;
    if (cnt == 1) cpu->of = (v & 0x8000u) != 0;
    set_szp16(cpu, r); return r;
}
static inline uint32_t op_shr32(CpuState *cpu, uint32_t v, uint8_t cnt) {
    cnt &= 31; if (!cnt) return v;
    cpu->cf = (v >> (cnt - 1)) & 1u;
    uint32_t r = v >> cnt;
    if (cnt == 1) cpu->of = (v & 0x80000000u) != 0;
    set_szp32(cpu, r); return r;
}

static inline uint8_t op_sar8(CpuState *cpu, uint8_t v, uint8_t cnt) {
    cnt &= 31; if (!cnt) return v;
    int8_t sv = (int8_t)v;
    cpu->cf = (cnt < 8) ? (uint8_t)((sv >> (cnt - 1)) & 1) : (sv < 0);
    uint8_t r = (cnt < 8) ? (uint8_t)(sv >> cnt) : (sv < 0 ? 0xFFu : 0);
    if (cnt == 1) cpu->of = 0;
    set_szp8(cpu, r); return r;
}
static inline uint16_t op_sar16(CpuState *cpu, uint16_t v, uint8_t cnt) {
    cnt &= 31; if (!cnt) return v;
    int16_t sv = (int16_t)v;
    cpu->cf = (cnt < 16) ? (uint8_t)((sv >> (cnt - 1)) & 1) : (sv < 0);
    uint16_t r = (cnt < 16) ? (uint16_t)(sv >> cnt) : (sv < 0 ? 0xFFFFu : 0);
    if (cnt == 1) cpu->of = 0;
    set_szp16(cpu, r); return r;
}
static inline uint32_t op_sar32(CpuState *cpu, uint32_t v, uint8_t cnt) {
    cnt &= 31; if (!cnt) return v;
    int32_t sv = (int32_t)v;
    cpu->cf = (uint8_t)((sv >> (cnt - 1)) & 1);
    uint32_t r = (uint32_t)(sv >> cnt);
    if (cnt == 1) cpu->of = 0;
    set_szp32(cpu, r); return r;
}

static inline uint32_t op_shld32(CpuState *cpu, uint32_t dst, uint32_t src, uint8_t cnt) {
    cnt &= 31; if (!cnt) return dst;
    cpu->cf = (dst >> (32 - cnt)) & 1u;
    uint32_t r = (dst << cnt) | (src >> (32 - cnt));
    set_szp32(cpu, r); return r;
}
static inline uint32_t op_shrd32(CpuState *cpu, uint32_t dst, uint32_t src, uint8_t cnt) {
    cnt &= 31; if (!cnt) return dst;
    cpu->cf = (dst >> (cnt - 1)) & 1u;
    uint32_t r = (dst >> cnt) | (src << (32 - cnt));
    set_szp32(cpu, r); return r;
}

/* ---- Rotates ---- */
static inline uint16_t op_rol16(CpuState *cpu, uint16_t v, uint8_t cnt) {
    cnt &= 15; if (!cnt) return v;
    uint16_t r = (uint16_t)((v << cnt) | (v >> (16 - cnt)));
    cpu->cf = r & 1u;
    if (cnt == 1) cpu->of = ((r >> 15) ^ cpu->cf) != 0;
    return r;
}
static inline uint16_t op_ror16(CpuState *cpu, uint16_t v, uint8_t cnt) {
    cnt &= 15; if (!cnt) return v;
    uint16_t r = (uint16_t)((v >> cnt) | (v << (16 - cnt)));
    cpu->cf = (r >> 15) & 1u;
    if (cnt == 1) cpu->of = (((r >> 15) ^ (r >> 14)) & 1u) != 0;
    return r;
}
static inline uint16_t op_rcl16(CpuState *cpu, uint16_t v, uint8_t cnt) {
    cnt %= 17;
    uint32_t tmp = v;
    for (uint8_t i = 0; i < cnt; ++i) {
        uint8_t new_cf = (tmp >> 15) & 1u;
        tmp = ((tmp << 1) | cpu->cf) & 0xFFFFu;
        cpu->cf = new_cf;
    }
    return (uint16_t)tmp;
}
static inline uint16_t op_rcr16(CpuState *cpu, uint16_t v, uint8_t cnt) {
    cnt %= 17;
    uint32_t tmp = v;
    for (uint8_t i = 0; i < cnt; ++i) {
        uint8_t new_cf = tmp & 1u;
        tmp = (tmp >> 1) | ((uint32_t)cpu->cf << 15);
        cpu->cf = new_cf;
    }
    return (uint16_t)tmp;
}

/* ---- Multiply / Divide ---- */
static inline void op_mul16(CpuState *cpu, uint16_t src) {
    uint32_t r = (uint32_t)AX * (uint32_t)src;
    AX = (uint16_t)(r & 0xFFFFu);
    DX = (uint16_t)(r >> 16);
    cpu->cf = cpu->of = (DX != 0);
}
static inline void op_imul16_1(CpuState *cpu, int16_t src) {
    int32_t r = (int32_t)(int16_t)AX * (int32_t)src;
    AX = (uint16_t)(r & 0xFFFFu);
    DX = (uint16_t)((uint32_t)r >> 16);
    cpu->cf = cpu->of = (r != (int32_t)(int16_t)AX);
}
static inline uint16_t op_imul16_2(CpuState *cpu, int16_t a, int16_t b) {
    int32_t r = (int32_t)a * (int32_t)b;
    uint16_t r16 = (uint16_t)r;
    cpu->cf = cpu->of = (r != (int32_t)(int16_t)r16);
    return r16;
}
static inline uint32_t op_imul32_2(CpuState *cpu, int32_t a, int32_t b) {
    int64_t r = (int64_t)a * (int64_t)b;
    uint32_t r32 = (uint32_t)r;
    cpu->cf = cpu->of = (r != (int64_t)(int32_t)r32);
    return r32;
}
static inline void op_div16(CpuState *cpu, uint16_t src) {
    if (src == 0) return;
    uint32_t num = ((uint32_t)DX << 16) | AX;
    AX = (uint16_t)(num / src);
    DX = (uint16_t)(num % src);
}
static inline void op_idiv16(CpuState *cpu, int16_t src) {
    if (src == 0) return;
    int32_t num = (int32_t)(((uint32_t)DX << 16) | AX);
    AX = (uint16_t)(int16_t)(num / src);
    DX = (uint16_t)(int16_t)(num % src);
}
static inline void op_div32(CpuState *cpu, uint32_t src) {
    if (src == 0) return;
    uint64_t num = ((uint64_t)EDX << 32) | EAX;
    EAX = (uint32_t)(num / src);
    EDX = (uint32_t)(num % src);
}
static inline void op_idiv32(CpuState *cpu, int32_t src) {
    if (src == 0) return;
    int64_t num = (int64_t)(((uint64_t)EDX << 32) | EAX);
    EAX = (uint32_t)(int32_t)(num / src);
    EDX = (uint32_t)(int32_t)(num % src);
}

/* ---- String operations ---- */
static inline void op_stosb(CpuState *cpu, bool rep) {
    int16_t step = cpu->df ? -1 : 1;
    uint16_t cnt = rep ? CX : 1;
    while (cnt--) {
        mem_w8(cpu->es, DI, AL);
        DI = (uint16_t)(DI + step);
    }
    if (rep) CX = 0;
}
static inline void op_stosw(CpuState *cpu, bool rep) {
    int16_t step = cpu->df ? -2 : 2;
    uint16_t cnt = rep ? CX : 1;
    while (cnt--) {
        mem_w16(cpu->es, DI, AX);
        DI = (uint16_t)(DI + step);
    }
    if (rep) CX = 0;
}
static inline void op_stosl(CpuState *cpu, bool rep) {
    int16_t step = cpu->df ? -4 : 4;
    uint16_t cnt = rep ? CX : 1;
    while (cnt--) {
        mem_w32(cpu->es, DI, EAX);
        DI = (uint16_t)(DI + step);
    }
    if (rep) CX = 0;
}
static inline void op_stosb32(CpuState *cpu, bool rep) {
    int32_t step = cpu->df ? -1 : 1;
    uint32_t cnt = rep ? ECX : 1;
    while (cnt--) {
        uint16_t ds = (uint16_t)(cpu->es + ((EDI >> 16) << 3));
        mem_w8(ds, (uint16_t)(EDI & 0xFFFFu), AL);
        EDI = (uint32_t)((int32_t)EDI + step);
    }
    if (rep) ECX = 0;
}
static inline void op_stosl32(CpuState *cpu, bool rep) {
    int32_t step = cpu->df ? -4 : 4;
    uint32_t cnt = rep ? ECX : 1;
    while (cnt--) {
        uint16_t ds = (uint16_t)(cpu->es + ((EDI >> 16) << 3));
        mem_w32(ds, (uint16_t)(EDI & 0xFFFFu), EAX);
        EDI = (uint32_t)((int32_t)EDI + step);
    }
    if (rep) ECX = 0;
}
static inline void op_movsb32(CpuState *cpu, uint16_t src_seg, bool rep) {
    int32_t step = cpu->df ? -1 : 1;
    uint32_t cnt = rep ? ECX : 1;
    while (cnt--) {
        uint16_t ss = (uint16_t)(src_seg + ((ESI >> 16) << 3));
        uint16_t ds = (uint16_t)(cpu->es + ((EDI >> 16) << 3));
        mem_w8(ds, (uint16_t)(EDI & 0xFFFFu), mem_r8(ss, (uint16_t)(ESI & 0xFFFFu)));
        ESI = (uint32_t)((int32_t)ESI + step);
        EDI = (uint32_t)((int32_t)EDI + step);
    }
    if (rep) ECX = 0;
}
static inline void op_movsl32(CpuState *cpu, uint16_t src_seg, bool rep) {
    int32_t step = cpu->df ? -4 : 4;
    uint32_t cnt = rep ? ECX : 1;
    while (cnt--) {
        uint16_t ss = (uint16_t)(src_seg + ((ESI >> 16) << 3));
        uint16_t ds = (uint16_t)(cpu->es + ((EDI >> 16) << 3));
        mem_w32(ds, (uint16_t)(EDI & 0xFFFFu), mem_r32(ss, (uint16_t)(ESI & 0xFFFFu)));
        ESI = (uint32_t)((int32_t)ESI + step);
        EDI = (uint32_t)((int32_t)EDI + step);
    }
    if (rep) ECX = 0;
}
static inline void op_movsb(CpuState *cpu, uint16_t src_seg, bool rep) {
    int16_t step = cpu->df ? -1 : 1;
    uint16_t cnt = rep ? CX : 1;
    while (cnt--) {
        mem_w8(cpu->es, DI, mem_r8(src_seg, SI));
        SI = (uint16_t)(SI + step);
        DI = (uint16_t)(DI + step);
    }
    if (rep) CX = 0;
}
static inline void op_movsw(CpuState *cpu, uint16_t src_seg, bool rep) {
    int16_t step = cpu->df ? -2 : 2;
    uint16_t cnt = rep ? CX : 1;
    while (cnt--) {
        mem_w16(cpu->es, DI, mem_r16(src_seg, SI));
        SI = (uint16_t)(SI + step);
        DI = (uint16_t)(DI + step);
    }
    if (rep) CX = 0;
}
static inline void op_movsl(CpuState *cpu, uint16_t src_seg, bool rep) {
    int16_t step = cpu->df ? -4 : 4;
    uint16_t cnt = rep ? CX : 1;
    while (cnt--) {
        mem_w32(cpu->es, DI, mem_r32(src_seg, SI));
        SI = (uint16_t)(SI + step);
        DI = (uint16_t)(DI + step);
    }
    if (rep) CX = 0;
}
static inline void op_scasb(CpuState *cpu, bool rep, bool repne) {
    int16_t step = cpu->df ? -1 : 1;
    if (!rep) {
        op_sub8(cpu, AL, mem_r8(cpu->es, DI));
        DI = (uint16_t)(DI + step);
        return;
    }
    while (CX != 0) {
        op_sub8(cpu, AL, mem_r8(cpu->es, DI));
        DI = (uint16_t)(DI + step);
        CX--;
        if (repne ? (cpu->zf != 0) : (cpu->zf == 0)) break;
    }
}
static inline void op_scasw(CpuState *cpu, bool rep, bool repne) {
    int16_t step = cpu->df ? -2 : 2;
    if (!rep) {
        op_sub16(cpu, AX, mem_r16(cpu->es, DI));
        DI = (uint16_t)(DI + step);
        return;
    }
    while (CX != 0) {
        op_sub16(cpu, AX, mem_r16(cpu->es, DI));
        DI = (uint16_t)(DI + step);
        CX--;
        if (repne ? (cpu->zf != 0) : (cpu->zf == 0)) break;
    }
}
static inline void op_cmpsb(CpuState *cpu, uint16_t src_seg, bool rep, bool repne) {
    int16_t step = cpu->df ? -1 : 1;
    if (!rep) {
        op_sub8(cpu, mem_r8(src_seg, SI), mem_r8(cpu->es, DI));
        SI = (uint16_t)(SI + step); DI = (uint16_t)(DI + step);
        return;
    }
    while (CX != 0) {
        op_sub8(cpu, mem_r8(src_seg, SI), mem_r8(cpu->es, DI));
        SI = (uint16_t)(SI + step); DI = (uint16_t)(DI + step);
        CX--;
        if (repne ? (cpu->zf != 0) : (cpu->zf == 0)) break;
    }
}
static inline void op_lodsb(CpuState *cpu, uint16_t src_seg) {
    int16_t step = cpu->df ? -1 : 1;
    AL = mem_r8(src_seg, SI);
    SI = (uint16_t)(SI + step);
}
static inline void op_lodsw(CpuState *cpu, uint16_t src_seg) {
    int16_t step = cpu->df ? -2 : 2;
    AX = mem_r16(src_seg, SI);
    SI = (uint16_t)(SI + step);
}

#endif /* BIGJOB_CPU_OPS_H */

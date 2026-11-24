#define _CRT_SECURE_NO_WARNINGS
#include "cpu.h"
void pic_auto(CPU *);
int main(void)
{
    static uint8_t memory[6 * 65536];
    static uint32_t bases[65536];
    CPU cpu = {0};
    cpu.mem = memory;
    cpu.mem_size = sizeof memory;
    cpu.sel_base = bases;
    for (unsigned i = 1; i <= 5; ++i) bases[i] = i * 65536u;
    cpu.ss = 1;
    enum { object = 0x40, table = 0x80, index = 3 };
    mem_write32(&cpu, 2, object + 0x42, (3u << 16) | table);
    mem_write32(&cpu, 2, object + 0x46, (4u << 16) | table);
    mem_write32(&cpu, 2, object + 0x4a, (5u << 16) | table);
    mem_write32(&cpu, 2, object + 0x4e, (4u << 16) | (table + 0x100));
    unsigned mode, flags, a, b, explicit_size, width, height;
    while (scanf("%u %u %u %u %u %u %u", &mode, &flags, &a, &b,
                 &explicit_size, &width, &height) == 7) {
        cpu.sp = 0xff00;
        cpu.bp = 0x1122;
        cpu.si = 0x3344;
        cpu.di = 0x5566;
        mem_write32(&cpu, 1, cpu.sp + 4, (2u << 16) | object);
        mem_write16(&cpu, 1, cpu.sp + 8, index);
        mem_write16(&cpu, 1, cpu.sp + 10, (uint16_t)mode);
        mem_write32(&cpu, 3, table + 4 * index, a);
        mem_write32(&cpu, 3, table + 4 * (index + 1), b);
        mem_write32(&cpu, 4, table + 4 * index, (height << 16) | width);
        mem_write32(&cpu, 4, table + 0x100 + 4 * index, explicit_size);
        mem_write16(&cpu, 5, table + 2 * index, (uint16_t)flags);
        pic_auto(&cpu);
        if (cpu.sp != 0xff04 || cpu.bp != 0x1122 ||
            cpu.si != 0x3344 || cpu.di != 0x5566) return 2;
        printf("%u\n", ((uint32_t)cpu.dx << 16) | cpu.ax);
    }
    return feof(stdin) ? 0 : 1;
}
uint16_t g_wsel;
void recomp_sel_write(uint16_t seg, uint16_t off, uint16_t val)
{
    (void)seg; (void)off; (void)val;
    abort();
}

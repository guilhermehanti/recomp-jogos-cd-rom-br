#include "../runtime/cpu.h"
#include "../runtime/win16_mem.h"
#include "../runtime/iso_vfs.h"
#include "../runtime/win16_api.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void recomp_dispatch_far(CpuState *cpu, uint16_t seg, uint16_t off) {
    (void)cpu; (void)seg; (void)off;
}
void recomp_dispatch_near(CpuState *cpu, uint16_t seg, uint16_t off) {
    (void)cpu; (void)seg; (void)off;
}

int main(void) {
    const char *iso = "D:\\emulacao pc antigo\\BIGJOBCD.ISO";
    assert(iso_vfs_init(iso, "build\\test_saves"));
    assert(win16_mem_init());
    assert(win16_api_init(true));

    /* 1. Verify ISO VFS locates core Brazilian assets */
    uint32_t lba = 0, sz = 0;
    assert(iso_vfs_find("BIGJOBNG.EXE", &lba, &sz) && sz == 667568);
    assert(iso_vfs_find("BIGJOB.MMS", &lba, &sz) && sz == 786711);
    assert(iso_vfs_find("BIGJOBCD.PIC", &lba, &sz));
    assert(iso_vfs_find("BIGJOBHD.PIC", &lba, &sz));
    assert(iso_vfs_find("BIGJOB.SND", &lba, &sz));
    assert(iso_vfs_find("BIGVOX.SND", &lba, &sz));
    assert(iso_vfs_find("BIGJOB.MOV", &lba, &sz));

    /* 2. Load BIGJOBNG.EXE directly from ISO into the 49 NE segments */
    size_t exe_sz = 0;
    uint8_t *exe = iso_vfs_read_all("BIGJOBNG.EXE", &exe_sz);
    assert(exe && exe_sz == 667568);
    assert(win16_load_ne_image(exe, exe_sz));
    free(exe);

    /* 3. Verify DGROUP compiler signature and relocated _INIT_ static constructor table */
    char sig[64];
    win16_read_cstr(NE_DGROUP_SEL, 0x004B, sig, sizeof(sig));
    assert(strstr(sig, "Borland C++ - Copyright 1994") != NULL);

    /* First static constructor at 49:20BA must be relocated to 2:15CC */
    uint16_t init0_off = mem_r16(NE_DGROUP_SEL, 0x20BA);
    uint16_t init0_sel = mem_r16(NE_DGROUP_SEL, 0x20BC);
    assert(init0_off == 0x15CC && init0_sel == NE_SEG_TO_SEL(2));

    /* 4. Verify NE RT_STRING loading (Portuguese strings) */
    char sbuf[128];
    size_t slen = 0;
    assert(win16_ne_load_string(1, sbuf, sizeof(sbuf), &slen) && strcmp(sbuf, "Big Job") == 0);
    assert(win16_ne_load_string(701, sbuf, sizeof(sbuf), &slen) && strcmp(sbuf, "BIGJOB.MMS") == 0);
    assert(win16_ne_load_string(704, sbuf, sizeof(sbuf), &slen) && strcmp(sbuf, "BIGJOB.MOV") == 0);

    /* 5. Verify huge GlobalAlloc (>64KB) with __AHSHIFT=3 selector arithmetic */
    uint16_t h = win16_global_alloc(0x0042, 150000);
    assert(h != 0);
    uint32_t ptr = win16_global_lock(h);
    uint16_t sel0 = (uint16_t)(ptr >> 16);
    uint16_t sel1 = (uint16_t)(sel0 + (1u << WIN16_AHSHIFT));
    uint16_t sel2 = (uint16_t)(sel1 + (1u << WIN16_AHSHIFT));
    mem_w32(sel0, 0x0000, 0x11223344u);
    mem_w32(sel1, 0x0010, 0x55667788u);
    mem_w32(sel2, 0x0020, 0x99AABBCCu);
    assert(mem_r32(sel0, 0x0000) == 0x11223344u);
    assert(mem_r32(sel1, 0x0010) == 0x55667788u);
    assert(mem_r32(sel2, 0x0020) == 0x99AABBCCu);
    assert(win16_global_size(h) == 150000);
    assert(win16_global_free(h) == 0);

    /* 6. Verify VFS _lopen / _llseek / _hread on BIGJOB.MMS directly from ISO */
    int16_t fd = vfs_lopen("D:\\BIGJOB.MMS", 0);
    assert(fd >= 10);
    uint8_t mms_hdr[16];
    assert(vfs_hread(fd, mms_hdr, 16) == 16);
    assert(memcmp(mms_hdr, "MMScript File", 13) == 0);
    assert(vfs_lclose(fd) == 0);

    /* 7. Verify Win16 API PASCAL stack bridge (USER!176 LoadString, GDI!128 MulDiv, INT 21h/30h) */
    CpuState cpu;
    memset(&cpu, 0, sizeof(cpu));
    cpu.ss = NE_DGROUP_SEL;
    cpu.ds = NE_DGROUP_SEL;
    cpu.esp.w = 0xF000;

    /* LoadString(hInst=1, uID=702, lpBuf=DGROUP:E000, nMax=64) */
    push16(&cpu, 1);
    push16(&cpu, 702);
    push32(&cpu, ((uint32_t)NE_DGROUP_SEL << 16) | 0xE000u);
    push16(&cpu, 64);
    win16_call_import(&cpu, "USER", 176);
    assert(cpu.esp.w == 0xF000);
    win16_read_cstr(NE_DGROUP_SEL, 0xE000, sbuf, sizeof(sbuf));
    assert(strcmp(sbuf, "BIGJOBCD.PIC,BIGJOBHD.PIC") == 0);

    /* MulDiv(100, 3, 4) == 75 */
    push16(&cpu, 100);
    push16(&cpu, 3);
    push16(&cpu, 4);
    win16_call_import(&cpu, "GDI", 128);
    assert(cpu.esp.w == 0xF000 && cpu.eax.w == 75);

    /* INT 21h / AH=30h */
    cpu.eax.b.h = 0x30;
    win16_int(&cpu, 0x21);
    assert(cpu.eax.b.l == 5 && cpu.cf == 0);

    win16_api_shutdown();
    win16_mem_shutdown();
    iso_vfs_shutdown();
    printf("PASS: test_runtime_base (ISO VFS + 49 NE segs + DGROUP relocs + NE strings + huge GlobalAlloc + Win16 API bridge)\n");
    return 0;
}

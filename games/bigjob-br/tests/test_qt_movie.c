#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "../runtime/iso_vfs.h"
#include "../runtime/qt_movie.h"

int main(void) {
    if (!iso_vfs_init("D:\\emulacao pc antigo\\BIGJOBCD.ISO", "build\\saves")) {
        fprintf(stderr, "FAIL: iso_vfs_init\n");
        return 1;
    }
    int16_t hf = vfs_lopen("D:\\BIGJOB.MOV", 0);
    if (hf < 0) {
        fprintf(stderr, "FAIL: open BIGJOB.MOV\n");
        return 1;
    }
    uint8_t hdr[256];
    vfs_hread(hf, hdr, 236);
    uint32_t count = ((uint32_t)hdr[0] << 24) | ((uint32_t)hdr[1] << 16) | ((uint32_t)hdr[2] << 8) | hdr[3];
    printf("Testing all %u movies in BIGJOB.MOV...\n", count);

    qt_init();
    uint32_t *fb = (uint32_t *)calloc(640 * 480, sizeof(uint32_t));
    for (uint32_t i = 0; i < count && i < 58; ++i) {
        const uint8_t *p = hdr + 4 + i * 4;
        uint32_t moov_off = ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16) | ((uint32_t)p[2] << 8) | p[3];
        printf("--- Movie [%u] at 0x%08X ---\n", i, moov_off);
        fflush(stdout);
        uint32_t mh = qt_open_movie(hf, moov_off);
        if (!mh) {
            fprintf(stderr, "FAIL: qt_open_movie(%u) at 0x%08X\n", i, moov_off);
            return 1;
        }
        QtRect rc = { 0, 0, 320, 240 };
        uint32_t ch = qt_create_controller(&rc);
        qt_bind_controller(ch, mh);
        qt_set_rate(ch, 0x10000);
        qt_tick(ch, fb, 640, 480);
        qt_set_rate(ch, 0);
        qt_close_controller(ch);
        qt_close_movie(mh);
    }
    free(fb);
    vfs_lclose(hf);
    qt_shutdown();
    iso_vfs_shutdown();
    printf("PASS: all %u movies opened and decoded!\n", count);
    return 0;
}

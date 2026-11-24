#include "video_filter.h"
#include <stdlib.h>

const char *video_filter_name(VideoFilterMode mode) {
    switch (mode) {
    case VF_HD_SMOOTH:      return "HD Contornos";
    case VF_SHARP_BILINEAR: return "Sharp-Bilinear";
    case VF_BILINEAR:       return "Suave (Bilinear)";
    case VF_CRT_SCANLINES:  return "CRT (Scanlines)";
    case VF_NEAREST:        return "Original (Pixelado)";
    default:                return "HD Contornos";
    }
}

/* Fast perceptual BGRX color distance (weighted RGB approximation of YUV distance) */
static inline int color_dist(uint32_t a, uint32_t b) {
    if (a == b) return 0;
    int db = (int)(a & 0xFFu) - (int)(b & 0xFFu);
    int dg = (int)((a >> 8) & 0xFFu) - (int)((b >> 8) & 0xFFu);
    int dr = (int)((a >> 16) & 0xFFu) - (int)((b >> 16) & 0xFFu);
    if (db < 0) db = -db;
    if (dg < 0) dg = -dg;
    if (dr < 0) dr = -dr;
    return (dr * 3 + dg * 4 + db * 2);
}

static inline uint32_t mix_50_50(uint32_t a, uint32_t b) {
    return ((a & 0x00FEFEFEu) >> 1) + ((b & 0x00FEFEFEu) >> 1) + (a & b & 0x00010101u);
}

static inline uint32_t mix_75_25(uint32_t a, uint32_t b) {
    return mix_50_50(a, mix_50_50(a, b));
}

static inline uint32_t mix_87_13(uint32_t a, uint32_t b) {
    return mix_50_50(a, mix_75_25(a, b));
}

static inline uint32_t darken_scanline(uint32_t c) {
    /* ~84% brightness: warm authentic CRT scanline without crushing dark areas */
    uint32_t r = ((c >> 16) & 0xFFu);
    uint32_t g = ((c >> 8)  & 0xFFu);
    uint32_t b = (c         & 0xFFu);
    r = (r * 214u) >> 8;
    g = (g * 214u) >> 8;
    b = (b * 214u) >> 8;
    return (r << 16) | (g << 8) | b;
}

static void filter_sharp_2x(const uint32_t *src, int w, int h, uint32_t *dst) {
    int dw = w * 2;
    for (int y = 0; y < h; ++y) {
        const uint32_t *srow = src + y * w;
        uint32_t *d0 = dst + (y * 2) * dw;
        uint32_t *d1 = d0 + dw;
        for (int x = 0; x < w; ++x) {
            uint32_t c = srow[x];
            d0[x * 2]     = c;
            d0[x * 2 + 1] = c;
            d1[x * 2]     = c;
            d1[x * 2 + 1] = c;
        }
    }
}

static void filter_crt_2x(const uint32_t *src, int w, int h, uint32_t *dst) {
    int dw = w * 2;
    for (int y = 0; y < h; ++y) {
        const uint32_t *srow = src + y * w;
        uint32_t *d0 = dst + (y * 2) * dw;
        uint32_t *d1 = d0 + dw;
        for (int x = 0; x < w; ++x) {
            uint32_t c  = srow[x];
            uint32_t cl = (x > 0)     ? srow[x - 1] : c;
            uint32_t cr = (x + 1 < w) ? srow[x + 1] : c;
            /* Gentle horizontal phosphor bleed softens 256-color dithering like a CRT */
            uint32_t p0 = mix_87_13(c, cl);
            uint32_t p1 = mix_87_13(c, cr);
            d0[x * 2]     = p0;
            d0[x * 2 + 1] = p1;
            d1[x * 2]     = darken_scanline(p0);
            d1[x * 2 + 1] = darken_scanline(p1);
        }
    }
}

/* Color-aware 2x edge-directed upscaler (Scale2x/xBR hybrid):
 * Smooths jagged diagonal contours on cartoon characters, outlines, and text
 * while preserving 1px horizontal/vertical lines and dithered backgrounds. */
static void filter_hd_smooth_2x(const uint32_t *src, int w, int h, uint32_t *dst) {
    const int SIM_THR = 140;  /* Colors closer than this are treated as same contour */
    int dw = w * 2;

    for (int y = 0; y < h; ++y) {
        int ym1 = (y > 0) ? (y - 1) : 0;
        int yp1 = (y + 1 < h) ? (y + 1) : (h - 1);
        const uint32_t *r_up = src + ym1 * w;
        const uint32_t *r_md = src + y   * w;
        const uint32_t *r_dn = src + yp1 * w;
        uint32_t *d0 = dst + (y * 2) * dw;
        uint32_t *d1 = d0 + dw;

        for (int x = 0; x < w; ++x) {
            int xm1 = (x > 0) ? (x - 1) : 0;
            int xp1 = (x + 1 < w) ? (x + 1) : (w - 1);

            uint32_t A = r_up[xm1], B = r_up[x], C = r_up[xp1];
            uint32_t D = r_md[xm1], E = r_md[x], F = r_md[xp1];
            uint32_t G = r_dn[xm1], H = r_dn[x], I = r_dn[xp1];

            uint32_t e0 = E, e1 = E, e2 = E, e3 = E;

            /* Only smooth diagonals when opposite cardinal neighbors actually differ
             * (prevents smearing 1px lines and 256-color checker dithering). */
            if (color_dist(B, H) > SIM_THR && color_dist(D, F) > SIM_THR) {
                /* Top-left corner: B vs D */
                if (color_dist(B, D) <= SIM_THR && color_dist(E, B) > SIM_THR) {
                    uint32_t edge = mix_50_50(B, D);
                    int strong = (color_dist(E, A) > SIM_THR) &&
                                 (color_dist(B, C) <= SIM_THR || color_dist(D, G) <= SIM_THR);
                    e0 = strong ? mix_75_25(edge, E) : mix_50_50(edge, E);
                }
                /* Top-right corner: B vs F */
                if (color_dist(B, F) <= SIM_THR && color_dist(E, B) > SIM_THR) {
                    uint32_t edge = mix_50_50(B, F);
                    int strong = (color_dist(E, C) > SIM_THR) &&
                                 (color_dist(B, A) <= SIM_THR || color_dist(F, I) <= SIM_THR);
                    e1 = strong ? mix_75_25(edge, E) : mix_50_50(edge, E);
                }
                /* Bottom-left corner: H vs D */
                if (color_dist(H, D) <= SIM_THR && color_dist(E, H) > SIM_THR) {
                    uint32_t edge = mix_50_50(H, D);
                    int strong = (color_dist(E, G) > SIM_THR) &&
                                 (color_dist(H, I) <= SIM_THR || color_dist(D, A) <= SIM_THR);
                    e2 = strong ? mix_75_25(edge, E) : mix_50_50(edge, E);
                }
                /* Bottom-right corner: H vs F */
                if (color_dist(H, F) <= SIM_THR && color_dist(E, H) > SIM_THR) {
                    uint32_t edge = mix_50_50(H, F);
                    int strong = (color_dist(E, I) > SIM_THR) &&
                                 (color_dist(H, G) <= SIM_THR || color_dist(F, C) <= SIM_THR);
                    e3 = strong ? mix_75_25(edge, E) : mix_50_50(edge, E);
                }
            }

            d0[x * 2]     = e0;
            d0[x * 2 + 1] = e1;
            d1[x * 2]     = e2;
            d1[x * 2 + 1] = e3;
        }
    }
}

int video_filter_apply_2x(VideoFilterMode mode,
                          const uint32_t *src, int w, int h,
                          uint32_t *dst) {
    if (!src || !dst || w <= 0 || h <= 0) return 0;
    switch (mode) {
    case VF_HD_SMOOTH:
        filter_hd_smooth_2x(src, w, h, dst);
        return 1;
    case VF_SHARP_BILINEAR:
        filter_sharp_2x(src, w, h, dst);
        return 1;
    case VF_CRT_SCANLINES:
        filter_crt_2x(src, w, h, dst);
        return 1;
    case VF_BILINEAR:
    case VF_NEAREST:
    default:
        return 0;
    }
}

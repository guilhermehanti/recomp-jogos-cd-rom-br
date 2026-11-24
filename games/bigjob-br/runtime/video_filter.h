#ifndef BIGJOB_VIDEO_FILTER_H
#define BIGJOB_VIDEO_FILTER_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum VideoFilterMode {
    VF_HD_SMOOTH      = 0, /* 2x edge-directed cartoon/contour smoothing + Halftone */
    VF_SHARP_BILINEAR = 1, /* 2x integer prescale + Halftone (eliminates uneven pixels) */
    VF_BILINEAR       = 2, /* Smooth GDI Halftone (softens 256-color dithering) */
    VF_CRT_SCANLINES  = 3, /* 2x CRT-style horizontal dither blend + subtle scanlines */
    VF_NEAREST        = 4, /* Raw nearest-neighbor (original pixelated look) */
    VF_COUNT          = 5
} VideoFilterMode;

const char *video_filter_name(VideoFilterMode mode);

/* Upscale 640x480 BGRX framebuffer to 1280x960 BGRX (dst must hold 1280*960 uint32_t).
 * Returns true if dst was populated (1280x960 output), false if caller should blit src directly. */
int video_filter_apply_2x(VideoFilterMode mode,
                          const uint32_t *src, int w, int h,
                          uint32_t *dst);

#ifdef __cplusplus
}
#endif

#endif /* BIGJOB_VIDEO_FILTER_H */

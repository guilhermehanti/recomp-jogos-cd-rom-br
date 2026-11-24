#ifndef BIGJOB_QT_MOVIE_H
#define BIGJOB_QT_MOVIE_H

#include <stdint.h>
#include <stdbool.h>

typedef struct QtRect {
    int16_t left, top, right, bottom;
} QtRect;

void     qt_init(void);
void     qt_shutdown(void);

/* Open a movie at moov_offset inside an open VFS file handle (BIGJOB.MOV). Returns handle >= 1 or 0 on error. */
uint32_t qt_open_movie(int16_t vfs_handle, uint32_t moov_offset);
void     qt_close_movie(uint32_t movie_handle);
uint32_t qt_get_timescale(uint32_t movie_handle);

/* Controller lifecycle */
uint32_t qt_create_controller(const QtRect *rc);
void     qt_close_controller(uint32_t ctrl_handle);
void     qt_bind_controller(uint32_t ctrl_handle, uint32_t movie_handle);

/* Playback control */
void     qt_set_rate(uint32_t ctrl_handle, uint32_t rate_fixed16);
void     qt_seek(uint32_t ctrl_handle, uint32_t time_val);
bool     qt_is_playing(uint32_t ctrl_handle);

/* Frame advance & blitting into host 640x480 BGRX framebuffer. Returns true if framebuffer was updated. */
bool     qt_tick(uint32_t ctrl_handle, uint32_t *fb, int fb_w, int fb_h);
bool     qt_paint(uint32_t ctrl_handle, uint32_t *fb, int fb_w, int fb_h);

#endif /* BIGJOB_QT_MOVIE_H */

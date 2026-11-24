#define _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mmsystem.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "qt_movie.h"
#include "iso_vfs.h"

/* ---- Big-endian helpers ---- */
static inline uint16_t be16(const uint8_t *p) {
    return (uint16_t)(((uint16_t)p[0] << 8) | p[1]);
}
static inline uint32_t be32(const uint8_t *p) {
    return ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16) |
           ((uint32_t)p[2] << 8)  | (uint32_t)p[3];
}
static inline uint32_t fourcc(char a, char b, char c, char d) {
    return ((uint32_t)(uint8_t)a << 24) | ((uint32_t)(uint8_t)b << 16) |
           ((uint32_t)(uint8_t)c << 8)  | (uint32_t)(uint8_t)d;
}

/* ---- Cinepak (cvid) decoder ---- */
typedef struct CvCodebook {
    uint8_t y[4];
    int8_t  u, v;
} CvCodebook;

typedef struct CvStrip {
    CvCodebook v4[256];
    CvCodebook v1[256];
} CvStrip;

#define MAX_CV_STRIPS 16

static inline uint8_t clamp_u8(int x) {
    if (x < 0) return 0;
    if (x > 255) return 255;
    return (uint8_t)x;
}

static inline uint32_t yuv_to_bgrx(uint8_t y, int8_t u, int8_t v) {
    int r = (int)y + ((int)v * 2);
    int g = (int)y - (((int)u) >> 1) - (int)v;
    int b = (int)y + ((int)u * 2);
    return ((uint32_t)clamp_u8(r) << 16) | ((uint32_t)clamp_u8(g) << 8) | (uint32_t)clamp_u8(b);
}

static inline void put_px(uint32_t *rgb, int w, int h, int x, int y, uint32_t c) {
    if (x >= 0 && x < w && y >= 0 && y < h) rgb[y * w + x] = c;
}

static void cv_draw_v1(uint32_t *rgb, int w, int h, int x, int y, const CvCodebook *cb) {
    uint32_t c0 = yuv_to_bgrx(cb->y[0], cb->u, cb->v);
    uint32_t c1 = yuv_to_bgrx(cb->y[1], cb->u, cb->v);
    uint32_t c2 = yuv_to_bgrx(cb->y[2], cb->u, cb->v);
    uint32_t c3 = yuv_to_bgrx(cb->y[3], cb->u, cb->v);
    put_px(rgb, w, h, x + 0, y + 0, c0); put_px(rgb, w, h, x + 1, y + 0, c0);
    put_px(rgb, w, h, x + 2, y + 0, c1); put_px(rgb, w, h, x + 3, y + 0, c1);
    put_px(rgb, w, h, x + 0, y + 1, c0); put_px(rgb, w, h, x + 1, y + 1, c0);
    put_px(rgb, w, h, x + 2, y + 1, c1); put_px(rgb, w, h, x + 3, y + 1, c1);
    put_px(rgb, w, h, x + 0, y + 2, c2); put_px(rgb, w, h, x + 1, y + 2, c2);
    put_px(rgb, w, h, x + 2, y + 2, c3); put_px(rgb, w, h, x + 3, y + 2, c3);
    put_px(rgb, w, h, x + 0, y + 3, c2); put_px(rgb, w, h, x + 1, y + 3, c2);
    put_px(rgb, w, h, x + 2, y + 3, c3); put_px(rgb, w, h, x + 3, y + 3, c3);
}

static void cv_draw_v4_2x2(uint32_t *rgb, int w, int h, int x, int y, const CvCodebook *cb) {
    put_px(rgb, w, h, x + 0, y + 0, yuv_to_bgrx(cb->y[0], cb->u, cb->v));
    put_px(rgb, w, h, x + 1, y + 0, yuv_to_bgrx(cb->y[1], cb->u, cb->v));
    put_px(rgb, w, h, x + 0, y + 1, yuv_to_bgrx(cb->y[2], cb->u, cb->v));
    put_px(rgb, w, h, x + 1, y + 1, yuv_to_bgrx(cb->y[3], cb->u, cb->v));
}

static void cv_load_codebook(const uint8_t *p, uint32_t len, CvCodebook *cb, bool selective, bool grey) {
    uint32_t pos = 0;
    int entry_sz = grey ? 4 : 6;
    if (!selective) {
        int n = (int)(len / (uint32_t)entry_sz);
        if (n > 256) n = 256;
        for (int i = 0; i < n; ++i) {
            cb[i].y[0] = p[pos + 0]; cb[i].y[1] = p[pos + 1];
            cb[i].y[2] = p[pos + 2]; cb[i].y[3] = p[pos + 3];
            cb[i].u = grey ? 0 : (int8_t)p[pos + 4];
            cb[i].v = grey ? 0 : (int8_t)p[pos + 5];
            pos += (uint32_t)entry_sz;
        }
    } else {
        int idx = 0;
        while (idx < 256 && pos + 4 <= len) {
            uint32_t mask = be32(p + pos);
            pos += 4;
            for (int b = 0; b < 32 && idx < 256; ++b, ++idx) {
                if (mask & (0x80000000u >> b)) {
                    if (pos + (uint32_t)entry_sz > len) return;
                    cb[idx].y[0] = p[pos + 0]; cb[idx].y[1] = p[pos + 1];
                    cb[idx].y[2] = p[pos + 2]; cb[idx].y[3] = p[pos + 3];
                    cb[idx].u = grey ? 0 : (int8_t)p[pos + 4];
                    cb[idx].v = grey ? 0 : (int8_t)p[pos + 5];
                    pos += (uint32_t)entry_sz;
                }
            }
        }
    }
}

static void cv_decode_frame(const uint8_t *data, uint32_t size, uint32_t *rgb, int w, int h, CvStrip *strips) {
    if (size < 10) return;
    uint16_t num_strips = be16(data + 8);
    if (num_strips > MAX_CV_STRIPS) num_strips = MAX_CV_STRIPS;

    uint32_t pos = 10;
    int y_base = 0;
    for (uint16_t s = 0; s < num_strips; ++s) {
        if (pos + 12 > size) break;
        uint16_t strip_id = be16(data + pos);
        uint16_t strip_sz = be16(data + pos + 2);
        int y1 = (int)be16(data + pos + 4);
        int x1 = (int)be16(data + pos + 6);
        int y2 = (int)be16(data + pos + 8);
        int x2 = (int)be16(data + pos + 10);
        if (strip_sz < 12 || pos + strip_sz > size) break;

        if (s > 0 && !(strip_id & 0x0100)) {
            /* Inherit previous strip's codebooks if needed */
        }
        int sy_top = (s == 0) ? y1 : y_base;
        int sy_bot = sy_top + (y2 - y1);
        if (y2 > y_base) { sy_top = y1; sy_bot = y2; }
        y_base = sy_bot;
        if (x2 > w) x2 = w;
        if (sy_bot > h) sy_bot = h;

        uint32_t cpos = pos + 12;
        uint32_t send = pos + strip_sz;
        while (cpos + 4 <= send) {
            uint16_t cid = be16(data + cpos);
            uint16_t csz = be16(data + cpos + 2);
            if (csz < 4 || cpos + csz > send) break;
            const uint8_t *cp = data + cpos + 4;
            uint32_t clen = (uint32_t)(csz - 4);

            switch (cid) {
            case 0x2000: cv_load_codebook(cp, clen, strips[s].v4, false, false); break;
            case 0x2100: cv_load_codebook(cp, clen, strips[s].v4, true,  false); break;
            case 0x2200: cv_load_codebook(cp, clen, strips[s].v1, false, false); break;
            case 0x2300: cv_load_codebook(cp, clen, strips[s].v1, true,  false); break;
            case 0x2400: cv_load_codebook(cp, clen, strips[s].v4, false, true);  break;
            case 0x2500: cv_load_codebook(cp, clen, strips[s].v4, true,  true);  break;
            case 0x2600: cv_load_codebook(cp, clen, strips[s].v1, false, true);  break;
            case 0x2700: cv_load_codebook(cp, clen, strips[s].v1, true,  true);  break;
            case 0x3000: { /* Intra V1/V4 */
                uint32_t p = 0;
                uint32_t flags = 0;
                int fbits = 0;
                for (int by = sy_top; by < sy_bot; by += 4) {
                    for (int bx = x1; bx < x2; bx += 4) {
                        if (fbits == 0) {
                            if (p + 4 > clen) goto done_chunk;
                            flags = be32(cp + p); p += 4; fbits = 32;
                        }
                        bool is_v4 = (flags & 0x80000000u) != 0;
                        flags <<= 1; fbits--;
                        if (is_v4) {
                            if (p + 4 > clen) goto done_chunk;
                            cv_draw_v4_2x2(rgb, w, h, bx + 0, by + 0, &strips[s].v4[cp[p + 0]]);
                            cv_draw_v4_2x2(rgb, w, h, bx + 2, by + 0, &strips[s].v4[cp[p + 1]]);
                            cv_draw_v4_2x2(rgb, w, h, bx + 0, by + 2, &strips[s].v4[cp[p + 2]]);
                            cv_draw_v4_2x2(rgb, w, h, bx + 2, by + 2, &strips[s].v4[cp[p + 3]]);
                            p += 4;
                        } else {
                            if (p + 1 > clen) goto done_chunk;
                            cv_draw_v1(rgb, w, h, bx, by, &strips[s].v1[cp[p++]]);
                        }
                    }
                }
                break;
            }
            case 0x3100: { /* Inter V1/V4 */
                uint32_t p = 0;
                uint32_t flags = 0;
                int fbits = 0;
                for (int by = sy_top; by < sy_bot; by += 4) {
                    for (int bx = x1; bx < x2; bx += 4) {
                        if (fbits == 0) {
                            if (p + 4 > clen) goto done_chunk;
                            flags = be32(cp + p); p += 4; fbits = 32;
                        }
                        bool coded = (flags & 0x80000000u) != 0;
                        flags <<= 1; fbits--;
                        if (!coded) continue;
                        if (fbits == 0) {
                            if (p + 4 > clen) goto done_chunk;
                            flags = be32(cp + p); p += 4; fbits = 32;
                        }
                        bool is_v4 = (flags & 0x80000000u) != 0;
                        flags <<= 1; fbits--;
                        if (is_v4) {
                            if (p + 4 > clen) goto done_chunk;
                            cv_draw_v4_2x2(rgb, w, h, bx + 0, by + 0, &strips[s].v4[cp[p + 0]]);
                            cv_draw_v4_2x2(rgb, w, h, bx + 2, by + 0, &strips[s].v4[cp[p + 1]]);
                            cv_draw_v4_2x2(rgb, w, h, bx + 0, by + 2, &strips[s].v4[cp[p + 2]]);
                            cv_draw_v4_2x2(rgb, w, h, bx + 2, by + 2, &strips[s].v4[cp[p + 3]]);
                            p += 4;
                        } else {
                            if (p + 1 > clen) goto done_chunk;
                            cv_draw_v1(rgb, w, h, bx, by, &strips[s].v1[cp[p++]]);
                        }
                    }
                }
                break;
            }
            case 0x3200: { /* All V1 */
                uint32_t p = 0;
                for (int by = sy_top; by < sy_bot; by += 4) {
                    for (int bx = x1; bx < x2; bx += 4) {
                        if (p >= clen) goto done_chunk;
                        cv_draw_v1(rgb, w, h, bx, by, &strips[s].v1[cp[p++]]);
                    }
                }
                break;
            }
            default:
                break;
            }
done_chunk:
            cpos += csz;
        }
        pos += strip_sz;
    }
}

/* ---- QuickTime moov parser & movie state ---- */
typedef struct QtSample {
    uint32_t offset;
    uint32_t size;
    uint32_t pts_ms;
} QtSample;

typedef struct QtMovie {
    bool      in_use;
    int16_t   vfs_handle;
    uint32_t  movie_timescale;
    uint32_t  duration_ms;

    /* Video track */
    int       vid_w, vid_h;
    QtSample *vid_samples;
    uint32_t  vid_count;
    uint32_t *rgb_buf;
    CvStrip   strips[MAX_CV_STRIPS];
    int       cur_frame;

    /* Audio track (raw PCM) */
    uint8_t  *pcm_data;
    uint32_t  pcm_size;
    uint32_t  audio_rate;
    uint16_t  audio_channels;
    uint16_t  audio_bits;
} QtMovie;

typedef struct QtController {
    bool      in_use;
    QtRect    rc;
    uint32_t  movie_handle;
    bool      playing;
    DWORD     start_tick;
    uint32_t  base_ms;
    HWAVEOUT  hwo;
    WAVEHDR   whdr;
    bool      audio_started;
} QtController;

#define MAX_QT_MOVIES 8
#define MAX_QT_CTRLS  8
static QtMovie      g_movies[MAX_QT_MOVIES];
static QtController g_ctrls[MAX_QT_CTRLS];

typedef struct SttsEntry { uint32_t count, dur; } SttsEntry;
typedef struct StscEntry { uint32_t first_chunk, samples_per_chunk, sdi; } StscEntry;

typedef struct ParsedTrack {
    uint32_t   handler; /* 'vide' or 'soun' */
    uint32_t   timescale;
    uint32_t   duration;
    int        vid_w, vid_h;
    uint32_t   audio_rate;
    uint16_t   audio_channels;
    uint16_t   audio_bits;
    SttsEntry *stts; uint32_t stts_cnt;
    StscEntry *stsc; uint32_t stsc_cnt;
    uint32_t   stsz_fixed;
    uint32_t  *stsz; uint32_t stsz_cnt;
    uint32_t  *stco; uint32_t stco_cnt;
} ParsedTrack;

static void free_parsed_track(ParsedTrack *t) {
    free(t->stts); free(t->stsc); free(t->stsz); free(t->stco);
    memset(t, 0, sizeof(*t));
}

static void parse_stbl(const uint8_t *buf, uint32_t len, ParsedTrack *t) {
    uint32_t pos = 0;
    while (pos + 8 <= len) {
        uint32_t sz = be32(buf + pos);
        uint32_t ty = be32(buf + pos + 4);
        if (sz < 8 || pos + sz > len) break;
        const uint8_t *p = buf + pos + 8;
        uint32_t plen = sz - 8;

        if (ty == fourcc('s','t','s','d') && plen >= 16) {
            uint32_t esz = be32(p + 8);
            const uint8_t *ep = p + 8;
            if (esz <= plen - 8) {
                if (t->handler == fourcc('v','i','d','e') && esz >= 36) {
                    t->vid_w = (int)be16(ep + 32);
                    t->vid_h = (int)be16(ep + 34);
                } else if (t->handler == fourcc('s','o','u','n') && esz >= 36) {
                    t->audio_channels = be16(ep + 24);
                    t->audio_bits     = be16(ep + 26);
                    t->audio_rate     = be32(ep + 32) >> 16;
                }
            }
        } else if (ty == fourcc('s','t','t','s') && plen >= 8) {
            uint32_t cnt = be32(p + 4);
            if (cnt <= (plen - 8) / 8) {
                t->stts = (SttsEntry *)malloc(cnt * sizeof(SttsEntry));
                t->stts_cnt = cnt;
                for (uint32_t i = 0; i < cnt; ++i) {
                    t->stts[i].count = be32(p + 8 + i * 8);
                    t->stts[i].dur   = be32(p + 12 + i * 8);
                }
            }
        } else if (ty == fourcc('s','t','s','c') && plen >= 8) {
            uint32_t cnt = be32(p + 4);
            if (cnt <= (plen - 8) / 12) {
                t->stsc = (StscEntry *)malloc(cnt * sizeof(StscEntry));
                t->stsc_cnt = cnt;
                for (uint32_t i = 0; i < cnt; ++i) {
                    t->stsc[i].first_chunk       = be32(p + 8 + i * 12);
                    t->stsc[i].samples_per_chunk = be32(p + 12 + i * 12);
                    t->stsc[i].sdi               = be32(p + 16 + i * 12);
                }
            }
        } else if (ty == fourcc('s','t','s','z') && plen >= 12) {
            t->stsz_fixed = be32(p + 4);
            uint32_t cnt  = be32(p + 8);
            t->stsz_cnt   = cnt;
            if (t->stsz_fixed == 0 && cnt <= (plen - 12) / 4) {
                t->stsz = (uint32_t *)malloc(cnt * sizeof(uint32_t));
                for (uint32_t i = 0; i < cnt; ++i) t->stsz[i] = be32(p + 12 + i * 4);
            }
        } else if (ty == fourcc('s','t','c','o') && plen >= 8) {
            uint32_t cnt = be32(p + 4);
            if (cnt <= (plen - 8) / 4) {
                t->stco = (uint32_t *)malloc(cnt * sizeof(uint32_t));
                t->stco_cnt = cnt;
                for (uint32_t i = 0; i < cnt; ++i) t->stco[i] = be32(p + 8 + i * 4);
            }
        }
        pos += sz;
    }
}

static void parse_trak(const uint8_t *buf, uint32_t len, ParsedTrack *t) {
    uint32_t pos = 0;
    while (pos + 8 <= len) {
        uint32_t sz = be32(buf + pos);
        uint32_t ty = be32(buf + pos + 4);
        if (sz < 8 || pos + sz > len) break;
        const uint8_t *p = buf + pos + 8;
        uint32_t plen = sz - 8;

        if (ty == fourcc('m','d','i','a') || ty == fourcc('m','i','n','f')) {
            parse_trak(p, plen, t);
        } else if (ty == fourcc('s','t','b','l')) {
            parse_stbl(p, plen, t);
        } else if (ty == fourcc('m','d','h','d') && plen >= 20) {
            t->timescale = be32(p + 12);
            t->duration  = be32(p + 16);
        } else if (ty == fourcc('h','d','l','r') && plen >= 12) {
            uint32_t sub = be32(p + 8);
            if (sub == fourcc('v','i','d','e') || sub == fourcc('s','o','u','n')) {
                t->handler = sub;
            }
        }
        pos += sz;
    }
}

static void build_video_samples(const ParsedTrack *t, QtMovie *m) {
    if (t->stsz_cnt == 0 || t->stco_cnt == 0 || t->stsc_cnt == 0) return;
    m->vid_samples = (QtSample *)calloc(t->stsz_cnt, sizeof(QtSample));
    m->vid_count = t->stsz_cnt;
    m->vid_w = t->vid_w > 0 ? t->vid_w : 320;
    m->vid_h = t->vid_h > 0 ? t->vid_h : 240;
    m->rgb_buf = (uint32_t *)calloc((size_t)m->vid_w * (size_t)m->vid_h, sizeof(uint32_t));

    /* Compute sample file offsets from stsc + stco + stsz */
    uint32_t sidx = 0;
    for (uint32_t c = 0; c < t->stco_cnt && sidx < t->stsz_cnt; ++c) {
        uint32_t chunk1 = c + 1;
        uint32_t spc = t->stsc[0].samples_per_chunk;
        for (uint32_t e = 0; e < t->stsc_cnt; ++e) {
            if (t->stsc[e].first_chunk <= chunk1) spc = t->stsc[e].samples_per_chunk;
            else break;
        }
        uint32_t off = t->stco[c];
        for (uint32_t k = 0; k < spc && sidx < t->stsz_cnt; ++k, ++sidx) {
            uint32_t ssz = t->stsz_fixed ? t->stsz_fixed : t->stsz[sidx];
            m->vid_samples[sidx].offset = off;
            m->vid_samples[sidx].size   = ssz;
            off += ssz;
        }
    }

    /* Compute sample timestamps in ms from stts */
    uint32_t ts = t->timescale ? t->timescale : 600;
    uint64_t cur_t = 0;
    sidx = 0;
    for (uint32_t e = 0; e < t->stts_cnt && sidx < t->stsz_cnt; ++e) {
        for (uint32_t k = 0; k < t->stts[e].count && sidx < t->stsz_cnt; ++k, ++sidx) {
            m->vid_samples[sidx].pts_ms = (uint32_t)((cur_t * 1000ULL) / ts);
            cur_t += t->stts[e].dur;
        }
    }
    uint32_t dur_ms = (uint32_t)((cur_t * 1000ULL) / ts);
    if (dur_ms > m->duration_ms) m->duration_ms = dur_ms;
}

static void load_audio_track(const ParsedTrack *t, QtMovie *m) {
    if (t->stco_cnt == 0 || t->stsc_cnt == 0 || t->stsz_cnt == 0) return;
    m->audio_rate     = t->audio_rate ? t->audio_rate : 22050;
    m->audio_channels = t->audio_channels ? t->audio_channels : 1;
    m->audio_bits     = t->audio_bits ? t->audio_bits : 8;
    uint32_t bpf = (m->audio_bits / 8) * m->audio_channels;
    if (bpf == 0) bpf = 1;
    uint32_t total_bytes = t->stsz_cnt * (t->stsz_fixed ? t->stsz_fixed : bpf);
    if (total_bytes == 0 || total_bytes > 16 * 1024 * 1024) return;

    m->pcm_data = (uint8_t *)malloc(total_bytes);
    if (!m->pcm_data) return;

    uint32_t written = 0;
    uint32_t sidx = 0;
    for (uint32_t c = 0; c < t->stco_cnt && sidx < t->stsz_cnt; ++c) {
        uint32_t chunk1 = c + 1;
        uint32_t spc = t->stsc[0].samples_per_chunk;
        for (uint32_t e = 0; e < t->stsc_cnt; ++e) {
            if (t->stsc[e].first_chunk <= chunk1) spc = t->stsc[e].samples_per_chunk;
            else break;
        }
        if (sidx + spc > t->stsz_cnt) spc = t->stsz_cnt - sidx;
        uint32_t cbytes = spc * (t->stsz_fixed ? t->stsz_fixed : bpf);
        if (written + cbytes > total_bytes) cbytes = total_bytes - written;
        vfs_llseek(m->vfs_handle, (int32_t)t->stco[c], 0);
        int32_t got = vfs_hread(m->vfs_handle, m->pcm_data + written, cbytes);
        if (got > 0) written += (uint32_t)got;
        sidx += spc;
    }
    m->pcm_size = written;
    uint32_t aud_ms = (uint32_t)(((uint64_t)written * 1000ULL) / ((uint64_t)m->audio_rate * bpf));
    if (aud_ms > m->duration_ms) m->duration_ms = aud_ms;
}

void qt_init(void) {
    memset(g_movies, 0, sizeof(g_movies));
    memset(g_ctrls, 0, sizeof(g_ctrls));
}

void qt_shutdown(void) {
    for (uint32_t i = 1; i < MAX_QT_CTRLS; ++i) qt_close_controller(i);
    for (uint32_t i = 1; i < MAX_QT_MOVIES; ++i) qt_close_movie(i);
}

uint32_t qt_open_movie(int16_t vfs_handle, uint32_t moov_offset) {
    uint8_t hdr[8];
    vfs_llseek(vfs_handle, (int32_t)moov_offset, 0);
    if (vfs_hread(vfs_handle, hdr, 8) != 8) return 0;
    uint32_t moov_sz = be32(hdr);
    uint32_t moov_ty = be32(hdr + 4);
    if (moov_ty != fourcc('m','o','o','v') || moov_sz < 16 || moov_sz > 4 * 1024 * 1024) {
        fprintf(stderr, "[qt] Invalid moov at 0x%08X (sz=%u ty=%08X)\n", moov_offset, moov_sz, moov_ty);
        return 0;
    }

    uint8_t *moov = (uint8_t *)malloc(moov_sz - 8);
    if (!moov) return 0;
    if (vfs_hread(vfs_handle, moov, moov_sz - 8) != (int32_t)(moov_sz - 8)) {
        free(moov);
        return 0;
    }

    uint32_t mh = 0;
    for (uint32_t i = 1; i < MAX_QT_MOVIES; ++i) {
        if (!g_movies[i].in_use) { mh = i; break; }
    }
    if (!mh) { free(moov); return 0; }

    QtMovie *m = &g_movies[mh];
    memset(m, 0, sizeof(*m));
    m->in_use = true;
    m->vfs_handle = vfs_handle;
    m->movie_timescale = 600;
    m->cur_frame = -1;

    uint32_t pos = 0, mlen = moov_sz - 8;
    while (pos + 8 <= mlen) {
        uint32_t sz = be32(moov + pos);
        uint32_t ty = be32(moov + pos + 4);
        if (sz < 8 || pos + sz > mlen) break;
        const uint8_t *p = moov + pos + 8;
        uint32_t plen = sz - 8;
        if (ty == fourcc('m','v','h','d') && plen >= 20) {
            m->movie_timescale = be32(p + 12);
            uint32_t mdur = be32(p + 16);
            if (m->movie_timescale) m->duration_ms = (uint32_t)(((uint64_t)mdur * 1000ULL) / m->movie_timescale);
        } else if (ty == fourcc('t','r','a','k')) {
            ParsedTrack tr;
            memset(&tr, 0, sizeof(tr));
            parse_trak(p, plen, &tr);
            if (tr.handler == fourcc('v','i','d','e')) build_video_samples(&tr, m);
            else if (tr.handler == fourcc('s','o','u','n')) load_audio_track(&tr, m);
            free_parsed_track(&tr);
        }
        pos += sz;
    }
    free(moov);
    fprintf(stderr, "[qt] Opened movie #%u at 0x%08X: %dx%d %u frames, audio=%uHz %uch %ubit (%u bytes), dur=%ums\n",
            mh, moov_offset, m->vid_w, m->vid_h, m->vid_count,
            m->audio_rate, m->audio_channels, m->audio_bits, m->pcm_size, m->duration_ms);
    return mh;
}

void qt_close_movie(uint32_t mh) {
    if (mh == 0 || mh >= MAX_QT_MOVIES || !g_movies[mh].in_use) return;
    free(g_movies[mh].vid_samples);
    free(g_movies[mh].rgb_buf);
    free(g_movies[mh].pcm_data);
    memset(&g_movies[mh], 0, sizeof(QtMovie));
}

uint32_t qt_get_timescale(uint32_t mh) {
    if (mh == 0 || mh >= MAX_QT_MOVIES || !g_movies[mh].in_use) return 600;
    return g_movies[mh].movie_timescale ? g_movies[mh].movie_timescale : 600;
}

uint32_t qt_create_controller(const QtRect *rc) {
    for (uint32_t i = 1; i < MAX_QT_CTRLS; ++i) {
        if (!g_ctrls[i].in_use) {
            memset(&g_ctrls[i], 0, sizeof(QtController));
            g_ctrls[i].in_use = true;
            if (rc) g_ctrls[i].rc = *rc;
            return i;
        }
    }
    return 0;
}

static void ctrl_stop_audio(QtController *c) {
    if (c->hwo) {
        waveOutReset(c->hwo);
        if (c->whdr.dwFlags & WHDR_PREPARED) {
            waveOutUnprepareHeader(c->hwo, &c->whdr, sizeof(WAVEHDR));
        }
        waveOutClose(c->hwo);
        c->hwo = NULL;
    }
    c->audio_started = false;
}

void qt_close_controller(uint32_t ch) {
    if (ch == 0 || ch >= MAX_QT_CTRLS || !g_ctrls[ch].in_use) return;
    ctrl_stop_audio(&g_ctrls[ch]);
    memset(&g_ctrls[ch], 0, sizeof(QtController));
}

void qt_bind_controller(uint32_t ch, uint32_t mh) {
    if (ch == 0 || ch >= MAX_QT_CTRLS || !g_ctrls[ch].in_use) return;
    g_ctrls[ch].movie_handle = mh;
}

void qt_set_rate(uint32_t ch, uint32_t rate_fixed16) {
    if (ch == 0 || ch >= MAX_QT_CTRLS || !g_ctrls[ch].in_use) return;
    QtController *c = &g_ctrls[ch];
    if (rate_fixed16 == 0) {
        c->playing = false;
        ctrl_stop_audio(c);
        return;
    }
    c->playing = true;
    c->start_tick = timeGetTime();
    uint32_t mh = c->movie_handle;
    if (mh > 0 && mh < MAX_QT_MOVIES && g_movies[mh].in_use) {
        QtMovie *m = &g_movies[mh];
        if (m->pcm_data && m->pcm_size > 0 && !c->audio_started) {
            WAVEFORMATEX wfx;
            memset(&wfx, 0, sizeof(wfx));
            wfx.wFormatTag      = WAVE_FORMAT_PCM;
            wfx.nChannels       = m->audio_channels;
            wfx.nSamplesPerSec  = m->audio_rate;
            wfx.wBitsPerSample  = m->audio_bits;
            wfx.nBlockAlign     = (uint16_t)((wfx.nChannels * wfx.wBitsPerSample) / 8);
            wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * wfx.nBlockAlign;
            if (waveOutOpen(&c->hwo, WAVE_MAPPER, &wfx, 0, 0, CALLBACK_NULL) == MMSYSERR_NOERROR) {
                memset(&c->whdr, 0, sizeof(WAVEHDR));
                c->whdr.lpData         = (LPSTR)m->pcm_data;
                c->whdr.dwBufferLength = m->pcm_size;
                waveOutPrepareHeader(c->hwo, &c->whdr, sizeof(WAVEHDR));
                waveOutWrite(c->hwo, &c->whdr, sizeof(WAVEHDR));
                c->audio_started = true;
            }
        }
    }
}

void qt_seek(uint32_t ch, uint32_t time_val) {
    if (ch == 0 || ch >= MAX_QT_CTRLS || !g_ctrls[ch].in_use) return;
    QtController *c = &g_ctrls[ch];
    uint32_t ts = qt_get_timescale(c->movie_handle);
    c->base_ms = ts ? (uint32_t)(((uint64_t)time_val * 1000ULL) / ts) : 0;
    c->start_tick = timeGetTime();
    if (c->movie_handle > 0 && c->movie_handle < MAX_QT_MOVIES) {
        g_movies[c->movie_handle].cur_frame = -1;
    }
}

bool qt_is_playing(uint32_t ch) {
    if (ch == 0 || ch >= MAX_QT_CTRLS || !g_ctrls[ch].in_use) return false;
    return g_ctrls[ch].playing;
}

static void blit_movie_to_fb(const QtMovie *m, const QtRect *rc, uint32_t *fb, int fb_w, int fb_h) {
    if (!m->rgb_buf || !fb || m->vid_w <= 0 || m->vid_h <= 0) return;
    int dst_x = rc->left;
    int dst_y = rc->top;
    int dst_w = rc->right - rc->left;
    int dst_h = rc->bottom - rc->top;
    if (dst_w <= 0 || dst_h <= 0) {
        dst_w = m->vid_w; dst_h = m->vid_h;
        dst_x = (fb_w - dst_w) / 2;
        dst_y = (fb_h - dst_h) / 2;
    }
    for (int dy = 0; dy < dst_h; ++dy) {
        int fy = dst_y + dy;
        if (fy < 0 || fy >= fb_h) continue;
        int sy = (dy * m->vid_h) / dst_h;
        for (int dx = 0; dx < dst_w; ++dx) {
            int fx = dst_x + dx;
            if (fx < 0 || fx >= fb_w) continue;
            int sx = (dx * m->vid_w) / dst_w;
            fb[fy * fb_w + fx] = m->rgb_buf[sy * m->vid_w + sx];
        }
    }
}

bool qt_paint(uint32_t ch, uint32_t *fb, int fb_w, int fb_h) {
    if (ch == 0 || ch >= MAX_QT_CTRLS || !g_ctrls[ch].in_use) return false;
    uint32_t mh = g_ctrls[ch].movie_handle;
    if (mh == 0 || mh >= MAX_QT_MOVIES || !g_movies[mh].in_use) return false;
    blit_movie_to_fb(&g_movies[mh], &g_ctrls[ch].rc, fb, fb_w, fb_h);
    return true;
}

bool qt_tick(uint32_t ch, uint32_t *fb, int fb_w, int fb_h) {
    if (ch == 0 || ch >= MAX_QT_CTRLS || !g_ctrls[ch].in_use) return false;
    QtController *c = &g_ctrls[ch];
    if (!c->playing) return false;
    uint32_t mh = c->movie_handle;
    if (mh == 0 || mh >= MAX_QT_MOVIES || !g_movies[mh].in_use) {
        c->playing = false;
        return false;
    }
    QtMovie *m = &g_movies[mh];
    uint32_t elapsed = c->base_ms + (uint32_t)(timeGetTime() - c->start_tick);

    /* Find target frame for elapsed ms */
    int target = m->cur_frame;
    while (target + 1 < (int)m->vid_count && m->vid_samples[target + 1].pts_ms <= elapsed) {
        target++;
    }

    bool updated = false;
    if (target > m->cur_frame) {
        /* Decode sequentially from cur_frame+1 to target (inter-frame Cinepak) */
        for (int f = m->cur_frame + 1; f <= target; ++f) {
            uint32_t fsz = m->vid_samples[f].size;
            if (fsz > 0 && fsz <= 2 * 1024 * 1024) {
                uint8_t *fbuf = (uint8_t *)malloc(fsz);
                if (fbuf) {
                    vfs_llseek(m->vfs_handle, (int32_t)m->vid_samples[f].offset, 0);
                    if (vfs_hread(m->vfs_handle, fbuf, fsz) == (int32_t)fsz) {
                        cv_decode_frame(fbuf, fsz, m->rgb_buf, m->vid_w, m->vid_h, m->strips);
                    }
                    free(fbuf);
                }
            }
        }
        m->cur_frame = target;
        blit_movie_to_fb(m, &c->rc, fb, fb_w, fb_h);
        updated = true;
    }

    /* Check natural end of movie */
    if (elapsed >= m->duration_ms && (m->cur_frame + 1 >= (int)m->vid_count)) {
        if (!c->audio_started || (c->whdr.dwFlags & WHDR_DONE) || elapsed > m->duration_ms + 500) {
            c->playing = false;
            ctrl_stop_audio(c);
        }
    }
    return updated;
}

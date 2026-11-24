#define _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "iso_extract.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wctype.h>

static uint32_t rd_u32le(const uint8_t *p) {
    return (uint32_t)p[0] | ((uint32_t)p[1] << 8) | ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
}

static bool wcs_ieq(const wchar_t *a, const wchar_t *b) {
    return _wcsicmp(a, b) == 0;
}

static void make_dirs_for_path(const wchar_t *path, bool is_file) {
    wchar_t tmp[1024];
    wcsncpy(tmp, path, 1023);
    tmp[1023] = L'\0';
    for (wchar_t *p = tmp; *p; ++p) {
        if (*p == L'/') *p = L'\\';
    }
    if (is_file) {
        wchar_t *slash = wcsrchr(tmp, L'\\');
        if (!slash) return;
        *slash = L'\0';
    }
    for (wchar_t *p = tmp + 3; *p; ++p) {
        if (*p == L'\\') {
            *p = L'\0';
            CreateDirectoryW(tmp, NULL);
            *p = L'\\';
        }
    }
    CreateDirectoryW(tmp, NULL);
}

static bool get_file_size_w(const wchar_t *path, uint64_t *out_sz) {
    WIN32_FILE_ATTRIBUTE_DATA fad;
    if (!GetFileAttributesExW(path, GetFileExInfoStandard, &fad)) return false;
    *out_sz = ((uint64_t)fad.nFileSizeHigh << 32) | (uint64_t)fad.nFileSizeLow;
    return true;
}

static bool has_ext_i(const wchar_t *path, const wchar_t *ext) {
    const wchar_t *dot = wcsrchr(path, L'.');
    return dot && _wcsicmp(dot, ext) == 0;
}

static void dir_of_path(const wchar_t *path, wchar_t *out_dir, size_t cap) {
    wcsncpy(out_dir, path, cap - 1);
    out_dir[cap - 1] = L'\0';
    wchar_t *s1 = wcsrchr(out_dir, L'\\');
    wchar_t *s2 = wcsrchr(out_dir, L'/');
    wchar_t *s = s1 > s2 ? s1 : s2;
    if (s) *(s + 1) = L'\0';
    else out_dir[0] = L'\0';
}

static bool detect_iso_geometry(const wchar_t *path, uint64_t base_off,
                                uint32_t *out_sec_sz, uint32_t *out_data_off) {
    FILE *f = _wfopen(path, L"rb");
    if (!f) return false;

    const struct { uint32_t sec; uint32_t off; } tries[] = {
        { 2048, 0 },
        { 2352, 16 }, /* MODE1/2352 */
        { 2352, 24 }, /* MODE2/2352 Form 1 */
        { 2336, 8  }, /* MODE2/2336 */
    };
    uint8_t sig[6];
    for (int i = 0; i < 4; ++i) {
        uint64_t pos = base_off + (uint64_t)16 * tries[i].sec + tries[i].off + 1;
        if (_fseeki64(f, (long long)pos, SEEK_SET) == 0 && fread(sig, 1, 5, f) == 5) {
            if (memcmp(sig, "CD001", 5) == 0) {
                *out_sec_sz = tries[i].sec;
                *out_data_off = tries[i].off;
                fclose(f);
                return true;
            }
        }
    }
    fclose(f);
    return false;
}

static bool parse_msf(const char *s, int *mm, int *ss, int *ff) {
    return sscanf(s, "%d:%d:%d", mm, ss, ff) == 3;
}

static uint64_t msf_to_frames(int mm, int ss, int ff) {
    return ((uint64_t)mm * 60 + (uint64_t)ss) * 75 + (uint64_t)ff;
}

static void frames_to_msf(uint64_t frames, int *mm, int *ss, int *ff) {
    *ff = (int)(frames % 75);
    frames /= 75;
    *ss = (int)(frames % 60);
    *mm = (int)(frames / 60);
}

static bool parse_cue_file(const wchar_t *cue_path, CdImageInfo *info, wchar_t *err_buf, size_t err_cap) {
    FILE *f = _wfopen(cue_path, L"rb");
    if (!f) {
        swprintf(err_buf, err_cap, L"Não foi possível abrir o arquivo CUE.");
        return false;
    }

    wchar_t cue_dir[1024];
    dir_of_path(cue_path, cue_dir, 1024);

    typedef struct {
        int track_num;
        bool is_audio;
        bool is_wave;
        uint32_t sec_size;
        uint32_t data_off;
        wchar_t file_path[1024];
        bool has_index01;
        int idx01_mm, idx01_ss, idx01_ff;
        bool has_index00;
        int idx00_mm, idx00_ss, idx00_ff;
    } CueTrack;

    CueTrack tracks[64];
    int ntracks = 0;
    wchar_t cur_file[1024] = {0};
    bool cur_is_wave = false;
    int cur_track = -1;

    char line[2048];
    while (fgets(line, sizeof(line), f)) {
        char *p = line;
        while (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n') ++p;
        if (*p == '\0') continue;

        if (_strnicmp(p, "FILE ", 5) == 0) {
            p += 5;
            while (*p == ' ' || *p == '\t') ++p;
            char fname[1024] = {0};
            if (*p == '"') {
                ++p;
                char *end = strchr(p, '"');
                if (end) {
                    size_t l = (size_t)(end - p);
                    if (l >= sizeof(fname)) l = sizeof(fname) - 1;
                    memcpy(fname, p, l);
                    fname[l] = '\0';
                    p = end + 1;
                }
            } else {
                sscanf(p, "%1023s", fname);
            }
            cur_is_wave = (strstr(p, "WAVE") != NULL || strstr(p, "wave") != NULL);

            wchar_t wfname[1024];
            if (!MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, fname, -1, wfname, 1024)) {
                MultiByteToWideChar(CP_ACP, 0, fname, -1, wfname, 1024);
            }
            if ((wfname[0] && wfname[1] == L':') || (wfname[0] == L'\\' && wfname[1] == L'\\')) {
                wcsncpy(cur_file, wfname, 1023);
            } else {
                swprintf(cur_file, 1024, L"%s%s", cue_dir, wfname);
            }
        } else if (_strnicmp(p, "TRACK ", 6) == 0) {
            int num = 0;
            char mode[64] = {0};
            if (sscanf(p + 6, "%d %63s", &num, mode) == 2 && ntracks < 64) {
                cur_track = ntracks++;
                memset(&tracks[cur_track], 0, sizeof(CueTrack));
                tracks[cur_track].track_num = num;
                wcsncpy(tracks[cur_track].file_path, cur_file, 1023);
                tracks[cur_track].is_wave = cur_is_wave;
                if (_stricmp(mode, "AUDIO") == 0) {
                    tracks[cur_track].is_audio = true;
                    tracks[cur_track].sec_size = 2352;
                } else if (_strnicmp(mode, "MODE1/2352", 10) == 0) {
                    tracks[cur_track].sec_size = 2352;
                    tracks[cur_track].data_off = 16;
                } else if (_strnicmp(mode, "MODE2/2352", 10) == 0) {
                    tracks[cur_track].sec_size = 2352;
                    tracks[cur_track].data_off = 24;
                } else if (_strnicmp(mode, "MODE1/2048", 10) == 0) {
                    tracks[cur_track].sec_size = 2048;
                    tracks[cur_track].data_off = 0;
                } else {
                    tracks[cur_track].sec_size = 2352;
                    tracks[cur_track].data_off = 16;
                }
            }
        } else if (_strnicmp(p, "INDEX ", 6) == 0 && cur_track >= 0) {
            int idx = -1;
            char msf[32] = {0};
            if (sscanf(p + 6, "%d %31s", &idx, msf) == 2) {
                int mm, ss, ff;
                if (parse_msf(msf, &mm, &ss, &ff)) {
                    if (idx == 1) {
                        tracks[cur_track].has_index01 = true;
                        tracks[cur_track].idx01_mm = mm;
                        tracks[cur_track].idx01_ss = ss;
                        tracks[cur_track].idx01_ff = ff;
                    } else if (idx == 0) {
                        tracks[cur_track].has_index00 = true;
                        tracks[cur_track].idx00_mm = mm;
                        tracks[cur_track].idx00_ss = ss;
                        tracks[cur_track].idx00_ff = ff;
                    }
                }
            }
        }
    }
    fclose(f);

    if (ntracks == 0) {
        swprintf(err_buf, err_cap, L"O arquivo CUE não contém faixas válidas.");
        return false;
    }

    int data_idx = -1;
    for (int i = 0; i < ntracks; ++i) {
        if (!tracks[i].is_audio) { data_idx = i; break; }
    }
    if (data_idx < 0) {
        swprintf(err_buf, err_cap, L"O arquivo CUE não possui faixa de dados (Track 01).");
        return false;
    }

    wcsncpy(info->data_file, tracks[data_idx].file_path, 1023);
    info->track1_byte_offset = tracks[data_idx].has_index01
        ? msf_to_frames(tracks[data_idx].idx01_mm, tracks[data_idx].idx01_ss, tracks[data_idx].idx01_ff) * tracks[data_idx].sec_size
        : 0;

    if (!detect_iso_geometry(info->data_file, info->track1_byte_offset, &info->sector_size, &info->data_offset)) {
        swprintf(err_buf, err_cap, L"Não foi possível encontrar o arquivo BIN/ISO referenciado no CUE:\n%s", info->data_file);
        return false;
    }

    /* Compute data track length (in MSF) for music.cue REM DATATRACK 01 */
    uint64_t data_frames = 0;
    if (data_idx + 1 < ntracks && wcs_ieq(tracks[data_idx].file_path, tracks[data_idx + 1].file_path)) {
        if (tracks[data_idx + 1].has_index01) {
            data_frames = msf_to_frames(tracks[data_idx + 1].idx01_mm, tracks[data_idx + 1].idx01_ss, tracks[data_idx + 1].idx01_ff);
        }
    } else {
        uint64_t fsz = 0;
        if (get_file_size_w(tracks[data_idx].file_path, &fsz) && info->sector_size > 0) {
            data_frames = fsz / info->sector_size;
        }
    }
    frames_to_msf(data_frames, &info->data_track_mm, &info->data_track_ss, &info->data_track_ff);

    /* Collect audio tracks */
    info->audio_track_count = 0;
    for (int i = 0; i < ntracks && info->audio_track_count < 32; ++i) {
        if (!tracks[i].is_audio) continue;
        uint64_t fsz = 0;
        if (!get_file_size_w(tracks[i].file_path, &fsz)) continue;

        uint64_t start_b = 0;
        if (!tracks[i].is_wave && tracks[i].has_index01) {
            start_b = msf_to_frames(tracks[i].idx01_mm, tracks[i].idx01_ss, tracks[i].idx01_ff) * 2352ULL;
        }
        uint64_t end_b = fsz;
        if (!tracks[i].is_wave && i + 1 < ntracks && wcs_ieq(tracks[i].file_path, tracks[i + 1].file_path)) {
            if (tracks[i + 1].has_index00) {
                end_b = msf_to_frames(tracks[i + 1].idx00_mm, tracks[i + 1].idx00_ss, tracks[i + 1].idx00_ff) * 2352ULL;
            } else if (tracks[i + 1].has_index01) {
                end_b = msf_to_frames(tracks[i + 1].idx01_mm, tracks[i + 1].idx01_ss, tracks[i + 1].idx01_ff) * 2352ULL;
            }
        }
        if (end_b <= start_b) continue;

        int k = info->audio_track_count++;
        info->audio_tracks[k].track_num = tracks[i].track_num;
        wcsncpy(info->audio_tracks[k].file_path, tracks[i].file_path, 1023);
        info->audio_tracks[k].is_wave = tracks[i].is_wave;
        info->audio_tracks[k].start_byte = start_b;
        info->audio_tracks[k].length_bytes = end_b - start_b;
        info->audio_tracks[k].index01_mm = tracks[i].idx01_mm;
        info->audio_tracks[k].index01_ss = tracks[i].idx01_ss;
        info->audio_tracks[k].index01_ff = tracks[i].idx01_ff;
    }
    info->has_audio_tracks = (info->audio_track_count > 0);
    return true;
}

bool cdimg_open(const wchar_t *selected_path, CdImageInfo *out_info, wchar_t *err_buf, size_t err_cap) {
    memset(out_info, 0, sizeof(*out_info));

    if (has_ext_i(selected_path, L".cue")) {
        return parse_cue_file(selected_path, out_info, err_buf, err_cap);
    }

    /* If user picked a .bin, check if a sibling .cue exists so we automatically grab CD audio too! */
    if (has_ext_i(selected_path, L".bin")) {
        wchar_t cue_try[1024];
        wcsncpy(cue_try, selected_path, 1023);
        cue_try[1023] = L'\0';
        wchar_t *dot = wcsrchr(cue_try, L'.');
        if (dot) {
            wcscpy(dot, L".cue");
            if (GetFileAttributesW(cue_try) != INVALID_FILE_ATTRIBUTES) {
                wchar_t dummy_err[256];
                if (parse_cue_file(cue_try, out_info, dummy_err, 256)) {
                    return true;
                }
                memset(out_info, 0, sizeof(*out_info));
            }
        }
    }

    wcsncpy(out_info->data_file, selected_path, 1023);
    out_info->track1_byte_offset = 0;
    if (!detect_iso_geometry(out_info->data_file, 0, &out_info->sector_size, &out_info->data_offset)) {
        swprintf(err_buf, err_cap, L"O arquivo selecionado não é uma imagem ISO 9660/CUE válida.");
        return false;
    }
    return true;
}

/* ---- ISO 9660 / Joliet Reader ---- */

typedef struct {
    FILE *fp;
    const CdImageInfo *info;
    bool joliet;
    uint32_t root_lba;
    uint32_t root_size;
} IsoReader;

static bool iso_read_sector(IsoReader *r, uint32_t lba, uint8_t out2048[2048]) {
    uint64_t pos = r->info->track1_byte_offset + (uint64_t)lba * r->info->sector_size + r->info->data_offset;
    if (_fseeki64(r->fp, (long long)pos, SEEK_SET) != 0) return false;
    return fread(out2048, 1, 2048, r->fp) == 2048;
}

static bool iso_reader_init(IsoReader *r, const CdImageInfo *info) {
    memset(r, 0, sizeof(*r));
    r->info = info;
    r->fp = _wfopen(info->data_file, L"rb");
    if (!r->fp) return false;

    uint8_t sec[2048];
    bool has_pvd = false, has_svd = false;
    uint32_t pvd_lba = 0, pvd_sz = 0, svd_lba = 0, svd_sz = 0;

    for (uint32_t s = 16; s < 64; ++s) {
        if (!iso_read_sector(r, s, sec)) break;
        if (memcmp(sec + 1, "CD001", 5) != 0) break;
        uint8_t type = sec[0];
        if (type == 255) break;
        if (type == 1 && !has_pvd) {
            pvd_lba = rd_u32le(sec + 156 + 2);
            pvd_sz  = rd_u32le(sec + 156 + 10);
            has_pvd = true;
        } else if (type == 2 && !has_svd) {
            /* Joliet escape sequences at offset 88: 25 2F 40, 25 2F 43, or 25 2F 45 */
            if (sec[88] == 0x25 && sec[89] == 0x2F &&
                (sec[90] == 0x40 || sec[90] == 0x43 || sec[90] == 0x45)) {
                svd_lba = rd_u32le(sec + 156 + 2);
                svd_sz  = rd_u32le(sec + 156 + 10);
                has_svd = true;
            }
        }
    }

    if (has_svd) {
        r->joliet = true;
        r->root_lba = svd_lba;
        r->root_size = svd_sz;
        return true;
    }
    if (has_pvd) {
        r->joliet = false;
        r->root_lba = pvd_lba;
        r->root_size = pvd_sz;
        return true;
    }
    fclose(r->fp);
    r->fp = NULL;
    return false;
}

static void iso_reader_close(IsoReader *r) {
    if (r->fp) { fclose(r->fp); r->fp = NULL; }
}

static void decode_iso_name(const uint8_t *raw, uint8_t len, bool joliet, wchar_t *out, size_t cap) {
    size_t n = 0;
    if (joliet) {
        for (uint8_t i = 0; i + 1 < len && n + 1 < cap; i += 2) {
            wchar_t ch = (wchar_t)(((uint16_t)raw[i] << 8) | raw[i + 1]);
            out[n++] = ch;
        }
    } else {
        for (uint8_t i = 0; i < len && n + 1 < cap; ++i) {
            out[n++] = (wchar_t)raw[i];
        }
    }
    out[n] = L'\0';
    /* Strip ISO 9660 ";1" version suffix */
    wchar_t *semi = wcschr(out, L';');
    if (semi) *semi = L'\0';
    /* Strip trailing dot if bare directory/file had no extension */
    size_t sl = wcslen(out);
    if (sl > 0 && out[sl - 1] == L'.') out[sl - 1] = L'\0';
}

typedef struct {
    wchar_t rel_path[512];
    wchar_t name[260];
    uint32_t lba;
    uint32_t size;
    bool is_dir;
} IsoEntry;

typedef struct {
    IsoEntry *items;
    size_t count;
    size_t cap;
    uint64_t total_file_bytes;
} IsoList;

static void iso_list_push(IsoList *list, const IsoEntry *e) {
    if (list->count == list->cap) {
        size_t ncap = list->cap ? list->cap * 2 : 256;
        list->items = (IsoEntry *)realloc(list->items, ncap * sizeof(IsoEntry));
        list->cap = ncap;
    }
    list->items[list->count++] = *e;
    if (!e->is_dir) list->total_file_bytes += e->size;
}

static bool iso_walk_dir(IsoReader *r, uint32_t dir_lba, uint32_t dir_size,
                         const wchar_t *prefix, IsoList *out_list, int depth) {
    if (depth > 16 || dir_size == 0 || dir_size > 16 * 1024 * 1024) return true;
    uint32_t nsec = (dir_size + 2047) / 2048;
    uint8_t *buf = (uint8_t *)malloc((size_t)nsec * 2048);
    if (!buf) return false;

    for (uint32_t i = 0; i < nsec; ++i) {
        if (!iso_read_sector(r, dir_lba + i, buf + (size_t)i * 2048)) {
            free(buf);
            return false;
        }
    }

    uint32_t pos = 0;
    while (pos < dir_size) {
        uint8_t rec_len = buf[pos];
        if (rec_len == 0) {
            pos = ((pos / 2048) + 1) * 2048;
            continue;
        }
        if (pos + rec_len > nsec * 2048 || rec_len < 34) break;

        uint32_t ext_lba = rd_u32le(buf + pos + 2);
        uint32_t ext_sz  = rd_u32le(buf + pos + 10);
        uint8_t  flags   = buf[pos + 25];
        uint8_t  name_ln = buf[pos + 32];
        const uint8_t *name_raw = buf + pos + 33;

        if (!(name_ln == 1 && (name_raw[0] == 0x00 || name_raw[0] == 0x01))) {
            IsoEntry e;
            memset(&e, 0, sizeof(e));
            decode_iso_name(name_raw, name_ln, r->joliet, e.name, 260);
            if (e.name[0] != L'\0') {
                if (prefix[0]) swprintf(e.rel_path, 512, L"%s\\%s", prefix, e.name);
                else wcsncpy(e.rel_path, e.name, 511);
                e.lba = ext_lba;
                e.size = ext_sz;
                e.is_dir = (flags & 0x02) != 0;
                iso_list_push(out_list, &e);
                if (e.is_dir) {
                    iso_walk_dir(r, e.lba, e.size, e.rel_path, out_list, depth + 1);
                }
            }
        }
        pos += rec_len;
    }

    free(buf);
    return true;
}

static bool iso_extract_entry(IsoReader *r, const IsoEntry *e, const wchar_t *dest_path,
                              uint64_t *done_bytes, uint64_t total_bytes,
                              IsoProgressFn progress, void *progress_user) {
    make_dirs_for_path(dest_path, true);
    FILE *out = _wfopen(dest_path, L"wb");
    if (!out) return false;

    uint8_t sec[2048];
    uint32_t remaining = e->size;
    uint32_t lba = e->lba;
    uint64_t last_report = *done_bytes;

    while (remaining > 0) {
        if (!iso_read_sector(r, lba++, sec)) {
            fclose(out);
            return false;
        }
        uint32_t chunk = remaining < 2048 ? remaining : 2048;
        if (fwrite(sec, 1, chunk, out) != chunk) {
            fclose(out);
            return false;
        }
        remaining -= chunk;
        *done_bytes += chunk;
        if (progress && (*done_bytes - last_report >= 262144 || remaining == 0)) {
            last_report = *done_bytes;
            progress(*done_bytes, total_bytes, e->rel_path, progress_user);
        }
    }
    fclose(out);
    return true;
}

bool cdimg_file_exists(const CdImageInfo *info, const wchar_t *rel_path) {
    IsoReader r;
    if (!iso_reader_init(&r, info)) return false;
    IsoList list = {0};
    iso_walk_dir(&r, r.root_lba, r.root_size, L"", &list, 0);
    bool found = false;
    for (size_t i = 0; i < list.count; ++i) {
        if (!list.items[i].is_dir && wcs_ieq(list.items[i].rel_path, rel_path)) {
            found = true;
            break;
        }
    }
    free(list.items);
    iso_reader_close(&r);
    return found;
}

bool cdimg_extract_single(const CdImageInfo *info, const wchar_t *rel_path, const wchar_t *dest_path,
                          wchar_t *err_buf, size_t err_cap) {
    IsoReader r;
    if (!iso_reader_init(&r, info)) {
        swprintf(err_buf, err_cap, L"Erro ao ler o sistema de arquivos ISO 9660.");
        return false;
    }
    IsoList list = {0};
    iso_walk_dir(&r, r.root_lba, r.root_size, L"", &list, 0);
    bool ok = false;
    for (size_t i = 0; i < list.count; ++i) {
        if (!list.items[i].is_dir && wcs_ieq(list.items[i].rel_path, rel_path)) {
            uint64_t done = 0;
            ok = iso_extract_entry(&r, &list.items[i], dest_path, &done, list.items[i].size, NULL, NULL);
            if (!ok) swprintf(err_buf, err_cap, L"Erro ao extrair %s da imagem.", rel_path);
            break;
        }
    }
    if (!ok && err_buf[0] == L'\0') {
        swprintf(err_buf, err_cap, L"Arquivo não encontrado na imagem do CD: %s", rel_path);
    }
    free(list.items);
    iso_reader_close(&r);
    return ok;
}

bool cdimg_extract_tree(const CdImageInfo *info, const wchar_t *dest_dir,
                        IsoFilterFn filter, void *filter_user,
                        const wchar_t *flatten_subdir,
                        IsoProgressFn progress, void *progress_user,
                        wchar_t *err_buf, size_t err_cap) {
    IsoReader r;
    if (!iso_reader_init(&r, info)) {
        swprintf(err_buf, err_cap, L"Erro ao abrir o sistema de arquivos ISO 9660 da imagem.");
        return false;
    }

    IsoList list = {0};
    if (!iso_walk_dir(&r, r.root_lba, r.root_size, L"", &list, 0)) {
        free(list.items);
        iso_reader_close(&r);
        swprintf(err_buf, err_cap, L"Erro ao listar diretórios da imagem do CD.");
        return false;
    }

    /* Calculate exact filtered byte total */
    uint64_t total_bytes = 0;
    for (size_t i = 0; i < list.count; ++i) {
        const IsoEntry *e = &list.items[i];
        if (filter && !filter(e->rel_path, e->is_dir, filter_user)) continue;
        if (!e->is_dir) total_bytes += e->size;
    }

    uint64_t done_bytes = 0;
    size_t flat_len = flatten_subdir ? wcslen(flatten_subdir) : 0;

    for (size_t i = 0; i < list.count; ++i) {
        const IsoEntry *e = &list.items[i];
        if (filter && !filter(e->rel_path, e->is_dir, filter_user)) continue;
        if (e->is_dir) continue;

        const wchar_t *out_rel = e->rel_path;
        if (flatten_subdir && _wcsnicmp(e->rel_path, flatten_subdir, flat_len) == 0 &&
            e->rel_path[flat_len] == L'\\') {
            out_rel = e->rel_path + flat_len + 1;
        }

        wchar_t dest_path[1024];
        swprintf(dest_path, 1024, L"%s\\%s", dest_dir, out_rel);
        if (!iso_extract_entry(&r, e, dest_path, &done_bytes, total_bytes, progress, progress_user)) {
            swprintf(err_buf, err_cap, L"Falha ao gravar arquivo extraído:\n%s", dest_path);
            free(list.items);
            iso_reader_close(&r);
            return false;
        }
    }

    free(list.items);
    iso_reader_close(&r);
    return true;
}

static void write_wav_header(FILE *out, uint32_t pcm_bytes) {
    uint8_t h[44];
    uint32_t riff_sz = 36 + pcm_bytes;
    uint32_t sample_rate = 44100;
    uint32_t byte_rate = 44100 * 4;
    memcpy(h + 0,  "RIFF", 4);
    memcpy(h + 4,  &riff_sz, 4);
    memcpy(h + 8,  "WAVE", 4);
    memcpy(h + 12, "fmt ", 4);
    uint32_t fmt_sz = 16; memcpy(h + 16, &fmt_sz, 4);
    uint16_t pcm = 1;     memcpy(h + 20, &pcm, 2);
    uint16_t ch = 2;      memcpy(h + 22, &ch, 2);
    memcpy(h + 24, &sample_rate, 4);
    memcpy(h + 28, &byte_rate, 4);
    uint16_t align = 4;   memcpy(h + 32, &align, 2);
    uint16_t bps = 16;    memcpy(h + 34, &bps, 2);
    memcpy(h + 36, "data", 4);
    memcpy(h + 40, &pcm_bytes, 4);
    fwrite(h, 1, 44, out);
}

bool cdimg_extract_audio(const CdImageInfo *info, const wchar_t *dest_dir,
                         IsoProgressFn progress, void *progress_user,
                         wchar_t *err_buf, size_t err_cap) {
    if (!info->has_audio_tracks) return true;

    wchar_t music_dir[1024];
    swprintf(music_dir, 1024, L"%s\\music", dest_dir);
    make_dirs_for_path(music_dir, false);

    wchar_t cue_out_path[1024];
    swprintf(cue_out_path, 1024, L"%s\\music.cue", music_dir);
    FILE *cue_out = _wfopen(cue_out_path, L"wb");
    if (!cue_out) {
        swprintf(err_buf, err_cap, L"Não foi possível criar music\\music.cue.");
        return false;
    }

    fprintf(cue_out, "REM Generated automatically - preserves original disc audio layout\r\n");
    fprintf(cue_out, "REM DATATRACK 01 %02d:%02d:%02d\r\n",
            info->data_track_mm, info->data_track_ss, info->data_track_ff);

    uint64_t total_bytes = 0;
    for (int i = 0; i < info->audio_track_count; ++i) total_bytes += info->audio_tracks[i].length_bytes;
    uint64_t done_bytes = 0;

    uint8_t *buf = (uint8_t *)malloc(262144);
    if (!buf) { fclose(cue_out); return false; }

    for (int i = 0; i < info->audio_track_count; ++i) {
        int tnum = info->audio_tracks[i].track_num;
        wchar_t wav_path[1024];
        swprintf(wav_path, 1024, L"%s\\track%02d.wav", music_dir, tnum);

        FILE *in = _wfopen(info->audio_tracks[i].file_path, L"rb");
        if (!in) {
            free(buf); fclose(cue_out);
            swprintf(err_buf, err_cap, L"Não foi possível abrir faixa de áudio %d.", tnum);
            return false;
        }
        FILE *out = _wfopen(wav_path, L"wb");
        if (!out) {
            fclose(in); free(buf); fclose(cue_out);
            swprintf(err_buf, err_cap, L"Não foi possível gravar track%02d.wav.", tnum);
            return false;
        }

        _fseeki64(in, (long long)info->audio_tracks[i].start_byte, SEEK_SET);
        if (!info->audio_tracks[i].is_wave) {
            write_wav_header(out, (uint32_t)info->audio_tracks[i].length_bytes);
        }

        uint64_t rem = info->audio_tracks[i].length_bytes;
        wchar_t label[128];
        swprintf(label, 128, L"Extraindo música do CD (faixa %02d)...", tnum);
        while (rem > 0) {
            size_t chunk = rem > 262144 ? 262144 : (size_t)rem;
            size_t got = fread(buf, 1, chunk, in);
            if (got == 0) break;
            fwrite(buf, 1, got, out);
            rem -= got;
            done_bytes += got;
            if (progress) progress(done_bytes, total_bytes, label, progress_user);
        }
        fclose(in);
        fclose(out);

        fprintf(cue_out, "FILE \"track%02d.wav\" WAVE\r\n", tnum);
        fprintf(cue_out, "  TRACK %02d AUDIO\r\n", tnum);
        fprintf(cue_out, "    INDEX 01 00:00:00\r\n");
    }

    free(buf);
    fclose(cue_out);
    return true;
}

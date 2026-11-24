#define _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "is3_extract.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ---- PKWARE Data Compression Library (Implode/Explode) decompressor ---- */

#define MAXBITS 13
#define MAXWIN  4096

typedef struct {
    const uint8_t *in;
    size_t in_len;
    size_t in_pos;
    int bitbuf;
    int bitcnt;
    uint8_t window[MAXWIN];
    int win_pos;
    int first_fill;
    FILE *out_fp;
    uint64_t written;
} ExplodeState;

typedef struct {
    short count[MAXBITS + 1];
    short symbol[256];
} HuffTable;

static int get_bits(ExplodeState *s, int need) {
    int val = s->bitbuf;
    while (s->bitcnt < need) {
        if (s->in_pos >= s->in_len) return -1;
        val |= (int)(s->in[s->in_pos++]) << s->bitcnt;
        s->bitcnt += 8;
    }
    s->bitbuf = val >> need;
    s->bitcnt -= need;
    return val & ((1 << need) - 1);
}

static int build_huff(HuffTable *h, const uint8_t *rep, int nrep) {
    short length[256];
    int sym = 0;
    for (int i = 0; i < nrep; ++i) {
        int len = rep[i] & 0x0F;
        int cnt = (rep[i] >> 4) + 1;
        while (cnt-- > 0 && sym < 256) {
            length[sym++] = (short)len;
        }
    }
    int nsym = sym;

    for (int len = 0; len <= MAXBITS; ++len) h->count[len] = 0;
    for (int i = 0; i < nsym; ++i) h->count[length[i]]++;

    short offs[MAXBITS + 1];
    offs[1] = 0;
    for (int len = 1; len < MAXBITS; ++len) {
        offs[len + 1] = offs[len] + h->count[len];
    }

    for (int i = 0; i < nsym; ++i) {
        if (length[i] != 0) {
            h->symbol[offs[length[i]]++] = (short)i;
        }
    }
    return nsym;
}

static int decode_huff(ExplodeState *s, const HuffTable *h) {
    int code = 0, first = 0, index = 0;
    for (int len = 1; len <= MAXBITS; ++len) {
        int b = get_bits(s, 1);
        if (b < 0) return -1;
        code |= (b ^ 1);
        int count = h->count[len];
        if (code - count < first) {
            return h->symbol[index + (code - first)];
        }
        index += count;
        first += count;
        first <<= 1;
        code <<= 1;
    }
    return -1;
}

static bool flush_window(ExplodeState *s, int len) {
    if (len <= 0) return true;
    if (fwrite(s->window, 1, (size_t)len, s->out_fp) != (size_t)len) return false;
    s->written += (uint64_t)len;
    return true;
}

static bool pkware_explode(const uint8_t *comp, size_t comp_len, FILE *out_fp, uint64_t *out_written) {
    static bool tables_init = false;
    static HuffTable lit_code, len_code, dist_code;
    static const uint8_t lit_len_rle[] = {
        11, 124, 8, 7, 28, 7, 188, 13, 76, 4, 10, 8, 12, 10, 12, 10, 8, 23, 8,
        9, 7, 6, 7, 8, 7, 6, 55, 8, 23, 24, 12, 11, 7, 9, 11, 12, 6, 7, 22, 5,
        7, 24, 6, 11, 9, 6, 7, 22, 7, 11, 38, 7, 9, 8, 25, 11, 8, 11, 9, 12,
        8, 12, 5, 38, 5, 38, 5, 11, 7, 5, 6, 21, 6, 10, 53, 8, 7, 24, 10, 27,
        44, 253, 253, 253, 252, 252, 252, 13, 12, 45, 12, 45, 12, 61, 12, 45,
        44, 173
    };
    static const uint8_t len_len_rle[] = { 2, 35, 36, 53, 38, 23 };
    static const uint8_t dist_len_rle[] = { 2, 20, 53, 230, 247, 151, 248 };
    static const short len_base[16] = {
        3, 2, 4, 5, 6, 7, 8, 9, 10, 12, 16, 24, 40, 72, 136, 264
    };
    static const uint8_t len_extra[16] = {
        0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 5, 6, 7, 8
    };

    if (!tables_init) {
        build_huff(&lit_code, lit_len_rle, (int)sizeof(lit_len_rle));
        build_huff(&len_code, len_len_rle, (int)sizeof(len_len_rle));
        build_huff(&dist_code, dist_len_rle, (int)sizeof(dist_len_rle));
        tables_init = true;
    }

    if (comp_len < 2) return false;
    ExplodeState s;
    memset(&s, 0, sizeof(s));
    s.in = comp;
    s.in_len = comp_len;
    s.out_fp = out_fp;
    s.first_fill = 1;

    int lit = get_bits(&s, 8);
    int dict = get_bits(&s, 8);
    if ((lit != 0 && lit != 1) || dict < 4 || dict > 6) return false;

    for (;;) {
        int flag = get_bits(&s, 1);
        if (flag < 0) return false;

        if (flag == 1) {
            int sym = decode_huff(&s, &len_code);
            if (sym < 0) return false;
            int extra = get_bits(&s, len_extra[sym]);
            if (extra < 0) return false;
            int len = len_base[sym] + extra;
            if (len == 519) break; /* end of stream */

            int d_extra_bits = (len == 2) ? 2 : dict;
            int d_sym = decode_huff(&s, &dist_code);
            if (d_sym < 0) return false;
            int d_low = get_bits(&s, d_extra_bits);
            if (d_low < 0) return false;
            int dist = (d_sym << d_extra_bits) + d_low + 1;

            if (s.first_fill && dist > s.win_pos) return false;

            while (len > 0) {
                int from = (s.win_pos - dist) & (MAXWIN - 1);
                s.window[s.win_pos++] = s.window[from];
                if (s.win_pos == MAXWIN) {
                    if (!flush_window(&s, MAXWIN)) return false;
                    s.win_pos = 0;
                    s.first_fill = 0;
                }
                len--;
            }
        } else {
            int sym = lit ? decode_huff(&s, &lit_code) : get_bits(&s, 8);
            if (sym < 0) return false;
            s.window[s.win_pos++] = (uint8_t)sym;
            if (s.win_pos == MAXWIN) {
                if (!flush_window(&s, MAXWIN)) return false;
                s.win_pos = 0;
                s.first_fill = 0;
            }
        }
    }

    if (s.win_pos > 0) {
        if (!flush_window(&s, s.win_pos)) return false;
    }
    if (out_written) *out_written = s.written;
    return true;
}

/* ---- InstallShield 3.0 Archive Reader ---- */

static uint16_t rd_u16(const uint8_t *p) { return (uint16_t)p[0] | ((uint16_t)p[1] << 8); }
static uint32_t rd_u32(const uint8_t *p) {
    return (uint32_t)p[0] | ((uint32_t)p[1] << 8) | ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
}

static void make_parent_dirs(const wchar_t *file_path) {
    wchar_t tmp[1024];
    wcsncpy(tmp, file_path, 1023);
    tmp[1023] = L'\0';
    for (wchar_t *p = tmp; *p; ++p) if (*p == L'/') *p = L'\\';
    wchar_t *slash = wcsrchr(tmp, L'\\');
    if (!slash) return;
    *slash = L'\0';
    for (wchar_t *p = tmp + 3; *p; ++p) {
        if (*p == L'\\') {
            *p = L'\0';
            CreateDirectoryW(tmp, NULL);
            *p = L'\\';
        }
    }
    CreateDirectoryW(tmp, NULL);
}

bool is3_extract_archive(const wchar_t *archive_path, const wchar_t *dest_dir,
                         Is3ProgressFn progress, void *progress_user,
                         wchar_t *err_buf, size_t err_cap) {
    FILE *f = _wfopen(archive_path, L"rb");
    if (!f) {
        swprintf(err_buf, err_cap, L"N\u00e3o foi poss\u00edvel abrir o arquivo InstallShield:\n%s", archive_path);
        return false;
    }

    uint8_t sig_hdr[63];
    if (fread(sig_hdr, 1, 63, f) != 63) {
        fclose(f);
        swprintf(err_buf, err_cap, L"Arquivo ARCHIVE.Z truncado ou inv\u00e1lido.");
        return false;
    }

    uint32_t magic = rd_u32(sig_hdr);
    uint8_t hdr_sz = sig_hdr[4];
    if (magic != 0x8C655D13u || hdr_sz != 0x3Au) {
        fclose(f);
        swprintf(err_buf, err_cap, L"Assinatura InstallShield 3.0 inv\u00e1lida em ARCHIVE.Z.");
        return false;
    }

    const uint8_t *h = sig_hdr + 5;
    uint16_t num_files        = rd_u16(h + 7);
    uint32_t total_orig_size  = rd_u32(h + 17);
    uint32_t dirs_list_offset = rd_u32(h + 36);
    uint16_t num_dirs         = rd_u16(h + 44);
    uint32_t files_list_off   = rd_u32(h + 46);

    char **dirs = (char **)calloc(num_dirs ? num_dirs : 1, sizeof(char *));
    if (!dirs) { fclose(f); return false; }

    fseek(f, (long)dirs_list_offset, SEEK_SET);
    for (uint16_t i = 0; i < num_dirs; ++i) {
        uint8_t dh[6];
        if (fread(dh, 1, 6, f) != 6) break;
        uint16_t name_sz = rd_u16(dh + 4);
        dirs[i] = (char *)calloc((size_t)name_sz + 2, 1);
        if (fread(dirs[i], 1, (size_t)name_sz + 1, f) != (size_t)name_sz + 1) break;
        dirs[i][name_sz] = '\0';
        fseek(f, 4, SEEK_CUR); /* unused u32 */
    }

    typedef struct {
        uint16_t dir_id;
        uint32_t orig_size;
        uint32_t comp_size;
        uint32_t data_offset;
        uint16_t flags;
        char name[260];
    } Is3File;

    Is3File *files = (Is3File *)calloc(num_files ? num_files : 1, sizeof(Is3File));
    if (!files) {
        for (uint16_t i = 0; i < num_dirs; ++i) free(dirs[i]);
        free(dirs);
        fclose(f);
        return false;
    }

    fseek(f, (long)files_list_off, SEEK_SET);
    for (uint16_t i = 0; i < num_files; ++i) {
        uint8_t fh[30];
        if (fread(fh, 1, 30, f) != 30) break;
        files[i].dir_id      = rd_u16(fh + 1);
        files[i].orig_size   = rd_u32(fh + 3);
        files[i].comp_size   = rd_u32(fh + 7);
        files[i].data_offset = rd_u32(fh + 11);
        files[i].flags       = rd_u16(fh + 25);
        uint8_t name_sz      = fh[29];
        size_t rd_ln = (size_t)name_sz + 1;
        char nbuf[300] = {0};
        if (fread(nbuf, 1, rd_ln, f) != rd_ln) break;
        nbuf[name_sz] = '\0';
        strncpy(files[i].name, nbuf, 259);
        fseek(f, 12, SEEK_CUR); /* versionMS, versionLS, unused */
    }

    bool ok = true;
    uint64_t done_bytes = 0;
    uint64_t total_bytes = total_orig_size ? (uint64_t)total_orig_size : 1;

    for (uint16_t i = 0; i < num_files && ok; ++i) {
        const char *dname = (files[i].dir_id < num_dirs && dirs[files[i].dir_id]) ? dirs[files[i].dir_id] : "";
        char rel[512];
        if (dname[0]) snprintf(rel, sizeof(rel), "%s\\%s", dname, files[i].name);
        else snprintf(rel, sizeof(rel), "%s", files[i].name);

        wchar_t wrel[512], dest_path[1024];
        MultiByteToWideChar(CP_ACP, 0, rel, -1, wrel, 512);
        swprintf(dest_path, 1024, L"%s\\%s", dest_dir, wrel);
        make_parent_dirs(dest_path);

        if (progress) {
            wchar_t msg[320];
            swprintf(msg, 320, L"Descompactando %s...", wrel);
            progress(done_bytes, total_bytes, msg, progress_user);
        }

        uint8_t *comp = (uint8_t *)malloc(files[i].comp_size ? files[i].comp_size : 1);
        if (!comp) { ok = false; break; }
        fseek(f, (long)files[i].data_offset, SEEK_SET);
        if (files[i].comp_size > 0 && fread(comp, 1, files[i].comp_size, f) != files[i].comp_size) {
            free(comp);
            swprintf(err_buf, err_cap, L"Falha ao ler dados comprimidos de %s.", wrel);
            ok = false;
            break;
        }

        FILE *out = _wfopen(dest_path, L"wb");
        if (!out) {
            free(comp);
            swprintf(err_buf, err_cap, L"N\u00e3o foi poss\u00edvel gravar %s.", dest_path);
            ok = false;
            break;
        }

        if (files[i].flags & 0x10) {
            if (files[i].comp_size > 0 && fwrite(comp, 1, files[i].comp_size, out) != files[i].comp_size) {
                ok = false;
            }
        } else {
            uint64_t written = 0;
            if (!pkware_explode(comp, files[i].comp_size, out, &written) || written != files[i].orig_size) {
                swprintf(err_buf, err_cap, L"Falha ao descompactar %s do ARCHIVE.Z.", wrel);
                ok = false;
            }
        }
        fclose(out);
        free(comp);
        done_bytes += files[i].orig_size;
    }

    for (uint16_t i = 0; i < num_dirs; ++i) free(dirs[i]);
    free(dirs);
    free(files);
    fclose(f);
    return ok;
}

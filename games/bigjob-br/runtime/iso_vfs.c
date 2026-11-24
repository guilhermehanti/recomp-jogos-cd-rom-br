#define _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "iso_vfs.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <direct.h>

#define MAX_ISO_FILES 512
#define MAX_VFS_FDS   64

typedef struct IsoEntry {
    char     path[128];
    char     base[64];
    char     host_path[MAX_PATH];
    uint32_t lba;
    uint32_t size;
} IsoEntry;

typedef struct VfsHandle {
    bool     in_use;
    bool     is_iso;
    bool     read_only;
    uint32_t iso_start;
    uint32_t iso_size;
    uint32_t iso_pos;
    FILE    *host_fp;
} VfsHandle;

static FILE     *g_iso_fp = NULL;
static bool      g_dir_mode = false;
static IsoEntry  g_iso_files[MAX_ISO_FILES];
static int       g_iso_count = 0;
static VfsHandle g_fds[MAX_VFS_FDS];
static char      g_save_dir[MAX_PATH] = "saves";

static uint32_t rd32le(const uint8_t *p) {
    return (uint32_t)p[0] | ((uint32_t)p[1] << 8) | ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
}

static int ci_cmp(const char *a, const char *b) {
    while (*a && *b) {
        int ca = toupper((unsigned char)*a++);
        int cb = toupper((unsigned char)*b++);
        if (ca != cb) return ca - cb;
    }
    return toupper((unsigned char)*a) - toupper((unsigned char)*b);
}

static const char *basename_of(const char *p) {
    const char *last = p;
    for (const char *s = p; *s; ++s) {
        if (*s == '\\' || *s == '/' || *s == ':') last = s + 1;
    }
    return last;
}

static void walk_iso_dir(uint32_t lba, uint32_t dir_size, const char *prefix) {
    if (!g_iso_fp || dir_size == 0 || dir_size > 1024 * 1024) return;
    uint8_t *buf = (uint8_t *)malloc(dir_size);
    if (!buf) return;
    _fseeki64(g_iso_fp, (int64_t)lba * 2048LL, SEEK_SET);
    if (fread(buf, 1, dir_size, g_iso_fp) != dir_size) {
        free(buf);
        return;
    }

    uint32_t pos = 0;
    while (pos < dir_size) {
        uint8_t rec_len = buf[pos];
        if (rec_len == 0) {
            pos = ((pos / 2048u) + 1u) * 2048u;
            continue;
        }
        if (pos + rec_len > dir_size) break;
        uint32_t ext_lba  = rd32le(buf + pos + 2);
        uint32_t ext_size = rd32le(buf + pos + 10);
        uint8_t  flags    = buf[pos + 25];
        uint8_t  name_len = buf[pos + 32];

        if (name_len > 0 && !(name_len == 1 && (buf[pos + 33] == 0 || buf[pos + 33] == 1))) {
            char name[64];
            size_t n = name_len < sizeof(name) - 1 ? name_len : sizeof(name) - 1;
            memcpy(name, buf + pos + 33, n);
            name[n] = '\0';
            char *semi = strchr(name, ';');
            if (semi) *semi = '\0';

            char full[128];
            if (prefix[0]) snprintf(full, sizeof(full), "%s/%s", prefix, name);
            else           snprintf(full, sizeof(full), "%s", name);

            if (flags & 0x02) {
                walk_iso_dir(ext_lba, ext_size, full);
            } else if (g_iso_count < MAX_ISO_FILES) {
                IsoEntry *e = &g_iso_files[g_iso_count++];
                snprintf(e->path, sizeof(e->path), "%s", full);
                snprintf(e->base, sizeof(e->base), "%s", name);
                e->lba  = ext_lba;
                e->size = ext_size;
            }
        }
        pos += rec_len;
    }
    free(buf);
}

static void mkdir_recursive(const char *path) {
    char tmp[320];
    snprintf(tmp, sizeof(tmp), "%s", path);
    for (char *p = tmp + 1; *p; ++p) {
        if (*p == '\\' || *p == '/') {
            if (*(p - 1) == ':') continue;
            char saved = *p;
            *p = '\0';
            _mkdir(tmp);
            *p = saved;
        }
    }
    _mkdir(tmp);
}

static void walk_host_dir(const char *host_dir, const char *rel_prefix) {
    char pattern[MAX_PATH];
    snprintf(pattern, sizeof(pattern), "%s\\*", host_dir);
    WIN32_FIND_DATAA fd;
    HANDLE h = FindFirstFileA(pattern, &fd);
    if (h == INVALID_HANDLE_VALUE) return;
    do {
        if (strcmp(fd.cFileName, ".") == 0 || strcmp(fd.cFileName, "..") == 0) continue;
        char child_host[MAX_PATH];
        snprintf(child_host, sizeof(child_host), "%s\\%s", host_dir, fd.cFileName);
        char child_rel[128];
        if (rel_prefix[0]) snprintf(child_rel, sizeof(child_rel), "%s/%s", rel_prefix, fd.cFileName);
        else               snprintf(child_rel, sizeof(child_rel), "%s", fd.cFileName);
        if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            if (ci_cmp(fd.cFileName, "saves") == 0 || ci_cmp(fd.cFileName, "recomp_obj") == 0 || ci_cmp(fd.cFileName, "recomp_gen") == 0) {
                continue;
            }
            walk_host_dir(child_host, child_rel);
        } else if (g_iso_count < MAX_ISO_FILES) {
            IsoEntry *e = &g_iso_files[g_iso_count++];
            snprintf(e->path, sizeof(e->path), "%s", child_rel);
            snprintf(e->base, sizeof(e->base), "%s", fd.cFileName);
            snprintf(e->host_path, sizeof(e->host_path), "%s", child_host);
            e->lba  = 0;
            e->size = fd.nFileSizeLow;
        }
    } while (FindNextFileA(h, &fd));
    FindClose(h);
}

static const IsoEntry *find_iso_entry(const char *name) {
    if (!name) return NULL;
    const char *base = basename_of(name);
    for (int i = 0; i < g_iso_count; ++i) {
        if (ci_cmp(g_iso_files[i].path, name) == 0 || ci_cmp(g_iso_files[i].base, base) == 0) {
            return &g_iso_files[i];
        }
    }
    return NULL;
}

bool iso_vfs_init(const char *iso_path, const char *save_dir) {
    iso_vfs_shutdown();
    if (save_dir && save_dir[0]) {
        snprintf(g_save_dir, sizeof(g_save_dir), "%s", save_dir);
    }
    mkdir_recursive(g_save_dir);
    if (!iso_path || !iso_path[0]) return false;

    DWORD attrs = GetFileAttributesA(iso_path);
    if (attrs != INVALID_FILE_ATTRIBUTES && (attrs & FILE_ATTRIBUTE_DIRECTORY)) {
        g_dir_mode = true;
        g_iso_count = 0;
        walk_host_dir(iso_path, "");
        if (!find_iso_entry("BIGJOBNG.EXE")) {
            fprintf(stderr, "[iso_vfs] Directory does not contain BIGJOBNG.EXE: %s\n", iso_path);
            g_dir_mode = false;
            g_iso_count = 0;
            return false;
        }
        return g_iso_count > 0;
    }

    g_dir_mode = false;
    g_iso_fp = fopen(iso_path, "rb");
    if (!g_iso_fp) {
        fprintf(stderr, "[iso_vfs] Cannot open ISO: %s\n", iso_path);
        return false;
    }

    uint8_t pvd[2048];
    _fseeki64(g_iso_fp, 16LL * 2048LL, SEEK_SET);
    if (fread(pvd, 1, sizeof(pvd), g_iso_fp) != sizeof(pvd) || memcmp(pvd + 1, "CD001", 5) != 0) {
        fprintf(stderr, "[iso_vfs] Invalid ISO9660 PVD\n");
        fclose(g_iso_fp);
        g_iso_fp = NULL;
        return false;
    }

    uint32_t root_lba  = rd32le(pvd + 156 + 2);
    uint32_t root_size = rd32le(pvd + 156 + 10);
    g_iso_count = 0;
    walk_iso_dir(root_lba, root_size, "");
    return g_iso_count > 0;
}

void iso_vfs_shutdown(void) {
    for (int i = 0; i < MAX_VFS_FDS; ++i) {
        if (g_fds[i].in_use && !g_fds[i].is_iso && g_fds[i].host_fp) {
            fclose(g_fds[i].host_fp);
        }
    }
    memset(g_fds, 0, sizeof(g_fds));
    if (g_iso_fp) {
        fclose(g_iso_fp);
        g_iso_fp = NULL;
    }
    g_dir_mode = false;
    g_iso_count = 0;
}

const char *iso_vfs_save_dir(void) {
    return g_save_dir;
}

bool iso_vfs_find(const char *name, uint32_t *out_lba, uint32_t *out_size) {
    const IsoEntry *e = find_iso_entry(name);
    if (!e) return false;
    if (out_lba)  *out_lba  = e->lba;
    if (out_size) *out_size = e->size;
    return true;
}

uint8_t *iso_vfs_read_all(const char *name, size_t *out_size) {
    const IsoEntry *e = find_iso_entry(name);
    if (!e) return NULL;
    if (g_dir_mode) {
        FILE *fp = fopen(e->host_path, "rb");
        if (!fp) return NULL;
        uint8_t *buf = (uint8_t *)malloc(e->size);
        if (!buf) { fclose(fp); return NULL; }
        size_t got = fread(buf, 1, e->size, fp);
        fclose(fp);
        if (got != e->size) { free(buf); return NULL; }
        if (out_size) *out_size = e->size;
        return buf;
    }
    if (!g_iso_fp) return NULL;
    uint8_t *buf = (uint8_t *)malloc(e->size);
    if (!buf) return NULL;
    _fseeki64(g_iso_fp, (int64_t)e->lba * 2048LL, SEEK_SET);
    if (fread(buf, 1, e->size, g_iso_fp) != e->size) {
        free(buf);
        return NULL;
    }
    if (out_size) *out_size = e->size;
    return buf;
}

static bool is_local_writable_name(const char *base) {
    return ci_cmp(base, "BIGJOB.SAV") == 0 || ci_cmp(base, "BIGJOB.INI") == 0;
}

static int alloc_fd(void) {
    /* Start at 10 so Win16 fds never collide with stdin/stdout/stderr (0..2) */
    for (int i = 10; i < MAX_VFS_FDS; ++i) {
        if (!g_fds[i].in_use) return i;
    }
    return -1;
}

int16_t vfs_lopen(const char *path, int16_t mode) {
    if (!path) return -1;
    const char *base = basename_of(path);
    int fd = alloc_fd();
    if (fd < 0) return -1;

    if (is_local_writable_name(base) || (mode & 3) != 0) {
        char local_path[320];
        snprintf(local_path, sizeof(local_path), "%s\\%s", g_save_dir, base);
        const char *fmode = ((mode & 3) == 0) ? "rb" : ((mode & 3) == 1) ? "wb" : "r+b";
        FILE *fp = fopen(local_path, fmode);
        if (!fp && (mode & 3) == 2) fp = fopen(local_path, "w+b");
        if (!fp) return -1;
        g_fds[fd].in_use    = true;
        g_fds[fd].is_iso    = false;
        g_fds[fd].read_only = false;
        g_fds[fd].host_fp   = fp;
        return (int16_t)fd;
    }

    const IsoEntry *e = find_iso_entry(path);
    if (e) {
        if (g_dir_mode) {
            FILE *fp = fopen(e->host_path, "rb");
            if (!fp) return -1;
            g_fds[fd].in_use    = true;
            g_fds[fd].is_iso    = false;
            g_fds[fd].read_only = true;
            g_fds[fd].host_fp   = fp;
            return (int16_t)fd;
        }
        g_fds[fd].in_use    = true;
        g_fds[fd].is_iso    = true;
        g_fds[fd].read_only = true;
        g_fds[fd].iso_start = e->lba * 2048u;
        g_fds[fd].iso_size  = e->size;
        g_fds[fd].iso_pos   = 0;
        g_fds[fd].host_fp   = NULL;
        return (int16_t)fd;
    }
    return -1;
}

int16_t vfs_lcreat(const char *path, int16_t attr) {
    (void)attr;
    if (!path) return -1;
    const char *base = basename_of(path);
    int fd = alloc_fd();
    if (fd < 0) return -1;
    char local_path[320];
    snprintf(local_path, sizeof(local_path), "%s\\%s", g_save_dir, base);
    FILE *fp = fopen(local_path, "w+b");
    if (!fp) return -1;
    g_fds[fd].in_use  = true;
    g_fds[fd].is_iso  = false;
    g_fds[fd].host_fp = fp;
    return (int16_t)fd;
}

int16_t vfs_lclose(int16_t fd) {
    if (fd < 0 || fd >= MAX_VFS_FDS || !g_fds[fd].in_use) return -1;
    if (!g_fds[fd].is_iso && g_fds[fd].host_fp) fclose(g_fds[fd].host_fp);
    memset(&g_fds[fd], 0, sizeof(VfsHandle));
    return 0;
}

int32_t vfs_llseek(int16_t fd, int32_t offset, int16_t origin) {
    if (fd < 0 || fd >= MAX_VFS_FDS || !g_fds[fd].in_use) return -1;
    VfsHandle *h = &g_fds[fd];
    if (h->is_iso) {
        int64_t target = 0;
        if (origin == 0)      target = offset;
        else if (origin == 1) target = (int64_t)h->iso_pos + offset;
        else if (origin == 2) target = (int64_t)h->iso_size + offset;
        else return -1;
        if (target < 0) target = 0;
        if (target > (int64_t)h->iso_size) target = h->iso_size;
        h->iso_pos = (uint32_t)target;
        return (int32_t)h->iso_pos;
    }
    if (fseek(h->host_fp, offset, origin) != 0) return -1;
    return (int32_t)ftell(h->host_fp);
}

int32_t vfs_hread(int16_t fd, uint8_t *buf, int32_t count) {
    if (fd < 0 || fd >= MAX_VFS_FDS || !g_fds[fd].in_use || !buf || count < 0) return -1;
    VfsHandle *h = &g_fds[fd];
    if (h->is_iso) {
        uint32_t rem = h->iso_size - h->iso_pos;
        uint32_t to_read = ((uint32_t)count < rem) ? (uint32_t)count : rem;
        if (to_read == 0) return 0;
        _fseeki64(g_iso_fp, (int64_t)h->iso_start + (int64_t)h->iso_pos, SEEK_SET);
        size_t got = fread(buf, 1, to_read, g_iso_fp);
        h->iso_pos += (uint32_t)got;
        return (int32_t)got;
    }
    return (int32_t)fread(buf, 1, (size_t)count, h->host_fp);
}

int32_t vfs_hwrite(int16_t fd, const uint8_t *buf, int32_t count) {
    if (fd < 0 || fd >= MAX_VFS_FDS || !g_fds[fd].in_use || !buf || count < 0) return -1;
    VfsHandle *h = &g_fds[fd];
    if (h->is_iso || h->read_only) return -1; /* CD/ISO assets are strictly read-only */
    return (int32_t)fwrite(buf, 1, (size_t)count, h->host_fp);
}

bool vfs_exists(const char *path) {
    if (!path) return false;
    const char *base = basename_of(path);
    if (is_local_writable_name(base)) {
        char local_path[320];
        snprintf(local_path, sizeof(local_path), "%s\\%s", g_save_dir, base);
        FILE *fp = fopen(local_path, "rb");
        if (fp) { fclose(fp); return true; }
        return false;
    }
    return iso_vfs_find(path, NULL, NULL);
}

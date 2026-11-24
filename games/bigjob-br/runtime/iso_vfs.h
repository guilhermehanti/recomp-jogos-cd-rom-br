#ifndef BIGJOB_ISO_VFS_H
#define BIGJOB_ISO_VFS_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Initialize read-only ISO9660 or extracted CD directory VFS and writable local save directory */
bool iso_vfs_init(const char *iso_path, const char *save_dir);
void iso_vfs_shutdown(void);

/* Locate a file inside the ISO by basename or path (case-insensitive) */
bool iso_vfs_find(const char *name, uint32_t *out_lba, uint32_t *out_size);

/* Read entire ISO file into newly allocated buffer (caller frees) */
uint8_t *iso_vfs_read_all(const char *name, size_t *out_size);

/* Win16 file handle API (_lopen/_lcreat/_lclose/_llseek/_hread/_hwrite) */
int16_t  vfs_lopen(const char *path, int16_t mode);
int16_t  vfs_lcreat(const char *path, int16_t attr);
int16_t  vfs_lclose(int16_t fd);
int32_t  vfs_llseek(int16_t fd, int32_t offset, int16_t origin);
int32_t  vfs_hread(int16_t fd, uint8_t *buf, int32_t count);
int32_t  vfs_hwrite(int16_t fd, const uint8_t *buf, int32_t count);
bool     vfs_exists(const char *path);

/* Path to local INI/SAV directory */
const char *iso_vfs_save_dir(void);

#ifdef __cplusplus
}
#endif

#endif /* BIGJOB_ISO_VFS_H */

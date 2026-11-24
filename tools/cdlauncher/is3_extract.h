#ifndef CDLAUNCHER_IS3_EXTRACT_H
#define CDLAUNCHER_IS3_EXTRACT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef void (*Is3ProgressFn)(uint64_t done_bytes, uint64_t total_bytes, const wchar_t *status, void *user);

/* Extract all files from an InstallShield 3.0 archive (e.g. ARCHIVE.Z) into dest_dir. */
bool is3_extract_archive(const wchar_t *archive_path, const wchar_t *dest_dir,
                         Is3ProgressFn progress, void *progress_user,
                         wchar_t *err_buf, size_t err_cap);

#endif

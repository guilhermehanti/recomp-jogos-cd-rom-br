#ifndef CDLAUNCHER_ISO_EXTRACT_H
#define CDLAUNCHER_ISO_EXTRACT_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

typedef void (*IsoProgressFn)(uint64_t done_bytes, uint64_t total_bytes, const wchar_t *status, void *user);

/* Filter callback: return true to extract this relative path (e.g. L"ORLY\\PICT.MHK"), false to skip. */
typedef bool (*IsoFilterFn)(const wchar_t *rel_path, bool is_dir, void *user);

typedef struct {
    wchar_t data_file[1024];       /* Path to .iso or Track 01 .bin */
    uint32_t sector_size;          /* 2048 or 2352 */
    uint32_t data_offset;          /* 0, 16, or 24 bytes inside each sector */
    uint64_t track1_byte_offset;   /* Start offset of Track 01 in data_file */

    bool has_audio_tracks;
    int audio_track_count;
    struct {
        int track_num;
        wchar_t file_path[1024];
        bool is_wave;
        uint64_t start_byte;
        uint64_t length_bytes;
        int index01_mm, index01_ss, index01_ff;
    } audio_tracks[32];
    int data_track_mm, data_track_ss, data_track_ff; /* Track 01 length for CUE REM DATATRACK 01 */
} CdImageInfo;

/* Open an .iso, .bin, or .cue file (automatically finds matching .cue if user picked .bin). */
bool cdimg_open(const wchar_t *selected_path, CdImageInfo *out_info, wchar_t *err_buf, size_t err_cap);

/* Check if a relative path (e.g. L"BRINV3R.EXE" or L"ARCHIVE\\ARCHIVE.Z") exists inside the ISO. */
bool cdimg_file_exists(const CdImageInfo *info, const wchar_t *rel_path);

/* Extract a single file from the ISO to an exact destination path. */
bool cdimg_extract_single(const CdImageInfo *info, const wchar_t *rel_path, const wchar_t *dest_path,
                          wchar_t *err_buf, size_t err_cap);

/* Extract all matching files from Track 01 ISO into dest_dir (if flatten_subdir != NULL, only extract
 * files directly inside that subdir and place them in dest_dir root). */
bool cdimg_extract_tree(const CdImageInfo *info, const wchar_t *dest_dir,
                        IsoFilterFn filter, void *filter_user,
                        const wchar_t *flatten_subdir,
                        IsoProgressFn progress, void *progress_user,
                        wchar_t *err_buf, size_t err_cap);

/* Extract all CD-Audio tracks (if present in .cue/.bin) into <dest_dir>\music\trackNN.wav + music.cue. */
bool cdimg_extract_audio(const CdImageInfo *info, const wchar_t *dest_dir,
                         IsoProgressFn progress, void *progress_user,
                         wchar_t *err_buf, size_t err_cap);

#endif

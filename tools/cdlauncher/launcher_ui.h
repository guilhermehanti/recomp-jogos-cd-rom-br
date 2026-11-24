#ifndef CDLAUNCHER_UI_H
#define CDLAUNCHER_UI_H

#include <windows.h>
#include <stdbool.h>
#include <stdint.h>

typedef struct {
    HWND hwnd;
    HWND label;
    HWND bar;
    int last_pct;
} ProgressWindow;

void ui_init(void);
void ui_get_exe_dir(wchar_t out_dir[1024]);
bool ui_pick_cd_image(const wchar_t *title, const wchar_t *filter, wchar_t out_path[1024]);

void ui_progress_open(ProgressWindow *pw, const wchar_t *title, const wchar_t *initial_text);
void ui_progress_update(ProgressWindow *pw, uint64_t done, uint64_t total, const wchar_t *status);
void ui_progress_close(ProgressWindow *pw);

bool ui_launch_game(const wchar_t *exe_path, const wchar_t *work_dir, wchar_t *err_buf, size_t err_cap);
bool ui_copy_file_checked(const wchar_t *src, const wchar_t *dst, bool overwrite, wchar_t *err_buf, size_t err_cap);

#endif

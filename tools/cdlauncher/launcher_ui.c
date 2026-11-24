#define _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commdlg.h>
#include <commctrl.h>
#include "launcher_ui.h"
#include <stdio.h>
#include <wchar.h>

void ui_init(void) {
    INITCOMMONCONTROLSEX icc = { sizeof(icc), ICC_PROGRESS_CLASS };
    InitCommonControlsEx(&icc);
}

void ui_get_exe_dir(wchar_t out_dir[1024]) {
    GetModuleFileNameW(NULL, out_dir, 1023);
    out_dir[1023] = L'\0';
    wchar_t *slash = wcsrchr(out_dir, L'\\');
    if (slash) *slash = L'\0';
}

bool ui_pick_cd_image(const wchar_t *title, const wchar_t *filter, wchar_t out_path[1024]) {
    out_path[0] = L'\0';
    OPENFILENAMEW ofn;
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = NULL;
    ofn.lpstrFilter = filter;
    ofn.lpstrFile = out_path;
    ofn.nMaxFile = 1024;
    ofn.lpstrTitle = title;
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY;
    return GetOpenFileNameW(&ofn) != 0;
}

static LRESULT CALLBACK progress_wnd_proc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    if (msg == WM_CLOSE) return 0; /* prevent accidental close mid-extraction */
    return DefWindowProcW(hwnd, msg, wp, lp);
}

void ui_progress_open(ProgressWindow *pw, const wchar_t *title, const wchar_t *initial_text) {
    ZeroMemory(pw, sizeof(*pw));
    pw->last_pct = -1;

    static bool reg = false;
    if (!reg) {
        WNDCLASSW wc;
        ZeroMemory(&wc, sizeof(wc));
        wc.lpfnWndProc = progress_wnd_proc;
        wc.hInstance = GetModuleHandleW(NULL);
        wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
        wc.lpszClassName = L"CdLauncherProgressWnd";
        wc.hCursor = LoadCursorW(NULL, MAKEINTRESOURCEW(32514));
        RegisterClassW(&wc);
        reg = true;
    }

    int w = 460, h = 135;
    int sx = (GetSystemMetrics(SM_CXSCREEN) - w) / 2;
    int sy = (GetSystemMetrics(SM_CYSCREEN) - h) / 2;

    pw->hwnd = CreateWindowExW(
        WS_EX_DLGMODALFRAME | WS_EX_TOPMOST,
        L"CdLauncherProgressWnd", title,
        WS_CAPTION | WS_VISIBLE,
        sx, sy, w, h, NULL, NULL, GetModuleHandleW(NULL), NULL);

    HFONT font = (HFONT)GetStockObject(DEFAULT_GUI_FONT);

    pw->label = CreateWindowExW(0, L"STATIC", initial_text,
        WS_CHILD | WS_VISIBLE | SS_PATHELLIPSIS,
        18, 18, w - 42, 24, pw->hwnd, NULL, GetModuleHandleW(NULL), NULL);
    SendMessageW(pw->label, WM_SETFONT, (WPARAM)font, TRUE);

    pw->bar = CreateWindowExW(0, PROGRESS_CLASSW, NULL,
        WS_CHILD | WS_VISIBLE | PBS_SMOOTH,
        18, 52, w - 42, 24, pw->hwnd, NULL, GetModuleHandleW(NULL), NULL);
    SendMessageW(pw->bar, PBM_SETRANGE, 0, MAKELPARAM(0, 1000));

    UpdateWindow(pw->hwnd);
}

void ui_progress_update(ProgressWindow *pw, uint64_t done, uint64_t total, const wchar_t *status) {
    if (!pw || !pw->hwnd) return;
    int pos = (total > 0) ? (int)((done * 1000ULL) / total) : 0;
    if (pos > 1000) pos = 1000;
    int pct = pos / 10;

    if (pct != pw->last_pct || status) {
        pw->last_pct = pct;
        SendMessageW(pw->bar, PBM_SETPOS, (WPARAM)pos, 0);
        if (status) {
            wchar_t text[512];
            swprintf(text, 512, L"%d%% - %s", pct, status);
            SetWindowTextW(pw->label, text);
        }
    }

    MSG msg;
    while (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
}

void ui_progress_close(ProgressWindow *pw) {
    if (pw && pw->hwnd) {
        DestroyWindow(pw->hwnd);
        pw->hwnd = NULL;
    }
}

bool ui_launch_game(const wchar_t *exe_path, const wchar_t *work_dir, wchar_t *err_buf, size_t err_cap) {
    STARTUPINFOW si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    ZeroMemory(&pi, sizeof(pi));
    si.cb = sizeof(si);

    wchar_t cmd[1200];
    swprintf(cmd, 1200, L"\"%s\"", exe_path);

    if (!CreateProcessW(exe_path, cmd, NULL, NULL, FALSE, 0, NULL, work_dir, &si, &pi)) {
        swprintf(err_buf, err_cap, L"Não foi possível iniciar o jogo (erro %lu):\n%s", GetLastError(), exe_path);
        return false;
    }
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
    return true;
}

bool ui_copy_file_checked(const wchar_t *src, const wchar_t *dst, bool overwrite, wchar_t *err_buf, size_t err_cap) {
    if (!overwrite && GetFileAttributesW(dst) != INVALID_FILE_ATTRIBUTES) {
        return true;
    }
    if (!CopyFileW(src, dst, FALSE)) {
        swprintf(err_buf, err_cap, L"Não foi possível copiar o arquivo necessário:\n%s", src);
        return false;
    }
    return true;
}

#define _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mmsystem.h>

#include "win16_api.h"
#include "win16_mem.h"
#include "iso_vfs.h"
#include "qt_movie.h"
#include "video_filter.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* ---- Stack argument helpers (no return address pushed for direct API calls) ---- */
static inline uint16_t stk16(const CpuState *cpu, uint16_t off) {
    return mem_r16(cpu->ss, (uint16_t)(cpu->esp.w + off));
}
static inline uint32_t stk32(const CpuState *cpu, uint16_t off) {
    return mem_r32(cpu->ss, (uint16_t)(cpu->esp.w + off));
}
static inline void ret_pascal(CpuState *cpu, uint16_t purge) {
    cpu->esp.w = (uint16_t)(cpu->esp.w + purge);
}
static inline void set_ret32(CpuState *cpu, uint32_t v) {
    AX = (uint16_t)(v & 0xFFFFu);
    DX = (uint16_t)(v >> 16);
}

/* ---- Minimal handle tables (Win16 16-bit handles <-> Win32 native handles) ---- */
#define MAX_WIN16_WND 32
#define MAX_WIN16_CLS 16
#define MAX_WIN16_GDI 128

typedef struct Win16ClassEntry {
    bool     in_use;
    char     name[64];
    uint16_t proc_seg;
    uint16_t proc_off;
    uint16_t cb_wnd_extra;
} Win16ClassEntry;

typedef struct Win16WndEntry {
    bool     in_use;
    HWND     hwnd32;
    uint16_t proc_seg;
    uint16_t proc_off;
    uint32_t extra_long0;
    uint16_t hpal16;
    int16_t  x, y, w, h;
} Win16WndEntry;

typedef struct Win16PalEntry {
    bool     in_use;
    uint16_t count;
    uint32_t rgba[256]; /* 0x00RRGGBB (BGRX for Win32 DIB) */
} Win16PalEntry;

int                    g_trace = 0;
static bool            g_debug = false;
static bool            g_headless = true;
static Win16ClassEntry g_classes[MAX_WIN16_CLS];
static Win16WndEntry   g_wnds[MAX_WIN16_WND];
static Win16PalEntry   g_pals[16];
static uint16_t        g_active_pal16 = 0;

/* Framebuffer 640x480 BGRX backed by a Win32 DIBSection so both StretchDIBits and GDI DrawText draw to g_fb */
static uint32_t       *g_fb = NULL;
static HDC             g_mem_hdc = NULL;
static HBITMAP         g_mem_bmp = NULL;
static HFONT           g_fonts[32];
static HRGN            g_rgns[32];
static HCURSOR         g_cursors[32];
static HCURSOR         g_cur_cursor = NULL;
static int             g_print_count = 0;
static int             g_print_band  = 0;
static bool            g_print_dirty = false;
static uint32_t       *g_print_fb    = NULL;
static int             g_fb_w = 640;
static int             g_fb_h = 480;
static HWND            g_host_hwnd = NULL;
static uint16_t        g_main_hwnd16 = 0;
static uint16_t        g_canvas_hwnd16 = 0;
static int             g_dib_blits = 0;

static void save_bmp_rgb24(const char *path, int w, int h, const uint32_t *bgrx_topdown) {
    FILE *fp = fopen(path, "wb");
    if (!fp) return;
    int row_stride = (w * 3 + 3) & ~3;
    uint32_t img_sz = (uint32_t)row_stride * (uint32_t)h;
    uint8_t hdr[54];
    memset(hdr, 0, sizeof(hdr));
    hdr[0] = 'B'; hdr[1] = 'M';
    uint32_t fsz = 54u + img_sz;
    memcpy(hdr + 2, &fsz, 4);
    uint32_t off = 54u;
    memcpy(hdr + 10, &off, 4);
    uint32_t biSize = 40u;
    memcpy(hdr + 14, &biSize, 4);
    memcpy(hdr + 18, &w, 4);
    memcpy(hdr + 22, &h, 4);
    uint16_t planes = 1, bpp = 24;
    memcpy(hdr + 26, &planes, 2);
    memcpy(hdr + 28, &bpp, 2);
    memcpy(hdr + 34, &img_sz, 4);
    fwrite(hdr, 1, 54, fp);
    uint8_t *row = (uint8_t *)calloc(1, (size_t)row_stride);
    if (row) {
        for (int y = h - 1; y >= 0; --y) {
            for (int x = 0; x < w; ++x) {
                uint32_t px = bgrx_topdown[y * w + x];
                row[x * 3 + 0] = (uint8_t)(px & 0xFF);
                row[x * 3 + 1] = (uint8_t)((px >> 8) & 0xFF);
                row[x * 3 + 2] = (uint8_t)((px >> 16) & 0xFF);
            }
            fwrite(row, 1, (size_t)row_stride, fp);
        }
        free(row);
    }
    fclose(fp);
}

/* Win16 message queue fed by Win32 host_wndproc */
#define MAX_WIN16_MSGS 256
typedef struct Win16Msg {
    uint16_t hwnd16;
    uint16_t msg;
    uint16_t wparam;
    uint32_t lparam;
} Win16Msg;

static Win16Msg g_msg_q[MAX_WIN16_MSGS];
static int      g_msg_head = 0;
static int      g_msg_tail = 0;

static void enqueue_win16_msg(uint16_t hwnd16, uint16_t msg, uint16_t wp, uint32_t lp) {
    int next = (g_msg_tail + 1) % MAX_WIN16_MSGS;
    if (next == g_msg_head) return; /* full */
    g_msg_q[g_msg_tail].hwnd16 = hwnd16;
    g_msg_q[g_msg_tail].msg    = msg;
    g_msg_q[g_msg_tail].wparam = wp;
    g_msg_q[g_msg_tail].lparam = lp;
    g_msg_tail = next;
}

static bool            g_fullscreen = false;
static WINDOWPLACEMENT g_prev_wp    = { sizeof(WINDOWPLACEMENT) };
static DWORD           g_prev_style = WS_OVERLAPPEDWINDOW;

static void calc_viewport(HWND hwnd, int *vx, int *vy, int *vw, int *vh) {
    RECT rc = { 0, 0, g_fb_w, g_fb_h };
    if (hwnd) GetClientRect(hwnd, &rc);
    int cw = rc.right - rc.left;
    int ch = rc.bottom - rc.top;
    if (cw <= 0 || ch <= 0) {
        *vx = 0; *vy = 0; *vw = g_fb_w; *vh = g_fb_h;
        return;
    }
    /* Preserve 4:3 (g_fb_w : g_fb_h) aspect ratio with centered letterbox/pillarbox */
    if (cw * g_fb_h >= ch * g_fb_w) {
        *vh = ch;
        *vw = (ch * g_fb_w) / g_fb_h;
    } else {
        *vw = cw;
        *vh = (cw * g_fb_h) / g_fb_w;
    }
    *vx = (cw - *vw) / 2;
    *vy = (ch - *vh) / 2;
}

static void toggle_fullscreen(HWND hwnd) {
    if (!hwnd) return;
    if (!g_fullscreen) {
        g_prev_style = (DWORD)GetWindowLongA(hwnd, GWL_STYLE);
        g_prev_wp.length = sizeof(WINDOWPLACEMENT);
        GetWindowPlacement(hwnd, &g_prev_wp);
        MONITORINFO mi = { sizeof(MONITORINFO) };
        if (GetMonitorInfoA(MonitorFromWindow(hwnd, MONITOR_DEFAULTTOPRIMARY), &mi)) {
            SetWindowLongA(hwnd, GWL_STYLE, (LONG)(g_prev_style & ~WS_OVERLAPPEDWINDOW));
            SetWindowPos(hwnd, HWND_TOP,
                         mi.rcMonitor.left, mi.rcMonitor.top,
                         mi.rcMonitor.right - mi.rcMonitor.left,
                         mi.rcMonitor.bottom - mi.rcMonitor.top,
                         SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
            g_fullscreen = true;
        }
    } else {
        SetWindowLongA(hwnd, GWL_STYLE, (LONG)g_prev_style);
        SetWindowPlacement(hwnd, &g_prev_wp);
        SetWindowPos(hwnd, NULL, 0, 0, 0, 0,
                     SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
        g_fullscreen = false;
    }
    InvalidateRect(hwnd, NULL, TRUE);
}

static VideoFilterMode g_video_filter = VF_HD_SMOOTH;
static uint32_t       *g_fb_2x        = NULL;
static DWORD           g_osd_until_ms = 0;
static char            g_osd_text[96] = "";
#define OSD_TIMER_ID 99

static void load_video_filter_pref(void) {
    char ini_path[MAX_PATH];
    snprintf(ini_path, sizeof(ini_path), "%s\\recomp.ini", iso_vfs_save_dir());
    int mode = (int)GetPrivateProfileIntA("Video", "Filter", (INT)VF_HD_SMOOTH, ini_path);
    if (mode < 0 || mode >= VF_COUNT) mode = VF_HD_SMOOTH;
    g_video_filter = (VideoFilterMode)mode;
}

static void save_video_filter_pref(void) {
    char ini_path[MAX_PATH];
    char val[16];
    snprintf(ini_path, sizeof(ini_path), "%s\\recomp.ini", iso_vfs_save_dir());
    snprintf(val, sizeof(val), "%d", (int)g_video_filter);
    WritePrivateProfileStringA("Video", "Filter", val, ini_path);
}

static void present_fb_to_hdc(HDC hdc) {
    if (!g_fb || !hdc) return;
    int vx = 0, vy = 0, vw = g_fb_w, vh = g_fb_h;
    calc_viewport(g_host_hwnd, &vx, &vy, &vw, &vh);

    const uint32_t *src_bits = g_fb;
    int src_w = g_fb_w;
    int src_h = g_fb_h;
    int stretch_mode = COLORONCOLOR;

    if (g_fb_2x && video_filter_apply_2x(g_video_filter, g_fb, g_fb_w, g_fb_h, g_fb_2x)) {
        src_bits = g_fb_2x;
        src_w = g_fb_w * 2;
        src_h = g_fb_h * 2;
        stretch_mode = HALFTONE;
    } else if (g_video_filter == VF_BILINEAR) {
        stretch_mode = HALFTONE;
    }

    BITMAPINFO bmi;
    memset(&bmi, 0, sizeof(bmi));
    bmi.bmiHeader.biSize        = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth       = src_w;
    bmi.bmiHeader.biHeight      = -src_h; /* top-down */
    bmi.bmiHeader.biPlanes      = 1;
    bmi.bmiHeader.biBitCount    = 32;
    bmi.bmiHeader.biCompression = BI_RGB;
    SetStretchBltMode(hdc, stretch_mode);
    if (stretch_mode == HALFTONE) {
        SetBrushOrgEx(hdc, 0, 0, NULL);
    }
    StretchDIBits(hdc, vx, vy, vw, vh, 0, 0, src_w, src_h,
                  src_bits, &bmi, DIB_RGB_COLORS, SRCCOPY);

    if (g_osd_until_ms && (int32_t)(GetTickCount() - g_osd_until_ms) < 0 && g_osd_text[0]) {
        HFONT hf = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
        HFONT oldf = (HFONT)SelectObject(hdc, hf);
        SIZE sz = { 0, 0 };
        int len = (int)strlen(g_osd_text);
        GetTextExtentPoint32A(hdc, g_osd_text, len, &sz);
        RECT box = { vx + 10, vy + 10, vx + 10 + sz.cx + 16, vy + 10 + sz.cy + 10 };
        HBRUSH bg = CreateSolidBrush(RGB(20, 20, 24));
        FillRect(hdc, &box, bg);
        DeleteObject(bg);
        FrameRect(hdc, &box, (HBRUSH)GetStockObject(WHITE_BRUSH));
        SetBkMode(hdc, TRANSPARENT);
        SetTextColor(hdc, RGB(255, 235, 110));
        TextOutA(hdc, box.left + 8, box.top + 5, g_osd_text, len);
        SelectObject(hdc, oldf);
    }
}

static void show_osd_toast(const char *msg, DWORD duration_ms) {
    snprintf(g_osd_text, sizeof(g_osd_text), "%s", msg);
    g_osd_until_ms = GetTickCount() + duration_ms;
    if (g_host_hwnd) {
        SetTimer(g_host_hwnd, OSD_TIMER_ID, duration_ms + 50, NULL);
        HDC hdc = GetDC(g_host_hwnd);
        present_fb_to_hdc(hdc);
        ReleaseDC(g_host_hwnd, hdc);
    }
}

static void cycle_video_filter(HWND hwnd) {
    (void)hwnd;
    g_video_filter = (VideoFilterMode)(((int)g_video_filter + 1) % VF_COUNT);
    save_video_filter_pref();
    char msg[96];
    snprintf(msg, sizeof(msg), "Filtro: %s (F9)", video_filter_name(g_video_filter));
    show_osd_toast(msg, 2000);
}

static LRESULT CALLBACK host_wndproc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        RECT rc;
        GetClientRect(hwnd, &rc);
        int vx = 0, vy = 0, vw = g_fb_w, vh = g_fb_h;
        calc_viewport(hwnd, &vx, &vy, &vw, &vh);
        HBRUSH blk = (HBRUSH)GetStockObject(BLACK_BRUSH);
        if (vx > 0) {
            RECT r1 = { 0, 0, vx, rc.bottom };
            RECT r2 = { vx + vw, 0, rc.right, rc.bottom };
            FillRect(hdc, &r1, blk);
            FillRect(hdc, &r2, blk);
        }
        if (vy > 0) {
            RECT r1 = { 0, 0, rc.right, vy };
            RECT r2 = { 0, vy + vh, rc.right, rc.bottom };
            FillRect(hdc, &r1, blk);
            FillRect(hdc, &r2, blk);
        }
        present_fb_to_hdc(hdc);
        EndPaint(hwnd, &ps);
        return 0;
    }
    case WM_ERASEBKGND:
        return 1;
    case WM_MOUSEMOVE:
    case WM_LBUTTONDOWN:
    case WM_LBUTTONUP:
    case WM_RBUTTONDOWN:
    case WM_RBUTTONUP: {
        int vx = 0, vy = 0, vw = g_fb_w, vh = g_fb_h;
        calc_viewport(hwnd, &vx, &vy, &vw, &vh);
        int mx = (int)(int16_t)LOWORD(lp);
        int my = (int)(int16_t)HIWORD(lp);
        int gx = (vw > 0) ? ((mx - vx) * g_fb_w) / vw : mx;
        int gy = (vh > 0) ? ((my - vy) * g_fb_h) / vh : my;
        if (gx < 0) gx = 0; else if (gx >= g_fb_w) gx = g_fb_w - 1;
        if (gy < 0) gy = 0; else if (gy >= g_fb_h) gy = g_fb_h - 1;
        uint32_t mapped_lp = (uint32_t)(((uint16_t)gy << 16) | (uint16_t)gx);
        uint16_t target = g_canvas_hwnd16 ? g_canvas_hwnd16 : g_main_hwnd16;
        if (target) enqueue_win16_msg(target, (uint16_t)msg, (uint16_t)wp, mapped_lp);
        return 0;
    }
    case WM_CHAR: {
        if (g_main_hwnd16) enqueue_win16_msg(g_main_hwnd16, WM_CHAR, (uint16_t)wp, (uint32_t)lp);
        return 0;
    }
    case WM_SYSKEYDOWN: {
        if (wp == VK_RETURN && (lp & (1 << 29))) {
            toggle_fullscreen(hwnd);
            return 0;
        }
        break;
    }
    case WM_KEYDOWN: {
        if (wp == VK_F11) {
            toggle_fullscreen(hwnd);
            return 0;
        }
        if (wp == VK_F9) {
            cycle_video_filter(hwnd);
            return 0;
        }
        /* Map NE RT_ACCELERATOR ID 1: VK_RIGHT=101, VK_LEFT=102, VK_UP=103, VK_DOWN=104, VK_F1=105 */
        uint16_t cmd = 0;
        if (wp == VK_RIGHT)      cmd = 101;
        else if (wp == VK_LEFT)  cmd = 102;
        else if (wp == VK_UP)    cmd = 103;
        else if (wp == VK_DOWN)  cmd = 104;
        else if (wp == VK_F1)    cmd = 105;
        if (cmd && g_main_hwnd16) {
            enqueue_win16_msg(g_main_hwnd16, WM_COMMAND, cmd, 0);
            return 0;
        }
        break;
    }
    case WM_TIMER:
        if (wp == OSD_TIMER_ID) {
            KillTimer(hwnd, OSD_TIMER_ID);
            g_osd_until_ms = 0;
            InvalidateRect(hwnd, NULL, FALSE);
            return 0;
        }
        break;
    case WM_SETCURSOR:
        if (LOWORD(lp) == HTCLIENT && g_cur_cursor) {
            SetCursor(g_cur_cursor);
            return TRUE;
        }
        break;
    case WM_CLOSE:
        if (g_main_hwnd16) enqueue_win16_msg(g_main_hwnd16, WM_QUIT, 0, 0);
        DestroyWindow(hwnd);
        g_host_hwnd = NULL;
        g_fullscreen = false;
        return 0;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcA(hwnd, msg, wp, lp);
}

static void ensure_host_window(void) {
    if (g_headless || g_host_hwnd) return;
    WNDCLASSA wc;
    memset(&wc, 0, sizeof(wc));
    wc.style         = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc   = host_wndproc;
    wc.hInstance     = GetModuleHandleA(NULL);
    wc.hIcon         = LoadIconA(wc.hInstance, MAKEINTRESOURCEA(1));
    wc.hCursor       = LoadCursorA(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wc.lpszClassName = "BigJobBRRecompHost";
    RegisterClassA(&wc);

    RECT rc = { 0, 0, g_fb_w, g_fb_h };
    DWORD style = WS_OVERLAPPEDWINDOW;
    AdjustWindowRect(&rc, style, FALSE);
    g_host_hwnd = CreateWindowA(
        "BigJobBRRecompHost", "Big Job BR (Recomp)",
        style, CW_USEDEFAULT, CW_USEDEFAULT,
        rc.right - rc.left, rc.bottom - rc.top,
        NULL, NULL, wc.hInstance, NULL
    );
    if (g_host_hwnd) {
        ShowWindow(g_host_hwnd, SW_SHOW);
        UpdateWindow(g_host_hwnd);
    }
}

/* Audio waveOut bridge (12x 2048-byte PCM 8-bit 22050Hz mono buffers) */
#define MAX_WAVEOUT_HDRS 16
static HWAVEOUT        g_hwo = NULL;
static WAVEHDR         g_whdr32[MAX_WAVEOUT_HDRS];
static uint8_t         g_whdr_buf[MAX_WAVEOUT_HDRS][4096];
static uint32_t        g_whdr16_ptrs[MAX_WAVEOUT_HDRS];
static bool            g_whdr_used[MAX_WAVEOUT_HDRS];

static void sync_waveout_done(void) {
    if (!g_hwo) return;
    for (int i = 0; i < MAX_WAVEOUT_HDRS; ++i) {
        if (g_whdr_used[i] && g_whdr16_ptrs[i] && (g_whdr32[i].dwFlags & WHDR_DONE)) {
            uint16_t s = (uint16_t)(g_whdr16_ptrs[i] >> 16);
            uint16_t o = (uint16_t)(g_whdr16_ptrs[i] & 0xFFFFu);
            uint32_t fl = mem_r32(s, (uint16_t)(o + 0x10));
            if (!(fl & 1u)) {
                mem_w32(s, (uint16_t)(o + 0x10), fl | 1u); /* WHDR_DONE */
            }
        }
    }
}

static int find_or_alloc_whdr(uint32_t lpwh16) {
    for (int i = 0; i < MAX_WAVEOUT_HDRS; ++i) {
        if (g_whdr_used[i] && g_whdr16_ptrs[i] == lpwh16) return i;
    }
    for (int i = 0; i < MAX_WAVEOUT_HDRS; ++i) {
        if (!g_whdr_used[i]) {
            g_whdr_used[i] = true;
            g_whdr16_ptrs[i] = lpwh16;
            memset(&g_whdr32[i], 0, sizeof(WAVEHDR));
            g_whdr32[i].lpData = (LPSTR)g_whdr_buf[i];
            return i;
        }
    }
    return -1;
}

/* ProcInstance table */
static uint32_t        g_proc_inst[32];

/* Cooperative Win16 fibers (seg37:1F13 resume / seg37:1F34 yield) */
#define MAX_WIN16_FIBERS 256
typedef struct Win16Fiber {
    bool     in_use;
    uint16_t ss_sel;
    uint16_t init_sp;
    uint16_t saved_sp;
    uint16_t obj_off;
    uint16_t obj_seg;
    LPVOID   host_fiber;
} Win16Fiber;

static LPVOID     g_main_fiber = NULL;
static Win16Fiber g_fibers[MAX_WIN16_FIBERS];
static int        g_active_fiber_idx = -1;

extern void seg37_exec(CpuState *cpu, uint16_t entry_off);

static VOID WINAPI win16_fiber_start_trampoline(LPVOID param) {
    CpuState *cpu = (CpuState *)param;
    int my_idx = g_active_fiber_idx;
    uint16_t obj_off = (my_idx >= 0) ? g_fibers[my_idx].obj_off : 0;
    uint16_t obj_seg = (my_idx >= 0) ? g_fibers[my_idx].obj_seg : 0;

    /* Match 37:1EE9..1EF5: push ax; push bx; push ax; push bx; push cs; callw 37:1E91; add sp,4; pop bx; pop cx */
    push16(cpu, obj_seg);
    push16(cpu, obj_off);
    push16(cpu, obj_seg);
    push16(cpu, obj_off);
    push16(cpu, NE_SEG_TO_SEL(37));
    push16(cpu, 0x1EF1u);
    cpu->cs = NE_SEG_TO_SEL(37);
    seg37_exec(cpu, 0x1E91u);
    SP = (uint16_t)(SP + 4);
    obj_off = pop16(cpu);
    obj_seg = pop16(cpu);

    /* Coroutine finished: switch back to main SS:SP and call 37:1EAD(obj, fiber_ss:sp) */
    uint16_t f_ss = cpu->ss, f_sp = SP;
    if (my_idx >= 0 && my_idx < MAX_WIN16_FIBERS) {
        g_fibers[my_idx].in_use = false;
    }
    g_active_fiber_idx = -1;
    cpu->ss = mem_r16(cpu->ds, 0x2220);
    SP = mem_r16(cpu->ds, 0x2222);
    push16(cpu, f_ss);
    push16(cpu, f_sp);
    push16(cpu, obj_seg);
    push16(cpu, obj_off);
    push16(cpu, NE_SEG_TO_SEL(37));
    push16(cpu, 0x1F0Au);
    cpu->cs = NE_SEG_TO_SEL(37);
    seg37_exec(cpu, 0x1EADu);
    SP = (uint16_t)(SP + 8);
    SwitchToFiber(g_main_fiber);
}

void win16_fiber_start(CpuState *cpu) {
    if (!g_main_fiber) {
        g_main_fiber = ConvertThreadToFiber(NULL);
        if (!g_main_fiber) g_main_fiber = GetCurrentFiber();
    }
    BP = (uint16_t)(BP + 1);
    push16(cpu, BP);
    BP = SP;
    push16(cpu, SI);
    push16(cpu, DI);
    SI = DI;
    push16(cpu, cpu->es);

    uint16_t obj_off = mem_r16(cpu->ss, (uint16_t)(BP + 6));
    uint16_t obj_seg = mem_r16(cpu->ss, (uint16_t)(BP + 8));
    uint16_t new_sp  = mem_r16(cpu->ss, (uint16_t)(BP + 10));
    uint16_t new_ss  = mem_r16(cpu->ss, (uint16_t)(BP + 12));

    mem_w16(cpu->ds, 0x2220, cpu->ss);
    mem_w16(cpu->ds, 0x2222, SP);
    cpu->ss = new_ss;
    SP = new_sp;

    int slot = -1;
    /* Only replace an existing in_use slot if it has the exact same stack top (abandoned fiber) or same object */
    for (int i = 0; i < MAX_WIN16_FIBERS; ++i) {
        if (g_fibers[i].in_use &&
            ((g_fibers[i].ss_sel == new_ss && g_fibers[i].init_sp == new_sp) ||
             (g_fibers[i].obj_seg == obj_seg && g_fibers[i].obj_off == obj_off))) {
            slot = i;
            break;
        }
    }
    if (slot < 0) {
        for (int i = 0; i < MAX_WIN16_FIBERS; ++i) {
            if (!g_fibers[i].in_use) { slot = i; break; }
        }
    }
    LPVOID target = NULL;
    if (slot >= 0) {
        if (g_fibers[slot].host_fiber && g_fibers[slot].host_fiber != GetCurrentFiber()) {
            DeleteFiber(g_fibers[slot].host_fiber);
            g_fibers[slot].host_fiber = NULL;
        }
        g_fibers[slot].in_use = true;
        g_fibers[slot].ss_sel = new_ss;
        g_fibers[slot].init_sp = new_sp;
        g_fibers[slot].saved_sp = new_sp;
        g_fibers[slot].obj_off = obj_off;
        g_fibers[slot].obj_seg = obj_seg;
        g_fibers[slot].host_fiber = CreateFiber(1024 * 1024, win16_fiber_start_trampoline, cpu);
        target = g_fibers[slot].host_fiber;
        g_active_fiber_idx = slot;
    } else {
        fprintf(stderr, "[win16_fiber] ERROR: fiber table full (max %d)!\n", MAX_WIN16_FIBERS);
    }
    if (target) SwitchToFiber(target);

    /* Returned to main_fiber (either via yield or completion) */
    cpu->es = pop16(cpu);
    DI = pop16(cpu);
    SI = pop16(cpu);
    BP = (uint16_t)(pop16(cpu) - 1);
    (void)pop16(cpu);
    cpu->cs = pop16(cpu);
}

void win16_fiber_resume(CpuState *cpu) {
    if (!g_main_fiber) {
        g_main_fiber = ConvertThreadToFiber(NULL);
        if (!g_main_fiber) g_main_fiber = GetCurrentFiber();
    }
    BP = (uint16_t)(BP + 1);
    push16(cpu, BP);
    BP = SP;
    push16(cpu, SI);
    push16(cpu, DI);
    SI = DI;
    push16(cpu, cpu->es);

    uint16_t new_sp = mem_r16(cpu->ss, (uint16_t)(BP + 6));
    uint16_t new_ss = mem_r16(cpu->ss, (uint16_t)(BP + 8));

    mem_w16(cpu->ds, 0x2220, cpu->ss);
    mem_w16(cpu->ds, 0x2222, SP);
    cpu->ss = new_ss;
    SP = new_sp;

    LPVOID target = NULL;
    for (int i = 0; i < MAX_WIN16_FIBERS; ++i) {
        if (g_fibers[i].in_use && g_fibers[i].ss_sel == new_ss && g_fibers[i].saved_sp == new_sp) {
            target = g_fibers[i].host_fiber;
            g_active_fiber_idx = i;
            break;
        }
    }
    if (!target) {
        fprintf(stderr, "[win16_fiber] WARN: resume %04X:%04X not found!\n", new_ss, new_sp);
        /* Restore main SS:SP so we don't return on the fiber stack! */
        cpu->ss = mem_r16(cpu->ds, 0x2220);
        SP = mem_r16(cpu->ds, 0x2222);
    } else {
        SwitchToFiber(target);
    }

    cpu->es = pop16(cpu);
    DI = pop16(cpu);
    SI = pop16(cpu);
    BP = (uint16_t)(pop16(cpu) - 1);
    (void)pop16(cpu);
    cpu->cs = pop16(cpu);
}

void win16_fiber_yield(CpuState *cpu) {
    /* Match 37:1F34 prologue: inc bp; push bp; mov bp,sp; push si; push di; mov si,di; push es */
    BP = (uint16_t)(BP + 1);
    push16(cpu, BP);
    BP = SP;
    push16(cpu, SI);
    push16(cpu, DI);
    SI = DI;
    push16(cpu, cpu->es);

    uint16_t out_off = mem_r16(cpu->ss, (uint16_t)(BP + 6));
    uint16_t out_seg = mem_r16(cpu->ss, (uint16_t)(BP + 8));
    mem_w16(out_seg, (uint16_t)(out_off + 2), cpu->ss);
    mem_w16(out_seg, out_off, SP);

    if (g_active_fiber_idx >= 0 && g_active_fiber_idx < MAX_WIN16_FIBERS) {
        g_fibers[g_active_fiber_idx].ss_sel = cpu->ss;
        g_fibers[g_active_fiber_idx].saved_sp = SP;
    }
    g_active_fiber_idx = -1;

    /* Restore main SS:SP from DGROUP 2220:2222 */
    cpu->ss = mem_r16(cpu->ds, 0x2220);
    SP = mem_r16(cpu->ds, 0x2222);

    SwitchToFiber(g_main_fiber);

    /* Resumed back on this fiber when main_fiber calls 37:1F13 again!
     * Execute 37:1F2E..1F33 epilogue on fiber stack: pop es, di, si, bp; dec bp; lretw */
    cpu->es = pop16(cpu);
    DI = pop16(cpu);
    SI = pop16(cpu);
    BP = (uint16_t)(pop16(cpu) - 1);
    (void)pop16(cpu);
    cpu->cs = pop16(cpu);
}

bool win16_api_init(bool headless) {
    g_headless = headless;
    g_trace = getenv("BIGJOB_TRACE") != NULL;
    g_debug = g_trace || getenv("BIGJOB_DEBUG") != NULL;
    memset(g_classes, 0, sizeof(g_classes));
    memset(g_wnds, 0, sizeof(g_wnds));
    memset(g_pals, 0, sizeof(g_pals));
    memset(g_whdr32, 0, sizeof(g_whdr32));
    memset(g_whdr16_ptrs, 0, sizeof(g_whdr16_ptrs));
    memset(g_whdr_used, 0, sizeof(g_whdr_used));
    memset(g_proc_inst, 0, sizeof(g_proc_inst));
    memset(g_fonts, 0, sizeof(g_fonts));
    memset(g_rgns, 0, sizeof(g_rgns));
    memset(g_cursors, 0, sizeof(g_cursors));
    g_cur_cursor = LoadCursorA(NULL, IDC_ARROW);
    g_active_pal16 = 0;
    load_video_filter_pref();
    free(g_fb_2x);
    g_fb_2x = (uint32_t *)malloc((size_t)(g_fb_w * 2) * (size_t)(g_fb_h * 2) * sizeof(uint32_t));
    qt_init();

    BITMAPINFO bmi;
    memset(&bmi, 0, sizeof(bmi));
    bmi.bmiHeader.biSize        = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth       = g_fb_w;
    bmi.bmiHeader.biHeight      = -g_fb_h;
    bmi.bmiHeader.biPlanes      = 1;
    bmi.bmiHeader.biBitCount    = 32;
    bmi.bmiHeader.biCompression = BI_RGB;
    g_mem_hdc = CreateCompatibleDC(NULL);
    g_mem_bmp = CreateDIBSection(g_mem_hdc, &bmi, DIB_RGB_COLORS, (void **)&g_fb, NULL, 0);
    if (!g_mem_hdc || !g_mem_bmp || !g_fb) return false;
    SelectObject(g_mem_hdc, g_mem_bmp);
    memset(g_fb, 0, (size_t)g_fb_w * (size_t)g_fb_h * sizeof(uint32_t));
    if (!g_headless) {
        ensure_host_window();
        if (g_debug) fprintf(stderr, "[recomp] Host window created: hwnd=%p\n", (void *)g_host_hwnd);
    }
    return true;
}

void win16_api_shutdown(void) {
    qt_shutdown();
    if (g_hwo) {
        waveOutReset(g_hwo);
        for (int i = 0; i < MAX_WAVEOUT_HDRS; ++i) {
            if (g_whdr_used[i] && (g_whdr32[i].dwFlags & WHDR_PREPARED)) {
                waveOutUnprepareHeader(g_hwo, &g_whdr32[i], sizeof(WAVEHDR));
            }
        }
        waveOutClose(g_hwo);
        g_hwo = NULL;
    }
    for (int i = 1; i < MAX_WIN16_WND; ++i) {
        if (g_wnds[i].in_use && g_wnds[i].hwnd32) {
            DestroyWindow(g_wnds[i].hwnd32);
        }
    }
    for (int i = 1; i < 32; ++i) {
        if (g_fonts[i]) { DeleteObject(g_fonts[i]); g_fonts[i] = NULL; }
        if (g_rgns[i])  { DeleteObject(g_rgns[i]);  g_rgns[i]  = NULL; }
        if (g_cursors[i]) { DestroyCursor(g_cursors[i]); g_cursors[i] = NULL; }
    }
    if (g_mem_bmp) { DeleteObject(g_mem_bmp); g_mem_bmp = NULL; }
    if (g_mem_hdc) { DeleteDC(g_mem_hdc); g_mem_hdc = NULL; }
    free(g_fb_2x);
    g_fb_2x = NULL;
    free(g_print_fb);
    g_print_fb = NULL;
    g_fb = NULL;
}

uint32_t win16_invoke_wndproc(CpuState *cpu, uint16_t proc_seg, uint16_t proc_off,
                              uint16_t hwnd16, uint16_t msg16, uint16_t wparam, uint32_t lparam) {
    /* Win16 WndProc PASCAL: push hwnd16, msg16, wparam, lparam, then far return CS:IP
     * so BP+06=lparam, BP+0A=wparam, BP+0C=msg16, BP+0E=hwnd16 and lretw $0xA pops CS:IP + 10 bytes */
    uint16_t saved_cs = cpu->cs;
    push16(cpu, hwnd16);
    push16(cpu, msg16);
    push16(cpu, wparam);
    push32(cpu, lparam);
    push16(cpu, saved_cs);
    push16(cpu, 0);
    recomp_dispatch_far(cpu, proc_seg, proc_off);
    cpu->cs = saved_cs;
    return ((uint32_t)DX << 16) | AX;
}

/* ---- KERNEL ---- */
static void handle_kernel(CpuState *cpu, uint16_t ord) {
    switch (ord) {
    case 3: /* GetVersion() -> 0x0A03 (Windows 3.10) */
        AX = 0x0A03; DX = 0x0500;
        break;
    case 15: { /* GlobalAlloc(wFlags:2, dwBytes:4) purge=6 */
        uint32_t sz = stk32(cpu, 0);
        uint16_t fl = stk16(cpu, 4);
        AX = win16_global_alloc(fl, sz);
        ret_pascal(cpu, 6);
        break;
    }
    case 16: { /* GlobalReAlloc(hMem:2, dwBytes:4, wFlags:2) purge=8 */
        uint16_t fl = stk16(cpu, 0);
        uint32_t sz = stk32(cpu, 2);
        uint16_t h  = stk16(cpu, 6);
        AX = win16_global_realloc(h, sz, fl);
        ret_pascal(cpu, 8);
        break;
    }
    case 17: { /* GlobalFree(hMem:2) purge=2 */
        AX = win16_global_free(stk16(cpu, 0));
        ret_pascal(cpu, 2);
        break;
    }
    case 18: { /* GlobalLock(hMem:2) purge=2 */
        set_ret32(cpu, win16_global_lock(stk16(cpu, 0)));
        ret_pascal(cpu, 2);
        break;
    }
    case 19: { /* GlobalUnlock(hMem:2) purge=2 */
        AX = win16_global_unlock(stk16(cpu, 0));
        ret_pascal(cpu, 2);
        break;
    }
    case 20: { /* GlobalSize(hMem:2) purge=2 */
        set_ret32(cpu, win16_global_size(stk16(cpu, 0)));
        ret_pascal(cpu, 2);
        break;
    }
    case 21: { /* GlobalHandle(wMem:2) purge=2 */
        set_ret32(cpu, win16_global_handle(stk16(cpu, 0)));
        ret_pascal(cpu, 2);
        break;
    }
    case 25: { /* GlobalCompact(dwMinFree:4) purge=4 */
        set_ret32(cpu, 16u * 1024u * 1024u);
        ret_pascal(cpu, 4);
        break;
    }
    case 30: /* WaitEvent(hTask:2) purge=2 */
        AX = 0; ret_pascal(cpu, 2);
        break;
    case 36: /* GetCurrentTask() purge=0 */
        AX = 0x1001;
        break;
    case 47: { /* GetModuleHandle(lpModuleName:4) purge=4 */
        char name[64];
        uint32_t lp = stk32(cpu, 0);
        win16_read_cstr((uint16_t)(lp >> 16), (uint16_t)(lp & 0xFFFF), name, sizeof(name));
        AX = (_stricmp(name, "QTIM") == 0) ? 0x0101 : 0;
        ret_pascal(cpu, 4);
        break;
    }
    case 48: /* GetModuleUsage(hModule:2) purge=2 */
        AX = 1; ret_pascal(cpu, 2);
        break;
    case 49: { /* GetModuleFileName(hInst:2, lpBuf:4, nSize:2) purge=8 */
        uint16_t nsize = stk16(cpu, 0);
        uint32_t lpbuf = stk32(cpu, 2);
        const char *fake = "C:\\BIGJOB\\BIGJOBNG.EXE";
        win16_write_cstr((uint16_t)(lpbuf >> 16), (uint16_t)(lpbuf & 0xFFFF), fake, nsize);
        AX = (uint16_t)strlen(fake);
        ret_pascal(cpu, 8);
        break;
    }
    case 50: { /* GetProcAddress(hModule:2, lpProcName:4) purge=6 */
        uint32_t lp = stk32(cpu, 0);
        uint16_t hm = stk16(cpu, 4);
        char pname[64] = "";
        if (lp >> 16) win16_read_cstr((uint16_t)(lp >> 16), (uint16_t)lp, pname, sizeof(pname));
        if (hm == 0x0101)      set_ret32(cpu, 0xFF010001u);
        else if (hm == 0x0102) set_ret32(cpu, (_stricmp(pname, "_CMgrInitialize") == 0) ? 0xFF020002u :
                                              (_stricmp(pname, "_CMgrTerminate") == 0)  ? 0xFF020003u : 0xFF020001u);
        else if (hm == 0x0103) {
            if (strcmp(pname, "#6") == 0)       set_ret32(cpu, 0xFF030006u);
            else if (strcmp(pname, "#7") == 0)  set_ret32(cpu, 0xFF030007u);
            else if (strcmp(pname, "#11") == 0) set_ret32(cpu, 0xFF03000Bu);
            else                                set_ret32(cpu, 0);
        } else {
            set_ret32(cpu, 0);
        }
        ret_pascal(cpu, 6);
        break;
    }
    case 51: { /* MakeProcInstance(lpProc:4, hInst:2) purge=6 */
        uint32_t lp = stk32(cpu, 2);
        set_ret32(cpu, lp); /* return original far pointer directly */
        ret_pascal(cpu, 6);
        break;
    }
    case 52: /* FreeProcInstance(lpProc:4) purge=4 */
        ret_pascal(cpu, 4);
        break;
    case 58: { /* GetProfileString(app:4, key:4, def:4, buf:4, sz:2) purge=18 */
        uint16_t sz  = stk16(cpu, 0);
        uint32_t buf = stk32(cpu, 2);
        uint32_t def = stk32(cpu, 6);
        uint32_t key = stk32(cpu, 10);
        uint32_t app = stk32(cpu, 14);
        char astr[64] = "", kstr[64] = "", dstr[128] = "";
        if (app) win16_read_cstr((uint16_t)(app >> 16), (uint16_t)app, astr, sizeof(astr));
        if (key) win16_read_cstr((uint16_t)(key >> 16), (uint16_t)key, kstr, sizeof(kstr));
        if (def) win16_read_cstr((uint16_t)(def >> 16), (uint16_t)def, dstr, sizeof(dstr));
        const char *res = dstr;
        if (_stricmp(astr, "windows") == 0 && _stricmp(kstr, "device") == 0) {
            res = "BigJobVirtualPrinter,winspool,FILE:";
        }
        win16_write_cstr((uint16_t)(buf >> 16), (uint16_t)buf, res, sz);
        AX = (uint16_t)strlen(res);
        ret_pascal(cpu, 18);
        break;
    }
    case 60: /* FindResource(hInst:2, lpName:4, lpType:4) purge=10 */
        AX = 1; ret_pascal(cpu, 10);
        break;
    case 74: { /* OpenFile(lpFileName:4, lpReOpenBuff:4, wStyle:2) purge=10 */
        uint16_t style = stk16(cpu, 0);
        uint32_t ofb   = stk32(cpu, 2);
        uint32_t fname = stk32(cpu, 6);
        char path[260];
        win16_read_cstr((uint16_t)(fname >> 16), (uint16_t)fname, path, sizeof(path));
        if (style & 0x4000) { /* OF_EXIST */
            AX = vfs_exists(path) ? 1 : 0xFFFF;
        } else {
            int16_t fd = vfs_lopen(path, (int16_t)(style & 3));
            AX = (fd >= 0) ? (uint16_t)fd : 0xFFFF;
        }
        if (g_trace) fprintf(stderr, "[T]     OpenFile('%s', 0x%04X) -> %04X\n", path, style, AX);
        if (ofb) {
            win16_write_cstr((uint16_t)(ofb >> 16), (uint16_t)((ofb & 0xFFFF) + 8), path, 128);
        }
        ret_pascal(cpu, 10);
        break;
    }
    case 81: /* _lclose(fd:2) purge=2 */
        AX = (uint16_t)vfs_lclose((int16_t)stk16(cpu, 0));
        ret_pascal(cpu, 2);
        break;
    case 83: { /* _lcreat(lpPath:4, attr:2) purge=6 */
        uint16_t attr = stk16(cpu, 0);
        uint32_t lp   = stk32(cpu, 2);
        char path[260];
        win16_read_cstr((uint16_t)(lp >> 16), (uint16_t)lp, path, sizeof(path));
        AX = (uint16_t)vfs_lcreat(path, (int16_t)attr);
        ret_pascal(cpu, 6);
        break;
    }
    case 84: { /* _llseek(fd:2, lOffset:4, iOrigin:2) purge=8 */
        int16_t origin = (int16_t)stk16(cpu, 0);
        int32_t off    = (int32_t)stk32(cpu, 2);
        int16_t fd     = (int16_t)stk16(cpu, 6);
        set_ret32(cpu, (uint32_t)vfs_llseek(fd, off, origin));
        ret_pascal(cpu, 8);
        break;
    }
    case 85: { /* _lopen(lpPath:4, mode:2) purge=6 */
        int16_t mode = (int16_t)stk16(cpu, 0);
        uint32_t lp  = stk32(cpu, 2);
        char path[260];
        win16_read_cstr((uint16_t)(lp >> 16), (uint16_t)lp, path, sizeof(path));
        AX = (uint16_t)vfs_lopen(path, mode);
        ret_pascal(cpu, 6);
        break;
    }
    case 91: /* InitTask() purge=0 */
        AX = 1;
        cpu->es = NE_DGROUP_SEL;
        DI = 0x1001; /* hInstance */
        SI = 0;      /* hPrevInstance */
        BX = 0x0080; /* cmdLine offset */
        DX = 1;      /* SW_SHOWNORMAL */
        break;
    case 95: { /* LoadLibrary(lpLibFileName:4) purge=4 */
        uint32_t lp = stk32(cpu, 0);
        char name[64];
        win16_read_cstr((uint16_t)(lp >> 16), (uint16_t)lp, name, sizeof(name));
        if (_stricmp(name, "QTIM.DLL") == 0)          AX = 0x0101;
        else if (_stricmp(name, "QTIMCMGR.DLL") == 0) AX = 0x0102;
        else if (_stricmp(name, "ver.dll") == 0)      AX = 0x0103;
        else                                          AX = 2; /* < 0x20: not found */
        ret_pascal(cpu, 4);
        break;
    }
    case 96: /* FreeLibrary(hLib:2) purge=2 */
        ret_pascal(cpu, 2);
        break;
    case 107: /* SetErrorMode(wMode:2) purge=2 */
        AX = 0; ret_pascal(cpu, 2);
        break;
    case 128: { /* GetPrivateProfileString(app:4, key:4, def:4, buf:4, sz:2, file:4) purge=22 */
        uint32_t file_lp = stk32(cpu, 0);
        uint16_t sz      = stk16(cpu, 4);
        uint32_t buf_lp  = stk32(cpu, 6);
        uint32_t def_lp  = stk32(cpu, 10);
        uint32_t key_lp  = stk32(cpu, 14);
        uint32_t app_lp  = stk32(cpu, 18);
        char app[64] = "", key[64] = "", def[128] = "", fname[128] = "", local_ini[320], out[256];
        if (app_lp)  win16_read_cstr((uint16_t)(app_lp >> 16), (uint16_t)app_lp, app, sizeof(app));
        if (key_lp)  win16_read_cstr((uint16_t)(key_lp >> 16), (uint16_t)key_lp, key, sizeof(key));
        if (def_lp)  win16_read_cstr((uint16_t)(def_lp >> 16), (uint16_t)def_lp, def, sizeof(def));
        if (file_lp) win16_read_cstr((uint16_t)(file_lp >> 16), (uint16_t)file_lp, fname, sizeof(fname));
        snprintf(local_ini, sizeof(local_ini), "%s\\BIGJOB.INI", iso_vfs_save_dir());
        DWORD n = GetPrivateProfileStringA(app, key, def, out, sizeof(out), local_ini);
        win16_write_cstr((uint16_t)(buf_lp >> 16), (uint16_t)buf_lp, out, sz);
        AX = (uint16_t)(n < sz ? n : (sz ? sz - 1 : 0));
        ret_pascal(cpu, 22);
        break;
    }
    case 129: { /* WritePrivateProfileString(app:4, key:4, val:4, file:4) purge=16 */
        uint32_t val_lp = stk32(cpu, 4);
        uint32_t key_lp = stk32(cpu, 8);
        uint32_t app_lp = stk32(cpu, 12);
        char app[64] = "", key[64] = "", val[256] = "", local_ini[320];
        if (app_lp) win16_read_cstr((uint16_t)(app_lp >> 16), (uint16_t)app_lp, app, sizeof(app));
        if (key_lp) win16_read_cstr((uint16_t)(key_lp >> 16), (uint16_t)key_lp, key, sizeof(key));
        if (val_lp) win16_read_cstr((uint16_t)(val_lp >> 16), (uint16_t)val_lp, val, sizeof(val));
        snprintf(local_ini, sizeof(local_ini), "%s\\BIGJOB.INI", iso_vfs_save_dir());
        AX = (uint16_t)WritePrivateProfileStringA(app, key_lp ? key : NULL, val_lp ? val : NULL, local_ini);
        ret_pascal(cpu, 16);
        break;
    }
    case 132: /* GetWinFlags() purge=0 -> WF_PMODE | WF_CPU386 | WF_80x87 */
        AX = 0x0415; DX = 0;
        break;
    case 134: { /* GetWindowsDirectory(lpBuf:4, nSize:2) purge=6 */
        uint16_t nsize = stk16(cpu, 0);
        uint32_t lpbuf = stk32(cpu, 2);
        const char *dir = iso_vfs_save_dir();
        win16_write_cstr((uint16_t)(lpbuf >> 16), (uint16_t)lpbuf, dir, nsize);
        AX = (uint16_t)strlen(dir);
        ret_pascal(cpu, 6);
        break;
    }
    case 150: /* DirectedYield(hTask:2) purge=2 */
        ret_pascal(cpu, 2);
        break;
    case 166: /* WinExec(lpCmd:4, uCmdShow:2) purge=6 */
        AX = 33; ret_pascal(cpu, 6);
        break;
    case 348: { /* hmemcpy(dst:4, src:4, count:4) purge=12 */
        uint32_t cnt = stk32(cpu, 0);
        uint32_t src = stk32(cpu, 4);
        uint32_t dst = stk32(cpu, 8);
        win16_hmemcpy((uint16_t)(dst >> 16), (uint16_t)dst, (uint16_t)(src >> 16), (uint16_t)src, cnt);
        ret_pascal(cpu, 12);
        break;
    }
    case 349: { /* _hread(fd:2, hpvBuffer:4, cbBuffer:4) purge=10 */
        int32_t  cnt = (int32_t)stk32(cpu, 0);
        uint32_t buf = stk32(cpu, 4);
        int16_t  fd  = (int16_t)stk16(cpu, 8);
        uint8_t *p   = win16_ptr((uint16_t)(buf >> 16), (uint16_t)buf);
        set_ret32(cpu, (uint32_t)(p ? vfs_hread(fd, p, cnt) : -1));
        ret_pascal(cpu, 10);
        break;
    }
    case 350: { /* _hwrite(fd:2, hpvBuffer:4, cbBuffer:4) purge=10 */
        int32_t  cnt = (int32_t)stk32(cpu, 0);
        uint32_t buf = stk32(cpu, 4);
        int16_t  fd  = (int16_t)stk16(cpu, 8);
        uint8_t *p   = win16_ptr((uint16_t)(buf >> 16), (uint16_t)buf);
        set_ret32(cpu, (uint32_t)(p ? vfs_hwrite(fd, p, cnt) : -1));
        ret_pascal(cpu, 10);
        break;
    }
    default:
        fprintf(stderr, "[win16_api] Unhandled KERNEL!%u\n", ord);
        AX = 0;
        break;
    }
}

/* ---- USER ---- */
static void handle_user(CpuState *cpu, uint16_t ord) {
    switch (ord) {
    case 1: { /* MessageBox(hwnd:2, text:4, cap:4, type:2) purge=12 */
        uint16_t mtype = stk16(cpu, 0);
        uint32_t cap   = stk32(cpu, 2);
        uint32_t txt   = stk32(cpu, 6);
        char cbuf[128] = "", tbuf[256] = "";
        if (cap) win16_read_cstr((uint16_t)(cap >> 16), (uint16_t)cap, cbuf, sizeof(cbuf));
        if (txt) win16_read_cstr((uint16_t)(txt >> 16), (uint16_t)txt, tbuf, sizeof(tbuf));
        fprintf(stderr, "[MessageBox] %s: %s\n", cbuf, tbuf);
        AX = g_headless ? IDOK : (uint16_t)MessageBoxA(NULL, tbuf, cbuf, mtype);
        ret_pascal(cpu, 12);
        break;
    }
    case 5: /* InitApp(hInst:2) purge=2 */
        AX = 1; ret_pascal(cpu, 2);
        break;
    case 6: /* PostQuitMessage(exitCode:2) purge=2 */
        cpu->halted = true;
        cpu->exit_code = (int16_t)stk16(cpu, 0);
        if (g_debug) fprintf(stderr, "[recomp] USER!6 PostQuitMessage(exitCode=%d)\n", cpu->exit_code);
        fflush(stderr);
        ret_pascal(cpu, 2);
        break;
    case 18: /* SetCapture(hwnd:2) purge=2 */
        AX = stk16(cpu, 0); ret_pascal(cpu, 2);
        break;
    case 19: /* ReleaseCapture() purge=0 */
        AX = 1;
        break;
    case 39: { /* BeginPaint(hwnd:2, lpps:4) purge=6 */
        uint32_t lpps = stk32(cpu, 0);
        uint16_t hwnd = stk16(cpu, 4);
        if (lpps) {
            uint16_t s = (uint16_t)(lpps >> 16), o = (uint16_t)lpps;
            for (uint16_t i = 0; i < 32; ++i) mem_w8(s, (uint16_t)(o + i), 0);
            mem_w16(s, o, hwnd); /* hdc = hwnd */
            mem_w16(s, (uint16_t)(o + 8), 640);
            mem_w16(s, (uint16_t)(o + 10), 480);
        }
        AX = hwnd;
        ret_pascal(cpu, 6);
        break;
    }
    case 40: /* EndPaint(hwnd:2, lpps:4) purge=6 */
        ret_pascal(cpu, 6);
        break;
    case 41: { /* CreateWindow(...) purge=30 */
        int16_t  wh     = (int16_t)stk16(cpu, 10);
        int16_t  ww     = (int16_t)stk16(cpu, 12);
        int16_t  wy     = (int16_t)stk16(cpu, 14);
        int16_t  wx     = (int16_t)stk16(cpu, 16);
        uint32_t cls_lp = stk32(cpu, 26);
        char cname[64] = "";
        if (cls_lp) win16_read_cstr((uint16_t)(cls_lp >> 16), (uint16_t)cls_lp, cname, sizeof(cname));
        uint16_t hwnd16 = 0;
        for (uint16_t i = 1; i < MAX_WIN16_WND; ++i) {
            if (!g_wnds[i].in_use) {
                g_wnds[i].in_use = true;
                g_wnds[i].x = (wx == (int16_t)0x8000) ? 0 : wx;
                g_wnds[i].y = (wy == (int16_t)0x8000) ? 0 : wy;
                g_wnds[i].w = (ww == (int16_t)0x8000) ? 640 : ww;
                g_wnds[i].h = (wh == (int16_t)0x8000) ? 480 : wh;
                for (int c = 0; c < MAX_WIN16_CLS; ++c) {
                    if (g_classes[c].in_use && _stricmp(g_classes[c].name, cname) == 0) {
                        g_wnds[i].proc_seg = g_classes[c].proc_seg;
                        g_wnds[i].proc_off = g_classes[c].proc_off;
                        break;
                    }
                }
                hwnd16 = i;
                if (_stricmp(cname, "MMAppWindowClass") == 0 || !g_main_hwnd16) {
                    g_main_hwnd16 = hwnd16;
                    ensure_host_window();
                } else if (_stricmp(cname, "MMAppCanvasClass") == 0 || !g_canvas_hwnd16) {
                    g_canvas_hwnd16 = hwnd16;
                }
                break;
            }
        }
        AX = hwnd16;
        ret_pascal(cpu, 30);
        break;
    }
    case 42: /* ShowWindow(hwnd:2, nCmdShow:2) purge=4 */
        AX = 1; ret_pascal(cpu, 4);
        break;
    case 53: { /* DestroyWindow(hwnd:2) purge=2 */
        uint16_t h = stk16(cpu, 0);
        if (h > 0 && h < MAX_WIN16_WND) g_wnds[h].in_use = false;
        AX = 1; ret_pascal(cpu, 2);
        break;
    }
    case 57: { /* RegisterClass(lpwc:4) purge=4 */
        uint32_t lp = stk32(cpu, 0);
        uint16_t s = (uint16_t)(lp >> 16), o = (uint16_t)lp;
        uint16_t p_off = mem_r16(s, (uint16_t)(o + 2));
        uint16_t p_seg = mem_r16(s, (uint16_t)(o + 4));
        uint32_t n_lp  = mem_r32(s, (uint16_t)(o + 0x16));
        char cname[64] = "";
        if (n_lp) win16_read_cstr((uint16_t)(n_lp >> 16), (uint16_t)n_lp, cname, sizeof(cname));
        uint16_t atom = 0;
        for (int i = 0; i < MAX_WIN16_CLS; ++i) {
            if (!g_classes[i].in_use) {
                g_classes[i].in_use = true;
                snprintf(g_classes[i].name, sizeof(g_classes[i].name), "%s", cname);
                g_classes[i].proc_seg = p_seg;
                g_classes[i].proc_off = p_off;
                atom = (uint16_t)(0xC000 + i);
                break;
            }
        }
        AX = atom;
        ret_pascal(cpu, 4);
        break;
    }
    case 66: /* GetDC(hwnd:2) purge=2 */
        AX = stk16(cpu, 0) ? stk16(cpu, 0) : 1;
        ret_pascal(cpu, 2);
        break;
    case 68: /* ReleaseDC(hwnd:2, hdc:2) purge=4 */
        AX = 1; ret_pascal(cpu, 4);
        break;
    case 69: { /* SetCursor(hcur:2) purge=2 */
        uint16_t hcur = stk16(cpu, 0);
        HCURSOR hc = NULL;
        if (hcur >= 0x0400 && hcur < 0x0420) hc = g_cursors[hcur - 0x0400];
        else if (hcur == 1) hc = LoadCursorA(NULL, IDC_ARROW);
        g_cur_cursor = hc;
        if (!g_headless) SetCursor(hc);
        AX = hcur;
        ret_pascal(cpu, 2);
        break;
    }
    case 71: { /* ShowCursor(bShow:2) purge=2 */
        int bshow = (int16_t)stk16(cpu, 0);
        int cnt = g_headless ? 1 : ShowCursor(bshow != 0);
        AX = (uint16_t)cnt;
        ret_pascal(cpu, 2);
        break;
    }
    case 81: { /* FillRect(hdc:2, lprc:4, hbr:2) purge=8 */
        uint16_t hbr  = stk16(cpu, 0);
        uint32_t lprc = stk32(cpu, 2);
        if (g_mem_hdc && lprc) {
            uint16_t rs = (uint16_t)(lprc >> 16), ro = (uint16_t)lprc;
            RECT rc;
            rc.left   = (int16_t)mem_r16(rs, (uint16_t)(ro + 0));
            rc.top    = (int16_t)mem_r16(rs, (uint16_t)(ro + 2));
            rc.right  = (int16_t)mem_r16(rs, (uint16_t)(ro + 4));
            rc.bottom = (int16_t)mem_r16(rs, (uint16_t)(ro + 6));
            HBRUSH br = (HBRUSH)GetStockObject(BLACK_BRUSH);
            if (hbr >= 0x0500 && hbr < 0x0514) br = (HBRUSH)GetStockObject(hbr - 0x0500);
            FillRect(g_mem_hdc, &rc, br);
            if (g_host_hwnd) {
                HDC hdc = GetDC(g_host_hwnd);
                present_fb_to_hdc(hdc);
                ReleaseDC(g_host_hwnd, hdc);
            }
        }
        AX = 1;
        ret_pascal(cpu, 8);
        break;
    }
    case 85: { /* DrawText(hdc:2, lpStr:4, nCount:2, lpRect:4, wFormat:2) purge=14 */
        uint16_t fmt   = stk16(cpu, 0);
        uint32_t lprc  = stk32(cpu, 2);
        int16_t  cnt   = (int16_t)stk16(cpu, 6);
        uint32_t lpstr = stk32(cpu, 8);
        int h = 16;
        if (g_mem_hdc && lpstr && lprc) {
            char txt[256];
            win16_read_cstr((uint16_t)(lpstr >> 16), (uint16_t)lpstr, txt, sizeof(txt));
            uint16_t rs = (uint16_t)(lprc >> 16), ro = (uint16_t)lprc;
            RECT rc;
            rc.left   = (int16_t)mem_r16(rs, (uint16_t)(ro + 0));
            rc.top    = (int16_t)mem_r16(rs, (uint16_t)(ro + 2));
            rc.right  = (int16_t)mem_r16(rs, (uint16_t)(ro + 4));
            rc.bottom = (int16_t)mem_r16(rs, (uint16_t)(ro + 6));
            h = DrawTextA(g_mem_hdc, txt, cnt, &rc, fmt);
            if (fmt & DT_CALCRECT) {
                mem_w16(rs, (uint16_t)(ro + 0), (uint16_t)rc.left);
                mem_w16(rs, (uint16_t)(ro + 2), (uint16_t)rc.top);
                mem_w16(rs, (uint16_t)(ro + 4), (uint16_t)rc.right);
                mem_w16(rs, (uint16_t)(ro + 6), (uint16_t)rc.bottom);
            } else if (g_host_hwnd) {
                HDC hdc = GetDC(g_host_hwnd);
                present_fb_to_hdc(hdc);
                ReleaseDC(g_host_hwnd, hdc);
            }
        }
        AX = (uint16_t)h;
        ret_pascal(cpu, 14);
        break;
    }
    case 102: /* AdjustWindowRect(lprc:4, style:4, bMenu:2) purge=10 */
        ret_pascal(cpu, 10);
        break;
    case 107: /* DefWindowProc(hwnd:2, msg:2, wp:2, lp:4) purge=10 */
        set_ret32(cpu, 0); ret_pascal(cpu, 10);
        break;
    case 109: { /* PeekMessage(lpMsg:4, hwnd:2, min:2, max:2, remove:2) purge=12 */
        uint16_t remove = stk16(cpu, 0);
        uint32_t lpmsg  = stk32(cpu, 8);
        sync_waveout_done();
        if (g_fb && g_host_hwnd) {
            for (uint32_t c = 1; c < 8; ++c) {
                if (qt_tick(c, g_fb, g_fb_w, g_fb_h)) {
                    HDC hdc = GetDC(g_host_hwnd);
                    present_fb_to_hdc(hdc);
                    ReleaseDC(g_host_hwnd, hdc);
                }
            }
        }
        if (!g_headless) {
            MSG m;
            while (PeekMessageA(&m, NULL, 0, 0, PM_REMOVE)) {
                if (m.message == WM_QUIT) {
                    enqueue_win16_msg(g_main_hwnd16 ? g_main_hwnd16 : 1, WM_QUIT, 0, 0);
                } else {
                    TranslateMessage(&m);
                    DispatchMessageA(&m);
                }
            }
        }
        if (g_msg_head != g_msg_tail) {
            Win16Msg wm = g_msg_q[g_msg_head];
            if (remove & 1) g_msg_head = (g_msg_head + 1) % MAX_WIN16_MSGS;
            if (lpmsg) {
                uint16_t s = (uint16_t)(lpmsg >> 16), o = (uint16_t)lpmsg;
                mem_w16(s, (uint16_t)(o + 0), wm.hwnd16);
                mem_w16(s, (uint16_t)(o + 2), wm.msg);
                mem_w16(s, (uint16_t)(o + 4), wm.wparam);
                mem_w32(s, (uint16_t)(o + 6), wm.lparam);
                mem_w32(s, (uint16_t)(o + 10), (uint32_t)timeGetTime());
                mem_w32(s, (uint16_t)(o + 14), 0);
            }
            AX = 1;
        } else {
            Sleep(1);
            AX = 0;
        }
        ret_pascal(cpu, 12);
        break;
    }
    case 113: /* TranslateMessage(lpMsg:4) purge=4 */
        AX = 0; ret_pascal(cpu, 4);
        break;
    case 114: { /* DispatchMessage(lpMsg:4) purge=4 */
        uint32_t lpmsg = stk32(cpu, 0);
        ret_pascal(cpu, 4);
        if (lpmsg) {
            uint16_t s = (uint16_t)(lpmsg >> 16), o = (uint16_t)lpmsg;
            uint16_t hwnd16 = mem_r16(s, (uint16_t)(o + 0));
            uint16_t msg16  = mem_r16(s, (uint16_t)(o + 2));
            uint16_t wp16   = mem_r16(s, (uint16_t)(o + 4));
            uint32_t lp32   = mem_r32(s, (uint16_t)(o + 6));
            if (hwnd16 > 0 && hwnd16 < MAX_WIN16_WND && g_wnds[hwnd16].in_use && g_wnds[hwnd16].proc_seg) {
                uint32_t res = win16_invoke_wndproc(cpu, g_wnds[hwnd16].proc_seg, g_wnds[hwnd16].proc_off,
                                                    hwnd16, msg16, wp16, lp32);
                set_ret32(cpu, res);
                break;
            }
        }
        set_ret32(cpu, 0);
        break;
    }
    case 116: /* PostAppMessage(hTask:2, msg:2, wp:2, lp:4) purge=10 */
        AX = 1; ret_pascal(cpu, 10);
        break;
    case 125: /* InvalidateRect(hwnd:2, lprc:4, bErase:2) purge=8 */
        ret_pascal(cpu, 8);
        break;
    case 135: { /* GetWindowLong(hwnd:2, idx:2) purge=4 */
        uint16_t h = stk16(cpu, 2);
        set_ret32(cpu, (h > 0 && h < MAX_WIN16_WND) ? g_wnds[h].extra_long0 : 0);
        ret_pascal(cpu, 4);
        break;
    }
    case 136: { /* SetWindowLong(hwnd:2, idx:2, val:4) purge=8 */
        uint32_t val = stk32(cpu, 0);
        uint16_t h   = stk16(cpu, 6);
        uint32_t old = 0;
        if (h > 0 && h < MAX_WIN16_WND) {
            old = g_wnds[h].extra_long0;
            g_wnds[h].extra_long0 = val;
        }
        set_ret32(cpu, old);
        ret_pascal(cpu, 8);
        break;
    }
    case 173: { /* LoadCursor(hInst:2, lpName:4) purge=6 */
        uint32_t lpname = stk32(cpu, 0);
        uint16_t hinst  = stk16(cpu, 4);
        HCURSOR hc = NULL;
        if (hinst == 0) {
            uint16_t id = (uint16_t)(lpname & 0xFFFFu);
            hc = LoadCursorA(NULL, MAKEINTRESOURCEA(id));
        } else {
            /* Load NE RT_GROUP_CURSOR (12) -> RT_CURSOR (1) */
            const uint8_t *gdata = NULL;
            uint32_t gsize = 0;
            if ((lpname >> 16) == 0) {
                gdata = win16_ne_find_resource(12, (uint16_t)(lpname & 0xFFFFu), &gsize);
            }
            if (gdata && gsize >= 20) {
                uint16_t cur_id = (uint16_t)(gdata[18] | (gdata[19] << 8));
                uint32_t csize = 0;
                const uint8_t *cdata = win16_ne_find_resource(1, cur_id, &csize);
                if (cdata && csize > 4) {
                    hc = (HCURSOR)CreateIconFromResourceEx((PBYTE)cdata, csize, FALSE, 0x00030000, 0, 0, LR_DEFAULTCOLOR);
                }
            }
        }
        uint16_t hcur16 = 1;
        if (hc) {
            for (uint16_t i = 1; i < 32; ++i) {
                if (!g_cursors[i]) { g_cursors[i] = hc; hcur16 = (uint16_t)(0x0400 + i); break; }
            }
        }
        AX = hcur16;
        ret_pascal(cpu, 6);
        break;
    }
    case 174: /* LoadIcon(hInst:2, lpName:4) purge=6 */
        AX = 1; ret_pascal(cpu, 6);
        break;
    case 176: { /* LoadString(hInst:2, uID:2, lpBuf:4, nMax:2) purge=10 */
        uint16_t nmax = stk16(cpu, 0);
        uint32_t buf  = stk32(cpu, 2);
        uint16_t id   = stk16(cpu, 6);
        char tmp[256];
        size_t len = 0;
        if (win16_ne_load_string(id, tmp, sizeof(tmp), &len)) {
            win16_write_cstr((uint16_t)(buf >> 16), (uint16_t)buf, tmp, nmax);
            AX = (uint16_t)(len < nmax ? len : (nmax ? nmax - 1 : 0));
        } else {
            AX = 0;
        }
        ret_pascal(cpu, 10);
        break;
    }
    case 177: /* LoadAccelerators(hInst:2, lpName:4) purge=6 */
        AX = 1; ret_pascal(cpu, 6);
        break;
    case 178: /* TranslateAccelerator(hwnd:2, hAcc:2, lpMsg:4) purge=8 */
        AX = 0; ret_pascal(cpu, 8);
        break;
    case 179: { /* GetSystemMetrics(nIndex:2) purge=2 */
        uint16_t idx = stk16(cpu, 0);
        if (idx == 0)      AX = 640; /* SM_CXSCREEN */
        else if (idx == 1) AX = 480; /* SM_CYSCREEN */
        else               AX = (uint16_t)GetSystemMetrics(idx);
        ret_pascal(cpu, 2);
        break;
    }
    case 221: { /* ScrollDC(hdc:2, dx:2, dy:2, lprcScroll:4, lprcClip:4, hrgn:2, lprcUpd:4) purge=20 */
        uint32_t lp_upd = stk32(cpu, 0);
        uint16_t hrgn16 = stk16(cpu, 4);
        uint32_t lp_clp = stk32(cpu, 6);
        uint32_t lp_scr = stk32(cpu, 10);
        int16_t  dy     = (int16_t)stk16(cpu, 14);
        int16_t  dx     = (int16_t)stk16(cpu, 16);
        BOOL ok = TRUE;
        if (g_mem_hdc) {
            RECT rc_scr, rc_clp, rc_upd;
            RECT *p_scr = NULL, *p_clp = NULL, *p_upd = lp_upd ? &rc_upd : NULL;
            if (lp_scr) {
                uint16_t s = (uint16_t)(lp_scr >> 16), o = (uint16_t)lp_scr;
                rc_scr.left   = (int16_t)mem_r16(s, (uint16_t)(o + 0));
                rc_scr.top    = (int16_t)mem_r16(s, (uint16_t)(o + 2));
                rc_scr.right  = (int16_t)mem_r16(s, (uint16_t)(o + 4));
                rc_scr.bottom = (int16_t)mem_r16(s, (uint16_t)(o + 6));
                p_scr = &rc_scr;
            }
            if (lp_clp) {
                uint16_t s = (uint16_t)(lp_clp >> 16), o = (uint16_t)lp_clp;
                rc_clp.left   = (int16_t)mem_r16(s, (uint16_t)(o + 0));
                rc_clp.top    = (int16_t)mem_r16(s, (uint16_t)(o + 2));
                rc_clp.right  = (int16_t)mem_r16(s, (uint16_t)(o + 4));
                rc_clp.bottom = (int16_t)mem_r16(s, (uint16_t)(o + 6));
                p_clp = &rc_clp;
            }
            HRGN hr = (hrgn16 >= 0x0300 && hrgn16 < 0x0320) ? g_rgns[hrgn16 - 0x0300] : NULL;
            ok = ScrollDC(g_mem_hdc, dx, dy, p_scr, p_clp, hr, p_upd);
            if (lp_upd) {
                uint16_t s = (uint16_t)(lp_upd >> 16), o = (uint16_t)lp_upd;
                mem_w16(s, (uint16_t)(o + 0), (uint16_t)rc_upd.left);
                mem_w16(s, (uint16_t)(o + 2), (uint16_t)rc_upd.top);
                mem_w16(s, (uint16_t)(o + 4), (uint16_t)rc_upd.right);
                mem_w16(s, (uint16_t)(o + 6), (uint16_t)rc_upd.bottom);
            }
            if (g_host_hwnd) {
                HDC hdc = GetDC(g_host_hwnd);
                present_fb_to_hdc(hdc);
                ReleaseDC(g_host_hwnd, hdc);
            }
        }
        AX = (uint16_t)(ok ? 1 : 0);
        ret_pascal(cpu, 20);
        break;
    }
    case 225: /* EnumTaskWindows(hTask:2, lpEnum:4, lParam:4) purge=10 */
        AX = 1; ret_pascal(cpu, 10);
        break;
    case 229: /* GetTopWindow(hwnd:2) purge=2 */
        AX = 0; ret_pascal(cpu, 2);
        break;
    case 282: { /* SelectPalette(hdc:2, hpal:2, bForceBg:2) purge=6 */
        uint16_t hpal = stk16(cpu, 2);
        uint16_t old  = g_active_pal16;
        if (hpal > 0 && hpal < 16 && g_pals[hpal].in_use) g_active_pal16 = hpal;
        AX = old;
        ret_pascal(cpu, 6);
        break;
    }
    case 283: /* RealizePalette(hdc:2) purge=2 */
        AX = 256; ret_pascal(cpu, 2);
        break;
    case 420: { /* _wsprintf (cdecl! purge=0) */
        uint32_t out_lp = stk32(cpu, 0);
        uint32_t fmt_lp = stk32(cpu, 4);
        char fmt[256], out[512];
        win16_read_cstr((uint16_t)(fmt_lp >> 16), (uint16_t)fmt_lp, fmt, sizeof(fmt));
        /* Minimal safe formatter for %s, %d, %u, %x, %c, %% */
        uint16_t arg_off = 8;
        size_t oi = 0;
        for (size_t fi = 0; fmt[fi] && oi + 1 < sizeof(out); ++fi) {
            if (fmt[fi] != '%') { out[oi++] = fmt[fi]; continue; }
            fi++;
            bool is_long = false;
            if (fmt[fi] == 'l') { is_long = true; fi++; }
            char spec = fmt[fi];
            if (!spec) break;
            if (spec == '%') { out[oi++] = '%'; }
            else if (spec == 'd' || spec == 'i') {
                int32_t v = is_long ? (int32_t)stk32(cpu, arg_off) : (int16_t)stk16(cpu, arg_off);
                arg_off = (uint16_t)(arg_off + (is_long ? 4 : 2));
                oi += (size_t)snprintf(out + oi, sizeof(out) - oi, "%d", v);
            } else if (spec == 'u' || spec == 'x' || spec == 'X') {
                uint32_t v = is_long ? stk32(cpu, arg_off) : stk16(cpu, arg_off);
                arg_off = (uint16_t)(arg_off + (is_long ? 4 : 2));
                oi += (size_t)snprintf(out + oi, sizeof(out) - oi, spec == 'u' ? "%u" : (spec == 'x' ? "%x" : "%X"), v);
            } else if (spec == 's') {
                uint32_t slp = stk32(cpu, arg_off);
                arg_off = (uint16_t)(arg_off + 4);
                char sbuf[128] = "";
                if (slp) win16_read_cstr((uint16_t)(slp >> 16), (uint16_t)slp, sbuf, sizeof(sbuf));
                oi += (size_t)snprintf(out + oi, sizeof(out) - oi, "%s", sbuf);
            } else if (spec == 'c') {
                char c = (char)stk16(cpu, arg_off);
                arg_off = (uint16_t)(arg_off + 2);
                out[oi++] = c;
            }
        }
        out[oi] = '\0';
        win16_write_cstr((uint16_t)(out_lp >> 16), (uint16_t)out_lp, out, sizeof(out));
        AX = (uint16_t)oi;
        break;
    }
    case 483: /* SystemParametersInfo(uAction:2, uParam:2, lpvParam:4, fuWinIni:2) purge=10 */
        AX = 1; ret_pascal(cpu, 10);
        break;
    default:
        fprintf(stderr, "[win16_api] Unhandled USER!%u\n", ord);
        AX = 0;
        break;
    }
}

/* ---- GDI ---- */
static void handle_gdi(CpuState *cpu, uint16_t ord) {
    switch (ord) {
    case 2: { /* SetBkMode(hdc:2, mode:2) purge=4 */
        int mode = (int16_t)stk16(cpu, 0);
        AX = (uint16_t)(g_mem_hdc ? SetBkMode(g_mem_hdc, mode) : 1);
        ret_pascal(cpu, 4);
        break;
    }
    case 3: { /* SetMapMode(hdc:2, mode:2) purge=4 */
        int mode = (int16_t)stk16(cpu, 0);
        uint16_t hdc16 = stk16(cpu, 2);
        AX = (hdc16 == 0x0200) ? 1 : (uint16_t)(g_mem_hdc ? SetMapMode(g_mem_hdc, mode) : 1);
        ret_pascal(cpu, 4);
        break;
    }
    case 9: { /* SetTextColor(hdc:2, crColor:4) purge=6 */
        uint32_t cr = stk32(cpu, 0);
        uint32_t old = g_mem_hdc ? (uint32_t)SetTextColor(g_mem_hdc, (COLORREF)(cr & 0x00FFFFFFu)) : 0;
        set_ret32(cpu, old);
        ret_pascal(cpu, 6);
        break;
    }
    case 11: { /* SetWindowOrg(hdc:2, x:2, y:2) purge=6 */
        int y = (int16_t)stk16(cpu, 0);
        int x = (int16_t)stk16(cpu, 2);
        POINT old = { 0, 0 };
        if (g_mem_hdc) SetWindowOrgEx(g_mem_hdc, x, y, &old);
        set_ret32(cpu, ((uint32_t)(uint16_t)old.y << 16) | (uint16_t)old.x);
        ret_pascal(cpu, 6);
        break;
    }
    case 12: { /* SetWindowExt(hdc:2, x:2, y:2) purge=6 */
        int y = (int16_t)stk16(cpu, 0);
        int x = (int16_t)stk16(cpu, 2);
        uint16_t hdc16 = stk16(cpu, 4);
        SIZE old = { 1, 1 };
        if (hdc16 != 0x0200 && g_mem_hdc) SetWindowExtEx(g_mem_hdc, x, y, &old);
        set_ret32(cpu, ((uint32_t)(uint16_t)old.cy << 16) | (uint16_t)old.cx);
        ret_pascal(cpu, 6);
        break;
    }
    case 14: { /* SetViewportExt(hdc:2, x:2, y:2) purge=6 */
        int y = (int16_t)stk16(cpu, 0);
        int x = (int16_t)stk16(cpu, 2);
        uint16_t hdc16 = stk16(cpu, 4);
        SIZE old = { 1, 1 };
        if (hdc16 != 0x0200 && g_mem_hdc) SetViewportExtEx(g_mem_hdc, x, y, &old);
        set_ret32(cpu, ((uint32_t)(uint16_t)old.cy << 16) | (uint16_t)old.cx);
        ret_pascal(cpu, 6);
        break;
    }
    case 38: { /* Escape(hdc:2, nEscape:2, cbInput:2, lpvIn:4, lpvOut:4) purge=14 */
        uint32_t lpv_out = stk32(cpu, 0);
        uint16_t nesc    = stk16(cpu, 10);
        if (nesc == 3 && lpv_out) { /* NEXTBAND -> RECT(left, top, right, bottom) */
            uint16_t s = (uint16_t)(lpv_out >> 16), o = (uint16_t)lpv_out;
            if (g_print_band == 0) {
                mem_w16(s, (uint16_t)(o + 0), 0);
                mem_w16(s, (uint16_t)(o + 2), 0);
                mem_w16(s, (uint16_t)(o + 4), (uint16_t)g_fb_w);
                mem_w16(s, (uint16_t)(o + 6), (uint16_t)g_fb_h);
                g_print_band = 1;
            } else {
                mem_w16(s, (uint16_t)(o + 0), 0);
                mem_w16(s, (uint16_t)(o + 2), 0);
                mem_w16(s, (uint16_t)(o + 4), 0);
                mem_w16(s, (uint16_t)(o + 6), 0);
            }
        } else if (nesc == 12 && lpv_out) { /* GETPHYSPAGESIZE -> POINT(x, y) */
            uint16_t s = (uint16_t)(lpv_out >> 16), o = (uint16_t)lpv_out;
            mem_w16(s, (uint16_t)(o + 0), (uint16_t)g_fb_w);
            mem_w16(s, (uint16_t)(o + 2), (uint16_t)g_fb_h);
        }
        AX = 1;
        ret_pascal(cpu, 14);
        break;
    }
    case 44: { /* SelectClipRgn(hdc:2, hrgn:2) purge=4 */
        uint16_t hrgn16 = stk16(cpu, 0);
        HRGN hr = (hrgn16 >= 0x0300 && hrgn16 < 0x0320) ? g_rgns[hrgn16 - 0x0300] : NULL;
        int res = g_mem_hdc ? SelectClipRgn(g_mem_hdc, hr) : SIMPLEREGION;
        AX = (uint16_t)res;
        ret_pascal(cpu, 4);
        break;
    }
    case 45: { /* SelectObject(hdc:2, hObj:2) purge=4 */
        uint16_t hobj = stk16(cpu, 0);
        if (g_mem_hdc && hobj >= 0x0100 && hobj < 0x0120 && g_fonts[hobj - 0x0100]) {
            SelectObject(g_mem_hdc, g_fonts[hobj - 0x0100]);
        } else if (g_mem_hdc && hobj >= 0x0500 && hobj < 0x0514) {
            SelectObject(g_mem_hdc, GetStockObject(hobj - 0x0500));
        }
        AX = 1;
        ret_pascal(cpu, 4);
        break;
    }
    case 53: /* CreateDC(lpDriver:4, lpDevice:4, lpOutput:4, lpInit:4) purge=16 */
        g_print_band = 0;
        g_print_dirty = false;
        AX = 0x0200; /* Virtual printer DC */
        ret_pascal(cpu, 16);
        break;
    case 57: { /* CreateFontIndirect(lplf:4) purge=4 */
        uint32_t lp = stk32(cpu, 0);
        uint16_t hfont16 = 0;
        if (lp) {
            uint16_t s = (uint16_t)(lp >> 16), o = (uint16_t)lp;
            LOGFONTA lf;
            memset(&lf, 0, sizeof(lf));
            lf.lfHeight         = (int16_t)mem_r16(s, (uint16_t)(o + 0));
            lf.lfWidth          = (int16_t)mem_r16(s, (uint16_t)(o + 2));
            lf.lfEscapement     = (int16_t)mem_r16(s, (uint16_t)(o + 4));
            lf.lfOrientation    = (int16_t)mem_r16(s, (uint16_t)(o + 6));
            lf.lfWeight         = (int16_t)mem_r16(s, (uint16_t)(o + 8));
            lf.lfItalic         = mem_r8(s, (uint16_t)(o + 10));
            lf.lfUnderline      = mem_r8(s, (uint16_t)(o + 11));
            lf.lfStrikeOut      = mem_r8(s, (uint16_t)(o + 12));
            lf.lfCharSet        = mem_r8(s, (uint16_t)(o + 13));
            lf.lfOutPrecision   = mem_r8(s, (uint16_t)(o + 14));
            lf.lfClipPrecision  = mem_r8(s, (uint16_t)(o + 15));
            lf.lfQuality        = mem_r8(s, (uint16_t)(o + 16));
            lf.lfPitchAndFamily = mem_r8(s, (uint16_t)(o + 17));
            win16_read_cstr(s, (uint16_t)(o + 18), lf.lfFaceName, LF_FACESIZE);
            HFONT hf = CreateFontIndirectA(&lf);
            if (hf) {
                for (uint16_t i = 1; i < 32; ++i) {
                    if (!g_fonts[i]) { g_fonts[i] = hf; hfont16 = (uint16_t)(0x0100 + i); break; }
                }
            }
        }
        AX = hfont16 ? hfont16 : 1;
        ret_pascal(cpu, 4);
        break;
    }
    case 64: { /* CreateRectRgn(x1:2, y1:2, x2:2, y2:2) purge=8 */
        int y2 = (int16_t)stk16(cpu, 0);
        int x2 = (int16_t)stk16(cpu, 2);
        int y1 = (int16_t)stk16(cpu, 4);
        int x1 = (int16_t)stk16(cpu, 6);
        uint16_t hrgn16 = 0;
        HRGN hr = CreateRectRgn(x1, y1, x2, y2);
        if (hr) {
            for (uint16_t i = 1; i < 32; ++i) {
                if (!g_rgns[i]) { g_rgns[i] = hr; hrgn16 = (uint16_t)(0x0300 + i); break; }
            }
        }
        AX = hrgn16 ? hrgn16 : 1;
        ret_pascal(cpu, 8);
        break;
    }
    case 67:  AX = 1; ret_pascal(cpu, 8); break;  /* DPtoLP */
    case 68:  AX = 1; ret_pascal(cpu, 2); break;  /* DeleteDC */
    case 69: { /* DeleteObject(hObj:2) purge=2 */
        uint16_t hobj = stk16(cpu, 0);
        if (hobj >= 0x0100 && hobj < 0x0120 && g_fonts[hobj - 0x0100]) {
            DeleteObject(g_fonts[hobj - 0x0100]);
            g_fonts[hobj - 0x0100] = NULL;
        } else if (hobj >= 0x0300 && hobj < 0x0320 && g_rgns[hobj - 0x0300]) {
            DeleteObject(g_rgns[hobj - 0x0300]);
            g_rgns[hobj - 0x0300] = NULL;
        }
        AX = 1;
        ret_pascal(cpu, 2);
        break;
    }
    case 77: { /* GetClipBox(hdc:2, lprc:4) purge=6 */
        uint32_t lprc = stk32(cpu, 0);
        RECT rc = { 0, 0, g_fb_w, g_fb_h };
        int res = g_mem_hdc ? GetClipBox(g_mem_hdc, &rc) : SIMPLEREGION;
        if (lprc) {
            uint16_t s = (uint16_t)(lprc >> 16), o = (uint16_t)lprc;
            mem_w16(s, (uint16_t)(o + 0), (uint16_t)rc.left);
            mem_w16(s, (uint16_t)(o + 2), (uint16_t)rc.top);
            mem_w16(s, (uint16_t)(o + 4), (uint16_t)rc.right);
            mem_w16(s, (uint16_t)(o + 6), (uint16_t)rc.bottom);
        }
        AX = (uint16_t)res;
        ret_pascal(cpu, 6);
        break;
    }
    case 80: { /* GetDeviceCaps(hdc:2, nIndex:2) purge=4 */
        uint16_t idx = stk16(cpu, 0);
        if (idx == 8)        AX = (uint16_t)g_fb_w; /* HORZRES */
        else if (idx == 10)  AX = (uint16_t)g_fb_h; /* VERTRES */
        else if (idx == 12)  AX = 8;                /* BITSPIXEL */
        else if (idx == 14)  AX = 1;                /* PLANES */
        else if (idx == 38)  AX = 0x0100;           /* RASTERCAPS: RC_PALETTE */
        else if (idx == 104) AX = 256;              /* SIZEPALETTE */
        else                 AX = 96;
        ret_pascal(cpu, 4);
        break;
    }
    case 87: { /* GetStockObject(fnObject:2) purge=2 */
        uint16_t fn = stk16(cpu, 0);
        AX = (uint16_t)(0x0500 + (fn & 0x1F));
        ret_pascal(cpu, 2);
        break;
    }
    case 91: { /* GetTextExtent(hdc:2, lpStr:4, nCount:2) purge=8 */
        int16_t  cnt   = (int16_t)stk16(cpu, 0);
        uint32_t lpstr = stk32(cpu, 2);
        SIZE sz = { 64, 16 };
        if (g_mem_hdc && lpstr && cnt > 0) {
            char txt[256];
            win16_read_cstr((uint16_t)(lpstr >> 16), (uint16_t)lpstr, txt, sizeof(txt));
            int len = (int)strlen(txt);
            if (cnt < len) len = cnt;
            GetTextExtentPoint32A(g_mem_hdc, txt, len, &sz);
        }
        set_ret32(cpu, ((uint32_t)(uint16_t)sz.cy << 16) | (uint16_t)sz.cx);
        ret_pascal(cpu, 8);
        break;
    }
    case 97: { /* GetWindowOrg(hdc:2) purge=2 */
        POINT pt = { 0, 0 };
        if (g_mem_hdc) GetWindowOrgEx(g_mem_hdc, &pt);
        set_ret32(cpu, ((uint32_t)(uint16_t)pt.y << 16) | (uint16_t)pt.x);
        ret_pascal(cpu, 2);
        break;
    }
    case 128: { /* MulDiv(nNumber:2, nNumerator:2, nDenominator:2) purge=6 */
        int16_t den = (int16_t)stk16(cpu, 0);
        int16_t num = (int16_t)stk16(cpu, 2);
        int16_t val = (int16_t)stk16(cpu, 4);
        AX = (den != 0) ? (uint16_t)(((int32_t)val * (int32_t)num) / (int32_t)den) : 0xFFFF;
        ret_pascal(cpu, 6);
        break;
    }
    case 360: { /* CreatePalette(lpLogPal:4) purge=4 */
        uint32_t lp = stk32(cpu, 0);
        uint16_t s = (uint16_t)(lp >> 16), o = (uint16_t)lp;
        uint16_t cnt = mem_r16(s, (uint16_t)(o + 2));
        if (cnt > 256) cnt = 256;
        uint16_t hpal = 0;
        for (uint16_t i = 1; i < 16; ++i) {
            if (!g_pals[i].in_use) {
                g_pals[i].in_use = true;
                g_pals[i].count = cnt;
                for (uint16_t k = 0; k < cnt; ++k) {
                    uint8_t r = mem_r8(s, (uint16_t)(o + 4 + k * 4 + 0));
                    uint8_t g = mem_r8(s, (uint16_t)(o + 4 + k * 4 + 1));
                    uint8_t b = mem_r8(s, (uint16_t)(o + 4 + k * 4 + 2));
                    g_pals[i].rgba[k] = ((uint32_t)r << 16) | ((uint32_t)g << 8) | (uint32_t)b;
                }
                hpal = i;
                break;
            }
        }
        AX = hpal;
        ret_pascal(cpu, 4);
        break;
    }
    case 377: /* StartDoc(hdc:2, lpdi:4) purge=6 */
        g_print_band = 0;
        g_print_dirty = false;
        if (!g_print_fb) {
            g_print_fb = (uint32_t *)malloc((size_t)g_fb_w * (size_t)g_fb_h * sizeof(uint32_t));
        }
        if (g_print_fb) {
            for (int k = 0; k < g_fb_w * g_fb_h; ++k) g_print_fb[k] = 0x00FFFFFFu;
        }
        AX = 1;
        ret_pascal(cpu, 6);
        break;
    case 378: /* EndDoc(hdc:2) purge=2 */
        if (g_print_fb && g_print_dirty) {
            char path[MAX_PATH];
            g_print_count++;
            snprintf(path, sizeof(path), "%s\\print_%03d.bmp", iso_vfs_save_dir(), g_print_count);
            save_bmp_rgb24(path, g_fb_w, g_fb_h, g_print_fb);
            fprintf(stderr, "[recomp] Virtual printer saved %s (%dx%d)\n", path, g_fb_w, g_fb_h);
            char toast[96];
            snprintf(toast, sizeof(toast), "Impresso em saves\\print_%03d.bmp", g_print_count);
            show_osd_toast(toast, 3000);
            g_print_dirty = false;
        }
        AX = 1;
        ret_pascal(cpu, 2);
        break;
    case 439: { /* StretchDIBits(hdc, XDest, YDest, nDestW, nDestH, XSrc, YSrc, nSrcW, nSrcH, lpBits, lpBMI, iUsage, dwRop) purge=32 */
        uint16_t iUsage = stk16(cpu, 4);
        uint32_t lpBMI  = stk32(cpu, 6);
        uint32_t lpBits = stk32(cpu, 10);
        int16_t  src_h  = (int16_t)stk16(cpu, 14);
        int16_t  src_w  = (int16_t)stk16(cpu, 16);
        int16_t  src_y  = (int16_t)stk16(cpu, 18);
        int16_t  src_x  = (int16_t)stk16(cpu, 20);
        int16_t  dst_h  = (int16_t)stk16(cpu, 22);
        int16_t  dst_w  = (int16_t)stk16(cpu, 24);
        int16_t  dst_y  = (int16_t)stk16(cpu, 26);
        int16_t  dst_x  = (int16_t)stk16(cpu, 28);
        uint16_t hdc16  = stk16(cpu, 30);

        if (g_fb && lpBMI && lpBits && dst_w > 0 && dst_h > 0 && src_w > 0 && src_h > 0) {
            uint16_t bs = (uint16_t)(lpBMI >> 16), bo = (uint16_t)lpBMI;
            int32_t  bi_w = (int32_t)mem_r32(bs, (uint16_t)(bo + 4));
            int32_t  bi_h = (int32_t)mem_r32(bs, (uint16_t)(bo + 8));
            uint16_t bpp  = mem_r16(bs, (uint16_t)(bo + 14));
            if (bpp == 8 && bi_w > 0 && bi_h != 0) {
                bool bottom_up = (bi_h > 0);
                int32_t abs_h = bottom_up ? bi_h : -bi_h;
                int32_t stride = (bi_w + 3) & ~3;

                /* Resolve 256-entry color table to BGRX */
                uint32_t lut[256];
                Win16PalEntry *pal = (g_active_pal16 > 0 && g_active_pal16 < 16 && g_pals[g_active_pal16].in_use)
                                     ? &g_pals[g_active_pal16] : NULL;
                for (int i = 0; i < 256; ++i) {
                    if (iUsage == 1 && pal) { /* DIB_PAL_COLORS */
                        uint16_t pidx = mem_r16(bs, (uint16_t)(bo + 40 + i * 2));
                        lut[i] = pal->rgba[pidx & 0xFF];
                    } else { /* DIB_RGB_COLORS */
                        uint8_t b = mem_r8(bs, (uint16_t)(bo + 40 + i * 4 + 0));
                        uint8_t g = mem_r8(bs, (uint16_t)(bo + 40 + i * 4 + 1));
                        uint8_t r = mem_r8(bs, (uint16_t)(bo + 40 + i * 4 + 2));
                        lut[i] = ((uint32_t)r << 16) | ((uint32_t)g << 8) | (uint32_t)b;
                    }
                }

                uint16_t ps = (uint16_t)(lpBits >> 16), po = (uint16_t)lpBits;
                if (hdc16 == 0x0200) {
                    /* Virtual printer DC: composite banded strip into 640x480 virtual page buffer */
                    if (!g_print_fb) {
                        g_print_fb = (uint32_t *)malloc((size_t)g_fb_w * (size_t)g_fb_h * sizeof(uint32_t));
                        if (g_print_fb) {
                            for (int k = 0; k < g_fb_w * g_fb_h; ++k) g_print_fb[k] = 0x00FFFFFFu;
                        }
                    }
                    if (g_print_fb) {
                        for (int dy = 0; dy < dst_h; ++dy) {
                            int fy = dst_y + dy;
                            if (fy < 0 || fy >= g_fb_h) continue;
                            int sy = src_y + (dy * src_h) / dst_h;
                            int dib_row = bottom_up ? (src_y + src_h - 1 - (dy * src_h) / dst_h) : sy;
                            if (dib_row < 0 || dib_row >= abs_h) continue;
                            uint32_t row_off = (uint32_t)po + (uint32_t)dib_row * (uint32_t)stride;
                            for (int dx = 0; dx < dst_w; ++dx) {
                                int fx = dst_x + dx;
                                if (fx < 0 || fx >= g_fb_w) continue;
                                int sx = src_x + (dx * src_w) / dst_w;
                                if (sx < 0 || sx >= bi_w) continue;
                                uint32_t pix_off = row_off + (uint32_t)sx;
                                uint16_t cur_seg = (uint16_t)(ps + ((pix_off >> 16) << WIN16_AHSHIFT));
                                uint8_t idx = mem_r8(cur_seg, (uint16_t)(pix_off & 0xFFFFu));
                                g_print_fb[fy * g_fb_w + fx] = lut[idx];
                            }
                        }
                        g_print_dirty = true;
                    }
                } else {
                    for (int dy = 0; dy < dst_h; ++dy) {
                        int fy = dst_y + dy;
                        if (fy < 0 || fy >= g_fb_h) continue;
                        int sy = src_y + (dy * src_h) / dst_h;
                        /* Win16 bottom-up DIB (biHeight > 0): top of screen (dy=0) is row (src_y + src_h - 1) */
                        int dib_row = bottom_up ? (src_y + src_h - 1 - (dy * src_h) / dst_h) : sy;
                        if (dib_row < 0 || dib_row >= abs_h) continue;
                        uint32_t row_off = (uint32_t)po + (uint32_t)dib_row * (uint32_t)stride;
                        for (int dx = 0; dx < dst_w; ++dx) {
                            int fx = dst_x + dx;
                            if (fx < 0 || fx >= g_fb_w) continue;
                            int sx = src_x + (dx * src_w) / dst_w;
                            if (sx < 0 || sx >= bi_w) continue;
                            uint32_t pix_off = row_off + (uint32_t)sx;
                            uint16_t cur_seg = (uint16_t)(ps + ((pix_off >> 16) << WIN16_AHSHIFT));
                            uint8_t idx = mem_r8(cur_seg, (uint16_t)(pix_off & 0xFFFFu));
                            g_fb[fy * g_fb_w + fx] = lut[idx];
                        }
                    }
                    g_dib_blits++;
                    if (g_debug && g_dib_blits <= 5) {
                        uint32_t nonwhite = 0, nonzero = 0;
                        for (int k = 0; k < g_fb_w * g_fb_h; ++k) {
                            if (g_fb[k] != 0) nonzero++;
                            if (g_fb[k] != 0x00FFFFFFu) nonwhite++;
                        }
                        fprintf(stderr, "[recomp] StretchDIBits #%d: %dx%d bi=%dx%d bpp=%u iUsage=%u pal=%u (pal[0]=%06X pal[1]=%06X pal[255]=%06X) nonzero=%u nonwhite=%u\n",
                                g_dib_blits, src_w, src_h, bi_w, bi_h, bpp, iUsage, g_active_pal16,
                                pal ? pal->rgba[0] : 0, pal ? pal->rgba[1] : 0, pal ? pal->rgba[255] : 0,
                                nonzero, nonwhite);
                    }
                    for (uint32_t c = 1; c < 8; ++c) {
                        if (qt_is_playing(c)) qt_paint(c, g_fb, g_fb_w, g_fb_h);
                    }
                    if (g_host_hwnd) {
                        HDC hdc = GetDC(g_host_hwnd);
                        present_fb_to_hdc(hdc);
                        ReleaseDC(g_host_hwnd, hdc);
                    }
                }
            }
        }
        AX = (uint16_t)(src_h >= 0 ? src_h : -src_h);
        ret_pascal(cpu, 32);
        break;
    }
    default:
        fprintf(stderr, "[win16_api] Unhandled GDI!%u\n", ord);
        AX = 0;
        break;
    }
}

/* ---- MMSYSTEM ---- */
static void handle_mmsystem(CpuState *cpu, uint16_t ord) {
    switch (ord) {
    case 404: { /* waveOutOpen(lphwo:4, uDev:2, lpFmt:4, dwCb:4, dwInst:4, dwFlags:4) purge=22 */
        uint32_t lphwo = stk32(cpu, 18);
        if (g_headless) {
            if (lphwo) mem_w16((uint16_t)(lphwo >> 16), (uint16_t)lphwo, 1);
            AX = 0;
        } else {
            WAVEFORMATEX wfx;
            memset(&wfx, 0, sizeof(wfx));
            wfx.wFormatTag      = WAVE_FORMAT_PCM;
            wfx.nChannels       = 1;
            wfx.nSamplesPerSec  = 22050;
            wfx.nAvgBytesPerSec = 22050;
            wfx.nBlockAlign     = 1;
            wfx.wBitsPerSample  = 8;
            MMRESULT r = waveOutOpen(&g_hwo, WAVE_MAPPER, &wfx, 0, 0, CALLBACK_NULL);
            if (r == MMSYSERR_NOERROR && lphwo) {
                mem_w16((uint16_t)(lphwo >> 16), (uint16_t)lphwo, 1);
            }
            AX = (uint16_t)r;
        }
        ret_pascal(cpu, 22);
        break;
    }
    case 405: /* waveOutClose(hwo:2) purge=2 */
        if (g_hwo) {
            waveOutReset(g_hwo);
            for (int i = 0; i < MAX_WAVEOUT_HDRS; ++i) {
                if (g_whdr_used[i] && (g_whdr32[i].dwFlags & WHDR_PREPARED)) {
                    waveOutUnprepareHeader(g_hwo, &g_whdr32[i], sizeof(WAVEHDR));
                }
                g_whdr_used[i] = false;
            }
            waveOutClose(g_hwo);
            g_hwo = NULL;
        }
        AX = 0; ret_pascal(cpu, 2);
        break;
    case 406: { /* waveOutPrepareHeader(hwo:2, lpwh:4, cb:2) purge=8 */
        uint32_t lpwh = stk32(cpu, 2);
        if (lpwh) {
            find_or_alloc_whdr(lpwh);
            uint16_t s = (uint16_t)(lpwh >> 16), o = (uint16_t)lpwh;
            mem_w32(s, (uint16_t)(o + 0x10), mem_r32(s, (uint16_t)(o + 0x10)) | 2u); /* WHDR_PREPARED */
        }
        AX = 0; ret_pascal(cpu, 8);
        break;
    }
    case 407: { /* waveOutUnprepareHeader(hwo:2, lpwh:4, cb:2) purge=8 */
        uint32_t lpwh = stk32(cpu, 2);
        for (int i = 0; i < MAX_WAVEOUT_HDRS; ++i) {
            if (g_whdr_used[i] && g_whdr16_ptrs[i] == lpwh) {
                if (g_hwo && (g_whdr32[i].dwFlags & WHDR_PREPARED)) {
                    waveOutUnprepareHeader(g_hwo, &g_whdr32[i], sizeof(WAVEHDR));
                }
                g_whdr_used[i] = false;
                break;
            }
        }
        AX = 0; ret_pascal(cpu, 8);
        break;
    }
    case 408: { /* waveOutWrite(hwo:2, lpwh:4, cb:2) purge=8 */
        uint32_t lpwh = stk32(cpu, 2);
        sync_waveout_done();
        if (lpwh) {
            uint16_t s = (uint16_t)(lpwh >> 16), o = (uint16_t)lpwh;
            if (!g_hwo) {
                /* Headless mode: immediately mark WHDR_DONE */
                mem_w32(s, (uint16_t)(o + 0x10), mem_r32(s, (uint16_t)(o + 0x10)) | 1u);
                AX = 0;
            } else {
                int slot = find_or_alloc_whdr(lpwh);
                uint32_t lpdata = mem_r32(s, (uint16_t)(o + 0));
                uint32_t len    = mem_r32(s, (uint16_t)(o + 4));
                if (slot >= 0 && lpdata && len > 0) {
                    if (len > sizeof(g_whdr_buf[slot])) len = sizeof(g_whdr_buf[slot]);
                    uint8_t *src = win16_ptr((uint16_t)(lpdata >> 16), (uint16_t)lpdata);
                    if (src) memcpy(g_whdr_buf[slot], src, len);
                    if (g_whdr32[slot].dwFlags & WHDR_PREPARED) {
                        waveOutUnprepareHeader(g_hwo, &g_whdr32[slot], sizeof(WAVEHDR));
                    }
                    memset(&g_whdr32[slot], 0, sizeof(WAVEHDR));
                    g_whdr32[slot].lpData         = (LPSTR)g_whdr_buf[slot];
                    g_whdr32[slot].dwBufferLength = len;
                    /* Clear WHDR_DONE in Win16 header before submitting to hardware */
                    mem_w32(s, (uint16_t)(o + 0x10), (mem_r32(s, (uint16_t)(o + 0x10)) & ~1u) | 2u);
                    MMRESULT r = waveOutPrepareHeader(g_hwo, &g_whdr32[slot], sizeof(WAVEHDR));
                    if (r == MMSYSERR_NOERROR) {
                        r = waveOutWrite(g_hwo, &g_whdr32[slot], sizeof(WAVEHDR));
                    }
                    AX = (uint16_t)r;
                } else {
                    mem_w32(s, (uint16_t)(o + 0x10), mem_r32(s, (uint16_t)(o + 0x10)) | 1u);
                    AX = 0;
                }
            }
        } else {
            AX = 0;
        }
        ret_pascal(cpu, 8);
        break;
    }
    case 411: /* waveOutReset(hwo:2) purge=2 */
        if (g_hwo) {
            waveOutReset(g_hwo);
            sync_waveout_done();
        }
        AX = 0; ret_pascal(cpu, 2);
        break;
    case 607: /* timeGetTime() purge=0 */
        sync_waveout_done();
        set_ret32(cpu, (uint32_t)timeGetTime());
        break;
    default:
        fprintf(stderr, "[win16_api] Unhandled MMSYSTEM!%u\n", ord);
        AX = 0;
        break;
    }
}

void win16_ver_dispatch(CpuState *cpu, uint16_t ord) {
    /* Called via lcallw *fptr -> pop far return address first */
    (void)pop16(cpu);
    cpu->cs = pop16(cpu);
    if (ord == 6) { /* GetFileVersionInfoSize(lpFile:4, lpdwHandle:4) purge=8 */
        uint32_t lph = stk32(cpu, 0);
        if (lph) mem_w32((uint16_t)(lph >> 16), (uint16_t)lph, 0);
        set_ret32(cpu, 64);
        ret_pascal(cpu, 8);
    } else if (ord == 7) { /* GetFileVersionInfo(lpFile:4, dwHandle:4, dwLen:4, lpData:4) purge=16 */
        uint32_t lpdata = stk32(cpu, 0);
        if (lpdata) {
            uint16_t s = (uint16_t)(lpdata >> 16), o = (uint16_t)lpdata;
            for (uint16_t i = 0; i < 52; ++i) mem_w8(s, (uint16_t)(o + i), 0);
            mem_w32(s, o, 0xFEEF04BDu);      /* VS_FFI_SIGNATURE */
            mem_w32(s, (uint16_t)(o + 8), 0x00020000u);  /* dwFileVersionMS = 2.0 */
            mem_w32(s, (uint16_t)(o + 12), 0x00000000u); /* dwFileVersionLS */
            mem_w32(s, (uint16_t)(o + 16), 0x00020000u); /* dwProductVersionMS = 2.0 */
            mem_w32(s, (uint16_t)(o + 20), 0x00000000u); /* dwProductVersionLS */
        }
        AX = 1;
        ret_pascal(cpu, 16);
    } else if (ord == 11) { /* VerQueryValue(lpBlock:4, lpSubBlock:4, lplpBuf:4, lpuLen:4) purge=16 */
        uint32_t lpulen = stk32(cpu, 0);
        uint32_t lplp   = stk32(cpu, 4);
        uint32_t lpblk  = stk32(cpu, 12);
        if (lplp)   mem_w32((uint16_t)(lplp >> 16), (uint16_t)lplp, lpblk);
        if (lpulen) mem_w16((uint16_t)(lpulen >> 16), (uint16_t)lpulen, 52);
        AX = 1;
        ret_pascal(cpu, 16);
    }
}

/* In seg01 QT stubs, SP+0=saved DS (2), SP+2=ret IP (2), SP+4=ret CS (2), SP+6=arg0 */
static inline uint16_t qt_stk16(CpuState *cpu, uint16_t off) { return stk16(cpu, (uint16_t)(off + 6)); }
static inline uint32_t qt_stk32(CpuState *cpu, uint16_t off) { return stk32(cpu, (uint16_t)(off + 6)); }

void win16_qt_dispatch(CpuState *cpu, bool is_cmgr, uint16_t selector_bx) {
    /* Native QuickTime decoder (BIGJOB.MOV: Cinepak cvid + raw PCM) */
    if (g_debug && selector_bx != 0x05 && selector_bx != 0x0B) {
        fprintf(stderr, "[qt] %s/0x%02X SP=%04X\n", is_cmgr ? "CMgr" : "QTIM", selector_bx, SP);
    }
    if (!is_cmgr) {
        switch (selector_bx) {
        case 0x0B: case 0x0C: case 0x39: case 0x3A:
            set_ret32(cpu, 0);
            break;
        case 0x07: /* close movie */
            qt_close_movie(qt_stk32(cpu, 0));
            set_ret32(cpu, 0);
            break;
        case 0x1C: /* query movie time scale */
            set_ret32(cpu, qt_get_timescale(qt_stk32(cpu, 0)));
            break;
        case 0x29: { /* open movie(out_movie:4, hfile:2, moov_off:4, flags:2) */
            uint32_t out_lp   = qt_stk32(cpu, 0);
            int16_t  hfile    = (int16_t)qt_stk16(cpu, 4);
            uint32_t moov_off = qt_stk32(cpu, 6);
            uint32_t mh = qt_open_movie(hfile, moov_off);
            if (out_lp) mem_w32((uint16_t)(out_lp >> 16), (uint16_t)out_lp, mh);
            set_ret32(cpu, mh ? 0u : 0xFFFFFFFFu);
            break;
        }
        case 0x2F: case 0x31:
            set_ret32(cpu, 0);
            break;
        case 0x35: /* update/paint movie */
            if (g_fb && g_host_hwnd) {
                for (uint32_t c = 1; c < 8; ++c) {
                    if (qt_paint(c, g_fb, g_fb_w, g_fb_h)) {
                        HDC hdc = GetDC(g_host_hwnd);
                        present_fb_to_hdc(hdc);
                        ReleaseDC(g_host_hwnd, hdc);
                        break;
                    }
                }
            }
            set_ret32(cpu, 0);
            break;
        case 0x37: /* close controller */
            qt_close_controller(qt_stk32(cpu, 0));
            set_ret32(cpu, 0);
            break;
        case 0x38: { /* create controller(0:4, lpRect:4, 9:4, hwnd:2) */
            uint32_t lprc   = qt_stk32(cpu, 4);
            uint16_t hwnd16 = qt_stk16(cpu, 12);
            QtRect rc = { 0, 0, 0, 0 };
            if (lprc) {
                uint16_t s = (uint16_t)(lprc >> 16), o = (uint16_t)lprc;
                rc.left   = (int16_t)mem_r16(s, (uint16_t)(o + 0));
                rc.top    = (int16_t)mem_r16(s, (uint16_t)(o + 2));
                rc.right  = (int16_t)mem_r16(s, (uint16_t)(o + 4));
                rc.bottom = (int16_t)mem_r16(s, (uint16_t)(o + 6));
            }
            if (hwnd16 > 0 && hwnd16 < MAX_WIN16_WND && g_wnds[hwnd16].in_use) {
                rc.left   = (int16_t)(rc.left   + g_wnds[hwnd16].x);
                rc.top    = (int16_t)(rc.top    + g_wnds[hwnd16].y);
                rc.right  = (int16_t)(rc.right  + g_wnds[hwnd16].x);
                rc.bottom = (int16_t)(rc.bottom + g_wnds[hwnd16].y);
            }
            if (g_debug) {
                fprintf(stderr, "[qt] create_controller hwnd=%u rc=(%d,%d,%d,%d)\n",
                        hwnd16, rc.left, rc.top, rc.right, rc.bottom);
            }
            set_ret32(cpu, qt_create_controller(&rc));
            break;
        }
        default:
            set_ret32(cpu, 0xFFFFFFFFu);
            break;
        }
    } else {
        /* CMgr: 01=action, 05=get flags, 0B=msg, 13=bind, 14=query */
        uint32_t ch = qt_stk32(cpu, 0);
        switch (selector_bx) {
        case 0x01: { /* action(ctrl:4, action:2, payload:4) */
            uint16_t act = qt_stk16(cpu, 4);
            uint32_t pay = qt_stk32(cpu, 6);
            if (act == 8) {
                qt_set_rate(ch, pay);
                if (pay && g_fb && qt_tick(ch, g_fb, g_fb_w, g_fb_h) && g_host_hwnd) {
                    HDC hdc = GetDC(g_host_hwnd);
                    present_fb_to_hdc(hdc);
                    ReleaseDC(g_host_hwnd, hdc);
                }
            } else if (act == 12 && pay) {
                uint32_t tval = mem_r32((uint16_t)(pay >> 16), (uint16_t)pay);
                qt_seek(ch, tval);
            }
            set_ret32(cpu, 0);
            break;
        }
        case 0x05: { /* get flags(ctrl:4, lpOut:4) */
            if (g_fb && qt_tick(ch, g_fb, g_fb_w, g_fb_h) && g_host_hwnd) {
                HDC hdc = GetDC(g_host_hwnd);
                present_fb_to_hdc(hdc);
                ReleaseDC(g_host_hwnd, hdc);
            }
            uint32_t out_lp = qt_stk32(cpu, 4);
            if (out_lp) mem_w32((uint16_t)(out_lp >> 16), (uint16_t)out_lp, qt_is_playing(ch) ? 0x40u : 0u);
            set_ret32(cpu, 0);
            break;
        }
        case 0x0B: /* message pump */
            if (g_fb && qt_tick(ch, g_fb, g_fb_w, g_fb_h) && g_host_hwnd) {
                HDC hdc = GetDC(g_host_hwnd);
                present_fb_to_hdc(hdc);
                ReleaseDC(g_host_hwnd, hdc);
            }
            set_ret32(cpu, 0);
            break;
        case 0x13: /* bind(ctrl:4, movie:4, hwnd:2, 0:4) */
            qt_bind_controller(ch, qt_stk32(cpu, 4));
            set_ret32(cpu, 0);
            break;
        case 0x14:
        default:
            set_ret32(cpu, 0);
            break;
        }
    }
}

void win16_call_import(CpuState *cpu, const char *mod, uint16_t ordinal) {
    if (g_trace) fprintf(stderr, "[T]   import %s!%u SS:SP=%04X:%04X DS=%04X\\n", mod, ordinal, cpu->ss, cpu->esp.w, cpu->ds);
    if (strcmp(mod, "KERNEL") == 0)        handle_kernel(cpu, ordinal);
    else if (strcmp(mod, "USER") == 0)     handle_user(cpu, ordinal);
    else if (strcmp(mod, "GDI") == 0)      handle_gdi(cpu, ordinal);
    else if (strcmp(mod, "MMSYSTEM") == 0) handle_mmsystem(cpu, ordinal);
    else if (strcmp(mod, "WIN87EM") == 0)  { AX = 0; DX = 0; }
    else {
        fprintf(stderr, "[win16_api] Unknown import %s!%u\n", mod, ordinal);
        AX = 0;
    }
}

void win16_int(CpuState *cpu, uint8_t int_num) {
    static uint16_t s_dta_seg = 0;
    static uint16_t s_dta_off = 0x0080;
    if (s_dta_seg == 0) s_dta_seg = NE_DGROUP_SEL;

    if (int_num == 0x21) {
        uint8_t ah = AH;
        switch (ah) {
        case 0x1A: /* Set DTA (DS:DX) */
            s_dta_seg = cpu->ds;
            s_dta_off = DX;
            cpu->cf = 0;
            break;
        case 0x2F: /* Get DTA -> ES:BX */
            cpu->es = s_dta_seg;
            BX = s_dta_off;
            cpu->cf = 0;
            break;
        case 0x4E: { /* FindFirst (DS:DX = pattern, CX = attr) */
            char pat[260];
            win16_read_cstr(cpu->ds, DX, pat, sizeof(pat));
            if (CX & 0x08) { /* FA_LABEL: return volume label "BIGJOBCD" on drive D: (or any queried drive) */
                mem_w8(s_dta_seg, (uint16_t)(s_dta_off + 0x15), 0x08);
                win16_write_cstr(s_dta_seg, (uint16_t)(s_dta_off + 0x1E), "BIGJOBCD", 13);
                AX = 0;
                cpu->cf = 0;
            } else if (vfs_exists(pat)) {
                mem_w8(s_dta_seg, (uint16_t)(s_dta_off + 0x15), 0x20);
                win16_write_cstr(s_dta_seg, (uint16_t)(s_dta_off + 0x1E), "BIGJOB.DAT", 13);
                AX = 0;
                cpu->cf = 0;
            } else {
                AX = 18; /* ERROR_NO_MORE_FILES */
                cpu->cf = 1;
            }
            break;
        }
        case 0x30: /* Get DOS version -> 5.0 */
            AL = 5; AH = 0; BX = 0; CX = 0; cpu->cf = 0;
            break;
        case 0x4C: /* Exit process immediately (never falls through into 1:00E6) */
            cpu->halted = true;
            cpu->exit_code = AL;
            if (g_debug) fprintf(stderr, "[recomp] DOS INT 21h / AH=4Ch exit(code=%d)\n", cpu->exit_code);
            fflush(stderr);
            exit(cpu->exit_code);
            break;
        case 0x44: /* IOCTL (isatty) */
            DX = 0; cpu->cf = 0;
            break;
        case 0x42: /* lseek */
            set_ret32(cpu, (uint32_t)vfs_llseek((int16_t)BX, (int32_t)(((uint32_t)CX << 16) | DX), AL));
            cpu->cf = (DX == 0xFFFF && AX == 0xFFFF) ? 1 : 0;
            break;
        case 0x40: { /* _write */
            uint8_t *p = win16_ptr(cpu->ds, DX);
            int32_t r = p ? vfs_hwrite((int16_t)BX, p, CX) : -1;
            if (r < 0) { AX = 5; cpu->cf = 1; }
            else       { AX = (uint16_t)r; cpu->cf = 0; }
            break;
        }
        default:
            AX = 0; cpu->cf = 1;
            break;
        }
    } else if (int_num == 0x23) {
        cpu->halted = true;
        cpu->exit_code = 130;
    }
}

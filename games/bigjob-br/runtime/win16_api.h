#ifndef BIGJOB_WIN16_API_H
#define BIGJOB_WIN16_API_H

#include "cpu.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Initialize and tear down Win16 API bridge (Win32 window/GDI/waveOut/QuickTime) */
bool win16_api_init(bool headless);
void win16_api_shutdown(void);

/* Invoke a Win16 WndProc callback from host Win32 message dispatch */
uint32_t win16_invoke_wndproc(CpuState *cpu, uint16_t proc_seg, uint16_t proc_off,
                              uint16_t hwnd16, uint16_t msg16, uint16_t wparam, uint32_t lparam);

/* QuickTime 17-stub bridge & VER.DLL version check bridge */
void win16_qt_dispatch(CpuState *cpu, bool is_cmgr, uint16_t selector_bx);
void win16_ver_dispatch(CpuState *cpu, uint16_t ord);

/* Cooperative coroutine/fiber switch (seg37:1ECA start, seg37:1F13 resume, seg37:1F34 yield) */
void win16_fiber_start(CpuState *cpu);
void win16_fiber_resume(CpuState *cpu);
void win16_fiber_yield(CpuState *cpu);

#ifdef __cplusplus
}
#endif

#endif /* BIGJOB_WIN16_API_H */

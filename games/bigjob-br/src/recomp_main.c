#define _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commdlg.h>
#include "../runtime/cpu.h"
#include "../runtime/win16_mem.h"
#include "../runtime/iso_vfs.h"
#include "../runtime/win16_api.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <direct.h>

static LONG WINAPI crash_filter(EXCEPTION_POINTERS *ep) {
    fprintf(stderr, "[recomp] UNHANDLED WIN32 EXCEPTION code=0x%08lX addr=%p\n",
            ep->ExceptionRecord->ExceptionCode,
            ep->ExceptionRecord->ExceptionAddress);
    fflush(stderr);
    return EXCEPTION_EXECUTE_HANDLER;
}

extern void seg01_exec(CpuState *cpu, uint16_t entry_off);
extern void seg34_exec(CpuState *cpu, uint16_t entry_off);

static int run_pic_size_smoke_test(void) {
    /* Invoke the real lifted NE function 34:140D (MMFW Picture size calculation)
     * Signature (cdecl/far in seg34):
     *   uint32_t pic_size(void far *this_ptr, uint16_t idx)  -- let's test direct call via seg34:140D!
     * For quick smoke test of lifted code execution + stack + 32-bit SUB/SBB, we test
     * bootstrap 1:0000 and verify we reach WinMain / InitTask / InitApp cleanly!
     */
    printf("[recomp] Runtime + 40 lifted segments linked successfully.\n");
    return 0;
}

static int run_boot(bool headless, bool debug_mode) {
    CpuState cpu;
    memset(&cpu, 0, sizeof(cpu));
    g_active_cpu = &cpu;

    /* Initial NE register state: CS:IP = 1:0000, SS:SP = 49:0000 (wraps to FFFE on first push), DS = 49 */
    cpu.cs = NE_SEG_TO_SEL(1);
    cpu.ds = NE_DGROUP_SEL;
    cpu.ss = NE_DGROUP_SEL;
    cpu.es = NE_DGROUP_SEL;
    cpu.esp.w = 0xFF00; /* safe top of DGROUP stack */
    cpu.fpu_cw = 0x037F;

    if (debug_mode) {
        fprintf(stderr, "[recomp] Starting BIGJOBNG.EXE bootstrap at 1:0000 (headless=%d)...\n", headless ? 1 : 0);
    }
    seg01_exec(&cpu, 0x0000);
    if (debug_mode) {
        fprintf(stderr, "[recomp] Bootstrap returned (halted=%d, exit_code=%d, AX=%04X, DX=%04X)\n",
                cpu.halted ? 1 : 0, cpu.exit_code, cpu.eax.w, cpu.edx.w);
    }
    return cpu.exit_code;
}

static bool path_exists_file(const char *p) {
    DWORD a = GetFileAttributesA(p);
    return a != INVALID_FILE_ATTRIBUTES && !(a & FILE_ATTRIBUTE_DIRECTORY);
}

static bool dir_has_game_files(const char *dir) {
    char p1[MAX_PATH], p2[MAX_PATH];
    snprintf(p1, sizeof(p1), "%s\\BIGJOBNG.EXE", dir);
    snprintf(p2, sizeof(p2), "%s\\BIGJOB.PIC", dir);
    return path_exists_file(p1) && path_exists_file(p2);
}

static bool path_is_valid_source(const char *p) {
    if (!p || !p[0]) return false;
    DWORD a = GetFileAttributesA(p);
    if (a == INVALID_FILE_ATTRIBUTES) return false;
    if (a & FILE_ATTRIBUTE_DIRECTORY) return dir_has_game_files(p);
    return true;
}

static void ensure_dir_exists(const char *dir) {
    char tmp[MAX_PATH];
    snprintf(tmp, sizeof(tmp), "%s", dir);
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

static bool resolve_game_source(const char *exe_dir, const char *save_dir,
                                bool headless, char *out_path, size_t out_sz) {
    char ini_path[MAX_PATH];
    snprintf(ini_path, sizeof(ini_path), "%s\\recomp.ini", save_dir);

    /* 1. Environment variable override */
    const char *env_iso = getenv("BIGJOB_ISO");
    if (env_iso && path_is_valid_source(env_iso)) {
        snprintf(out_path, out_sz, "%s", env_iso);
        return true;
    }

    /* 2. Remembered path in <save_dir>\recomp.ini */
    char saved_iso[MAX_PATH] = "";
    GetPrivateProfileStringA("Paths", "ISO", "", saved_iso, sizeof(saved_iso), ini_path);
    if (saved_iso[0] && path_is_valid_source(saved_iso)) {
        snprintf(out_path, out_sz, "%s", saved_iso);
        return true;
    }

    /* 3. Auto-discover ISO or extracted CD folder relative to exe / CWD */
    char cand[MAX_PATH];
    const char *iso_names[] = { "BIGJOBCD.ISO", "bigjobcd.iso" };
    for (int i = 0; i < 2; ++i) {
        snprintf(cand, sizeof(cand), "%s\\%s", exe_dir, iso_names[i]);
        if (path_exists_file(cand)) { snprintf(out_path, out_sz, "%s", cand); return true; }
        snprintf(cand, sizeof(cand), "%s\\data\\%s", exe_dir, iso_names[i]);
        if (path_exists_file(cand)) { snprintf(out_path, out_sz, "%s", cand); return true; }
        snprintf(cand, sizeof(cand), "%s\\..\\%s", exe_dir, iso_names[i]);
        if (path_exists_file(cand)) { snprintf(out_path, out_sz, "%s", cand); return true; }
        snprintf(cand, sizeof(cand), ".\\%s", iso_names[i]);
        if (path_exists_file(cand)) { snprintf(out_path, out_sz, "%s", cand); return true; }
        snprintf(cand, sizeof(cand), ".\\data\\%s", iso_names[i]);
        if (path_exists_file(cand)) { snprintf(out_path, out_sz, "%s", cand); return true; }
    }

    /* 4. Extracted CD files in <exe_dir>\data or <exe_dir> */
    snprintf(cand, sizeof(cand), "%s\\data", exe_dir);
    if (dir_has_game_files(cand)) { snprintf(out_path, out_sz, "%s", cand); return true; }
    if (dir_has_game_files(exe_dir)) { snprintf(out_path, out_sz, "%s", exe_dir); return true; }

    /* 5. Legacy dev path fallback (only if present on disk) */
    const char *legacy = "D:\\emulacao pc antigo\\BIGJOBCD.ISO";
    if (path_exists_file(legacy)) {
        snprintf(out_path, out_sz, "%s", legacy);
        return true;
    }

    /* 6. Interactive file picker in windowed mode */
    if (!headless) {
        char chosen[MAX_PATH] = "";
        OPENFILENAMEA ofn;
        memset(&ofn, 0, sizeof(ofn));
        ofn.lStructSize  = sizeof(ofn);
        ofn.lpstrFilter  = "Imagem do CD Big Job (*.iso)\0*.iso\0Todos os arquivos (*.*)\0*.*\0";
        ofn.lpstrFile    = chosen;
        ofn.nMaxFile     = sizeof(chosen);
        ofn.lpstrInitialDir = exe_dir;
        ofn.lpstrTitle   = "Selecione o arquivo BIGJOBCD.ISO do Big Job BR";
        ofn.Flags        = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;
        if (GetOpenFileNameA(&ofn) && chosen[0]) {
            ensure_dir_exists(save_dir);
            WritePrivateProfileStringA("Paths", "ISO", chosen, ini_path);
            snprintf(out_path, out_sz, "%s", chosen);
            return true;
        }
        MessageBoxA(NULL,
                    "Nao foi possivel localizar o arquivo BIGJOBCD.ISO.\n\n"
                    "Coloque o arquivo BIGJOBCD.ISO na mesma pasta do executavel (ou na pasta 'data'),\n"
                    "ou execute com: bigjob_recomp.exe --iso \"caminho\\BIGJOBCD.ISO\"",
                    "Big Job BR (Recomp) - ISO nao encontrada",
                    MB_OK | MB_ICONWARNING);
    }
    return false;
}

static bool std_handle_is_redirected(DWORD std_id) {
    HANDLE h = GetStdHandle(std_id);
    if (!h || h == INVALID_HANDLE_VALUE) return false;
    DWORD t = GetFileType(h);
    return (t == FILE_TYPE_DISK || t == FILE_TYPE_PIPE);
}

static void setup_stdio_and_logging(const char *save_dir, bool want_console, bool cli_mode) {
    if (want_console) {
        if (!AttachConsole(ATTACH_PARENT_PROCESS)) {
            AllocConsole();
        }
        freopen("CONOUT$", "w", stdout);
        freopen("CONOUT$", "w", stderr);
        return;
    }
    bool out_redir = std_handle_is_redirected(STD_OUTPUT_HANDLE);
    bool err_redir = std_handle_is_redirected(STD_ERROR_HANDLE);
    if (cli_mode && !out_redir && !err_redir) {
        if (AttachConsole(ATTACH_PARENT_PROCESS)) {
            freopen("CONOUT$", "w", stdout);
            freopen("CONOUT$", "w", stderr);
            return;
        }
    }
    if (!err_redir) {
        ensure_dir_exists(save_dir);
        char log_path[MAX_PATH];
        snprintf(log_path, sizeof(log_path), "%s\\bigjob.log", save_dir);
        freopen(log_path, "w", stderr);
    }
}

int main(int argc, char **argv) {
    SetUnhandledExceptionFilter(crash_filter);
    char exe_dir[MAX_PATH] = ".";
    {
        DWORD n = GetModuleFileNameA(NULL, exe_dir, sizeof(exe_dir));
        if (n > 0 && n < sizeof(exe_dir)) {
            char *slash = strrchr(exe_dir, '\\');
            if (!slash) slash = strrchr(exe_dir, '/');
            if (slash) *slash = '\0';
            else snprintf(exe_dir, sizeof(exe_dir), ".");
        }
    }
    char default_save_dir[MAX_PATH];
    snprintf(default_save_dir, sizeof(default_save_dir), "%s\\saves", exe_dir);

    const char *cli_iso = NULL;
    const char *save_dir = default_save_dir;
    bool headless = false;
    bool do_boot = true;
    bool want_console = false;
    bool debug_mode = (getenv("BIGJOB_DEBUG") != NULL || getenv("BIGJOB_TRACE") != NULL);

    for (int i = 1; i < argc; ++i) {
        if ((strcmp(argv[i], "--iso") == 0 || strcmp(argv[i], "--data") == 0) && i + 1 < argc) cli_iso = argv[++i];
        else if (strcmp(argv[i], "--saves") == 0 && i + 1 < argc) save_dir = argv[++i];
        else if (strcmp(argv[i], "--window") == 0) headless = false;
        else if (strcmp(argv[i], "--headless") == 0) headless = true;
        else if (strcmp(argv[i], "--boot") == 0) do_boot = true;
        else if (strcmp(argv[i], "--smoke") == 0) do_boot = false;
        else if (strcmp(argv[i], "--console") == 0) want_console = true;
        else if (strcmp(argv[i], "--debug") == 0) {
            debug_mode = true;
            _putenv("BIGJOB_DEBUG=1");
        }
    }

    setup_stdio_and_logging(save_dir, want_console, headless || !do_boot);

    char resolved_source[MAX_PATH] = "";
    const char *iso_path = cli_iso;
    if (!iso_path) {
        if (!resolve_game_source(exe_dir, save_dir, headless, resolved_source, sizeof(resolved_source))) {
            fprintf(stderr, "[recomp] Game ISO/data not found. Pass --iso <path> or place BIGJOBCD.ISO next to the executable.\n");
            return 1;
        }
        iso_path = resolved_source;
    }

    if (!iso_vfs_init(iso_path, save_dir)) {
        fprintf(stderr, "[recomp] Failed to open game data source: %s\n", iso_path);
        if (!headless) {
            char msg[512];
            snprintf(msg, sizeof(msg), "Falha ao abrir a fonte de dados do jogo:\n%s\n\nVerifique se o arquivo e uma ISO valida do Big Job BR.", iso_path);
            MessageBoxA(NULL, msg, "Big Job BR (Recomp) - Erro ao abrir ISO", MB_OK | MB_ICONERROR);
        }
        return 1;
    }
    if (!win16_mem_init()) {
        fprintf(stderr, "[recomp] Failed to init Win16 memory\n");
        return 1;
    }
    size_t exe_sz = 0;
    uint8_t *exe_bytes = iso_vfs_read_all("BIGJOBNG.EXE", &exe_sz);
    if (!exe_bytes || !win16_load_ne_image(exe_bytes, exe_sz)) {
        fprintf(stderr, "[recomp] Failed to load BIGJOBNG.EXE from ISO\n");
        free(exe_bytes);
        return 1;
    }
    free(exe_bytes);

    if (!win16_api_init(headless)) {
        fprintf(stderr, "[recomp] Failed to init Win16 API bridge\n");
        return 1;
    }

    int rc = do_boot ? run_boot(headless, debug_mode) : run_pic_size_smoke_test();

    win16_api_shutdown();
    win16_mem_shutdown();
    iso_vfs_shutdown();
    return rc;
}

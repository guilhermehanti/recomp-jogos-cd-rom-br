#define _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

#include "../../../tools/cdlauncher/sha256.h"
#include "../../../tools/cdlauncher/iso_extract.h"
#include "../../../tools/cdlauncher/launcher_ui.h"

#define APP_TITLE L"Oficina de Inven\u00e7\u00f5es"
#define ORIG_BRINV3R_SHA256    "B7E651C590EF3FED96225DECA792A36D07BA4252A2F153B359C8164AC9FB311F"
#define PATCHED_BRINV3R_SHA256 "132E82496D4BB1EFF3AA6272F963EBB0E0D842D632BBED19D39AF2DA1A74288B"

typedef struct {
    uint32_t offset;
    uint8_t old_byte;
    uint8_t new_byte;
} BytePatch;

/* Exact 28-byte compatibility patch for Brazilian BRINV3R.EXE:
 *  - 0x007179..0x007180 & 0x0071A8..0x0071AE: force 24-bit color depth (0x18) on modern Windows
 *  - 0x00AAAA: bypass legacy CD-ROM drive check (jne -> jmp)
 *  - 0x02EF58..0x02EF63: redirect WINSPOOL.DRV import to txtdiag.dll (font antialiasing fix)
 */
static const BytePatch BRINV3R_PATCHES[] = {
    { 0x007179, 0x33, 0x6A }, { 0x00717A, 0xC0, 0x18 }, { 0x00717B, 0x66, 0x90 }, { 0x00717C, 0x8B, 0x90 },
    { 0x00717D, 0x44, 0x90 }, { 0x00717E, 0x24, 0x90 }, { 0x00717F, 0x1E, 0x90 }, { 0x007180, 0x50, 0x90 },
    { 0x0071A8, 0x33, 0xB8 }, { 0x0071A9, 0xC0, 0x18 }, { 0x0071AA, 0x66, 0x00 }, { 0x0071AB, 0x8B, 0x00 },
    { 0x0071AC, 0x44, 0x00 }, { 0x0071AD, 0x24, 0x90 }, { 0x0071AE, 0x1A, 0x90 },
    { 0x00AAAA, 0x75, 0xEB },
    { 0x02EF58, 0x57, 0x74 }, { 0x02EF59, 0x49, 0x78 }, { 0x02EF5A, 0x4E, 0x74 }, { 0x02EF5B, 0x53, 0x64 },
    { 0x02EF5C, 0x50, 0x69 }, { 0x02EF5D, 0x4F, 0x61 }, { 0x02EF5E, 0x4F, 0x67 }, { 0x02EF5F, 0x4C, 0x2E },
    { 0x02EF60, 0x2E, 0x64 }, { 0x02EF61, 0x44, 0x6C }, { 0x02EF62, 0x52, 0x6C }, { 0x02EF63, 0x56, 0x00 },
};

static bool file_exists_w(const wchar_t *p) {
    DWORD a = GetFileAttributesW(p);
    return a != INVALID_FILE_ATTRIBUTES && !(a & FILE_ATTRIBUTE_DIRECTORY);
}

static bool apply_brinv3r_patch(const wchar_t *exe_path, wchar_t *err_buf, size_t err_cap) {
    FILE *f = _wfopen(exe_path, L"r+b");
    if (!f) {
        swprintf(err_buf, err_cap, L"N\u00e3o foi poss\u00edvel abrir BRINV3R.EXE para aplicar o patch.");
        return false;
    }
    fseek(f, 0, SEEK_END);
    long sz = ftell(f);
    if (sz != 237056) {
        fclose(f);
        swprintf(err_buf, err_cap, L"Tamanho inesperado em BRINV3R.EXE (%ld bytes).", sz);
        return false;
    }
    uint8_t *buf = (uint8_t *)malloc((size_t)sz);
    if (!buf) { fclose(f); return false; }
    fseek(f, 0, SEEK_SET);
    if (fread(buf, 1, (size_t)sz, f) != (size_t)sz) {
        free(buf); fclose(f);
        swprintf(err_buf, err_cap, L"Falha ao ler BRINV3R.EXE.");
        return false;
    }

    size_t count = sizeof(BRINV3R_PATCHES) / sizeof(BRINV3R_PATCHES[0]);
    for (size_t i = 0; i < count; ++i) {
        uint32_t off = BRINV3R_PATCHES[i].offset;
        if (buf[off] != BRINV3R_PATCHES[i].old_byte) {
            free(buf); fclose(f);
            swprintf(err_buf, err_cap, L"Byte inesperado em BRINV3R.EXE (offset 0x%X).", off);
            return false;
        }
        buf[off] = BRINV3R_PATCHES[i].new_byte;
    }

    fseek(f, 0, SEEK_SET);
    bool ok = (fwrite(buf, 1, (size_t)sz, f) == (size_t)sz);
    free(buf);
    fclose(f);
    if (!ok) {
        swprintf(err_buf, err_cap, L"Falha ao salvar BRINV3R.EXE corrigido.");
        return false;
    }

    char hex[65];
    if (!sha256_file_w(exe_path, hex) || _stricmp(hex, PATCHED_BRINV3R_SHA256) != 0) {
        swprintf(err_buf, err_cap, L"Falha na verifica\u00e7\u00e3o p\u00f3s-patch de BRINV3R.EXE.");
        return false;
    }
    return true;
}

static bool write_invent_ini(const wchar_t *data_dir, wchar_t *err_buf, size_t err_cap) {
    wchar_t sys_dir[1024], u0_dir[1024], ini_path[1024];
    swprintf(sys_dir, 1024, L"%s\\system", data_dir);
    swprintf(u0_dir, 1024, L"%s\\U0", data_dir);
    CreateDirectoryW(sys_dir, NULL);
    CreateDirectoryW(u0_dir, NULL);

    swprintf(ini_path, 1024, L"%s\\invent.ini", sys_dir);
    FILE *f = _wfopen(ini_path, L"wb");
    if (!f) {
        swprintf(err_buf, err_cap, L"N\u00e3o foi poss\u00edvel criar system\\invent.ini.");
        return false;
    }
    const char *content =
        "[init]\r\n"
        "main=%.\\;!.\\;\r\n"
        "user=%.\\;\r\n"
        "minimize=0\r\n"
        "runoptions=8\r\n";
    fwrite(content, 1, strlen(content), f);
    fclose(f);
    return true;
}

static bool oficina_iso_filter(const wchar_t *rel_path, bool is_dir, void *user) {
    (void)user;
    if (_wcsnicmp(rel_path, L"VFW", 3) == 0 && (rel_path[3] == L'\0' || rel_path[3] == L'\\')) {
        return false;
    }
    if (!is_dir && _wcsicmp(rel_path, L"SETUP.EXE") == 0) {
        return false;
    }
    return true;
}

static void on_extract_progress(uint64_t done, uint64_t total, const wchar_t *status, void *user) {
    ui_progress_update((ProgressWindow *)user, done, total, status);
}

static bool deploy_patch_files(const wchar_t *exe_dir, const wchar_t *data_dir, wchar_t *err_buf, size_t err_cap) {
    wchar_t src[1024], dst[1024];

    swprintf(src, 1024, L"%s\\patch\\txtdiag.dll", exe_dir);
    swprintf(dst, 1024, L"%s\\txtdiag.dll", data_dir);
    if (!ui_copy_file_checked(src, dst, true, err_buf, err_cap)) return false;

    swprintf(src, 1024, L"%s\\patch\\winmm.dll", exe_dir);
    swprintf(dst, 1024, L"%s\\winmm.dll", data_dir);
    if (!ui_copy_file_checked(src, dst, true, err_buf, err_cap)) return false;

    swprintf(src, 1024, L"%s\\patch\\cdaudio.ini", exe_dir);
    swprintf(dst, 1024, L"%s\\cdaudio.ini", data_dir);
    if (!ui_copy_file_checked(src, dst, false, err_buf, err_cap)) return false;

    return true;
}

static bool is_game_ready(const wchar_t *data_dir) {
    wchar_t exe[1024], txtdiag[1024], winmm[1024], ini[1024];
    swprintf(exe, 1024, L"%s\\BRINV3R.EXE", data_dir);
    swprintf(txtdiag, 1024, L"%s\\txtdiag.dll", data_dir);
    swprintf(winmm, 1024, L"%s\\winmm.dll", data_dir);
    swprintf(ini, 1024, L"%s\\system\\invent.ini", data_dir);
    if (!file_exists_w(exe) || !file_exists_w(txtdiag) || !file_exists_w(winmm) || !file_exists_w(ini)) {
        return false;
    }
    char hex[65];
    return sha256_file_w(exe, hex) && _stricmp(hex, PATCHED_BRINV3R_SHA256) == 0;
}

static bool setup_from_disc(const wchar_t *exe_dir, const wchar_t *data_dir, bool forced) {
    if (!forced) {
        int r = MessageBoxW(NULL,
            L"Bem-vindo \u00e0 Oficina de Inven\u00e7\u00f5es (PT-BR)!\n\n"
            L"Na primeira execu\u00e7\u00e3o, selecione a imagem do seu CD original do jogo.\n"
            L"Dica: selecione o arquivo .CUE (com o .BIN na mesma pasta) para extrair tamb\u00e9m a m\u00fasica do CD, ou selecione um arquivo .ISO.\n\n"
            L"Atalhos no jogo:\n"
            L"  \u2022 Ctrl+I: abre o menu de tela cheia, escala e filtros gr\u00e1ficos.\n"
            L"  \u2022 Segure Shift ao abrir este execut\u00e1vel se quiser trocar a imagem do CD depois.\n\n"
            L"Clique em OK para escolher a imagem do CD.",
            APP_TITLE, MB_OKCANCEL | MB_ICONINFORMATION);
        if (r != IDOK) return false;
    }

    for (;;) {
        wchar_t picked[1024];
        const wchar_t *filter =
            L"Imagens de CD (*.cue;*.iso;*.bin)\0*.cue;*.iso;*.bin\0"
            L"Todos os arquivos (*.*)\0*.*\0";
        if (!ui_pick_cd_image(L"Selecione a imagem do CD - Oficina de Inven\u00e7\u00f5es (PT-BR)", filter, picked)) {
            return false;
        }

        wchar_t err[1024] = {0};
        CdImageInfo info;
        if (!cdimg_open(picked, &info, err, 1024)) {
            MessageBoxW(NULL, err, APP_TITLE, MB_OK | MB_ICONERROR);
            continue;
        }

        if (!cdimg_file_exists(&info, L"BRINV3R.EXE")) {
            MessageBoxW(NULL,
                L"A imagem selecionada n\u00e3o cont\u00e9m o arquivo BRINV3R.EXE.\n"
                L"Verifique se voc\u00ea escolheu o CD brasileiro de Oficina de Inven\u00e7\u00f5es.",
                APP_TITLE, MB_OK | MB_ICONWARNING);
            continue;
        }

        CreateDirectoryW(data_dir, NULL);

        wchar_t tmp_exe[1024];
        swprintf(tmp_exe, 1024, L"%s\\BRINV3R.EXE.check", data_dir);
        if (!cdimg_extract_single(&info, L"BRINV3R.EXE", tmp_exe, err, 1024)) {
            MessageBoxW(NULL, err, APP_TITLE, MB_OK | MB_ICONERROR);
            continue;
        }

        char hex[65];
        bool match = sha256_file_w(tmp_exe, hex) && _stricmp(hex, ORIG_BRINV3R_SHA256) == 0;
        DeleteFileW(tmp_exe);
        if (!match) {
            MessageBoxW(NULL,
                L"O arquivo BRINV3R.EXE encontrado nesta imagem n\u00e3o corresponde \u00e0 vers\u00e3o brasileira suportada.\n"
                L"Selecione a imagem original em Portugu\u00eas do Brasil de Oficina de Inven\u00e7\u00f5es.",
                APP_TITLE, MB_OK | MB_ICONWARNING);
            continue;
        }

        ProgressWindow pw;
        ui_progress_open(&pw, APP_TITLE, L"Preparando arquivos do CD...");

        bool ok = cdimg_extract_tree(&info, data_dir, oficina_iso_filter, NULL, NULL,
                                     on_extract_progress, &pw, err, 1024);
        if (ok && info.has_audio_tracks) {
            ok = cdimg_extract_audio(&info, data_dir, on_extract_progress, &pw, err, 1024);
        }
        if (ok) {
            ui_progress_update(&pw, 100, 100, L"Aplicando corre\u00e7\u00f5es de compatibilidade...");
            wchar_t game_exe[1024];
            swprintf(game_exe, 1024, L"%s\\BRINV3R.EXE", data_dir);
            ok = apply_brinv3r_patch(game_exe, err, 1024);
        }
        if (ok) ok = write_invent_ini(data_dir, err, 1024);
        if (ok) ok = deploy_patch_files(exe_dir, data_dir, err, 1024);

        ui_progress_close(&pw);

        if (!ok) {
            MessageBoxW(NULL, err, APP_TITLE, MB_OK | MB_ICONERROR);
            return false;
        }
        return true;
    }
}

int WINAPI wWinMain(HINSTANCE hInst, HINSTANCE hPrev, LPWSTR lpCmdLine, int nCmdShow) {
    (void)hInst; (void)hPrev; (void)nCmdShow;
    ui_init();

    wchar_t exe_dir[1024], data_dir[1024], err[1024] = {0};
    ui_get_exe_dir(exe_dir);
    swprintf(data_dir, 1024, L"%s\\oficina-data", exe_dir);

    bool force_pick = (GetAsyncKeyState(VK_SHIFT) & 0x8000) != 0 ||
                      (lpCmdLine && wcsstr(lpCmdLine, L"--trocar-iso") != NULL);

    if (force_pick || !is_game_ready(data_dir)) {
        if (!setup_from_disc(exe_dir, data_dir, force_pick)) {
            return 0;
        }
    } else {
        /* Ensure DLLs stay synced with patch\ in case of launcher update */
        deploy_patch_files(exe_dir, data_dir, err, 1024);
    }

    wchar_t game_exe[1024];
    swprintf(game_exe, 1024, L"%s\\BRINV3R.EXE", data_dir);
    if (!ui_launch_game(game_exe, data_dir, err, 1024)) {
        MessageBoxW(NULL, err, APP_TITLE, MB_OK | MB_ICONERROR);
        return 1;
    }
    return 0;
}

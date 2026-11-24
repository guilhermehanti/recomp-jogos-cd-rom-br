#define _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

#include "../../../tools/cdlauncher/sha256.h"
#include "../../../tools/cdlauncher/iso_extract.h"
#include "../../../tools/cdlauncher/is3_extract.h"
#include "../../../tools/cdlauncher/launcher_ui.h"

#define APP_TITLE L"Orly - Hist\u00f3rias para Desenhar"
#define ORLY_BR_EXE_SHA256 "562834EC09CC674C7D2CBA2E2B1AE1A002AD0F14BD88C92550E357E5B0C44666"

static bool file_exists_w(const wchar_t *p) {
    DWORD a = GetFileAttributesW(p);
    return a != INVALID_FILE_ATTRIBUTES && !(a & FILE_ATTRIBUTE_DIRECTORY);
}

static bool orly_mhk_filter(const wchar_t *rel_path, bool is_dir, void *user) {
    (void)user;
    if (is_dir) return false;
    if (_wcsnicmp(rel_path, L"ORLY\\", 5) != 0) return false;
    const wchar_t *dot = wcsrchr(rel_path, L'.');
    return dot && _wcsicmp(dot, L".MHK") == 0;
}

typedef struct {
    ProgressWindow *pw;
    uint64_t base_done;
    uint64_t scale_max;
    uint64_t grand_total;
} ScaledProgress;

static void on_scaled_progress(uint64_t done, uint64_t total, const wchar_t *status, void *user) {
    ScaledProgress *sp = (ScaledProgress *)user;
    uint64_t mapped = sp->base_done + (total > 0 ? (done * sp->scale_max) / total : 0);
    ui_progress_update(sp->pw, mapped, sp->grand_total, status);
}

static bool deploy_patch_files(const wchar_t *exe_dir, const wchar_t *data_dir, wchar_t *err_buf, size_t err_cap) {
    wchar_t src[1024], dst[1024];

    swprintf(src, 1024, L"%s\\patch\\winspool.dll", exe_dir);
    swprintf(dst, 1024, L"%s\\winspool.dll", data_dir);
    if (!ui_copy_file_checked(src, dst, true, err_buf, err_cap)) return false;

    swprintf(src, 1024, L"%s\\patch\\orlyfix.ini", exe_dir);
    swprintf(dst, 1024, L"%s\\orlyfix.ini", data_dir);
    if (!ui_copy_file_checked(src, dst, false, err_buf, err_cap)) return false;

    return true;
}

static bool is_game_ready(const wchar_t *data_dir) {
    wchar_t exe[1024], dll[1024], pict[1024];
    swprintf(exe, 1024, L"%s\\Orly32.exe", data_dir);
    swprintf(dll, 1024, L"%s\\winspool.dll", data_dir);
    swprintf(pict, 1024, L"%s\\PICT.MHK", data_dir);
    if (!file_exists_w(exe) || !file_exists_w(dll) || !file_exists_w(pict)) {
        return false;
    }
    char hex[65];
    return sha256_file_w(exe, hex) && _stricmp(hex, ORLY_BR_EXE_SHA256) == 0;
}

static bool setup_from_disc(const wchar_t *exe_dir, const wchar_t *data_dir, bool forced) {
    if (!forced) {
        int r = MessageBoxW(NULL,
            L"Bem-vindo ao Orly: Hist\u00f3rias para Desenhar (PT-BR)!\n\n"
            L"Na primeira execu\u00e7\u00e3o, selecione a imagem (.ISO, .CUE ou .BIN) do seu CD original brasileiro.\n"
            L"Os arquivos do jogo ser\u00e3o extra\u00eddos automaticamente na pasta 'orly-data'.\n\n"
            L"Atalhos no jogo:\n"
            L"  \u2022 Alt+Enter: alterna entre janela e tela cheia.\n"
            L"  \u2022 F1 a F4: escolhe o filtro gr\u00e1fico (pixel, bilinear, bic\u00fabico, sharp bilinear).\n"
            L"  \u2022 Segure Shift ao abrir este execut\u00e1vel se quiser trocar a imagem do CD depois.\n\n"
            L"Clique em OK para escolher a imagem do CD.",
            APP_TITLE, MB_OKCANCEL | MB_ICONINFORMATION);
        if (r != IDOK) return false;
    }

    for (;;) {
        wchar_t picked[1024];
        const wchar_t *filter =
            L"Imagens de CD (*.iso;*.cue;*.bin)\0*.iso;*.cue;*.bin\0"
            L"Todos os arquivos (*.*)\0*.*\0";
        if (!ui_pick_cd_image(L"Selecione a imagem do CD - Orly (PT-BR)", filter, picked)) {
            return false;
        }

        wchar_t err[1024] = {0};
        CdImageInfo info;
        if (!cdimg_open(picked, &info, err, 1024)) {
            MessageBoxW(NULL, err, APP_TITLE, MB_OK | MB_ICONERROR);
            continue;
        }

        if (!cdimg_file_exists(&info, L"ARCHIVE\\ARCHIVE.Z")) {
            MessageBoxW(NULL,
                L"A imagem selecionada n\u00e3o cont\u00e9m ARCHIVE\\ARCHIVE.Z.\n"
                L"Verifique se voc\u00ea selecionou o CD original de Orly (PT-BR).",
                APP_TITLE, MB_OK | MB_ICONWARNING);
            continue;
        }

        CreateDirectoryW(data_dir, NULL);

        ProgressWindow pw;
        ui_progress_open(&pw, APP_TITLE, L"Extraindo ARCHIVE.Z do CD...");

        wchar_t tmp_arc[1024];
        swprintf(tmp_arc, 1024, L"%s\\ARCHIVE.Z.tmp", data_dir);

        bool ok = cdimg_extract_single(&info, L"ARCHIVE\\ARCHIVE.Z", tmp_arc, err, 1024);
        if (ok) {
            ScaledProgress sp = { &pw, 0, 150, 1000 };
            ok = is3_extract_archive(tmp_arc, data_dir, on_scaled_progress, &sp, err, 1024);
        }
        DeleteFileW(tmp_arc);

        if (ok) {
            wchar_t exe_path[1024];
            swprintf(exe_path, 1024, L"%s\\Orly32.exe", data_dir);
            char hex[65];
            if (!sha256_file_w(exe_path, hex) || _stricmp(hex, ORLY_BR_EXE_SHA256) != 0) {
                ui_progress_close(&pw);
                DeleteFileW(exe_path);
                MessageBoxW(NULL,
                    L"O execut\u00e1vel Orly32.exe deste CD n\u00e3o corresponde \u00e0 vers\u00e3o brasileira suportada.\n"
                    L"Verifique se a imagem \u00e9 a edi\u00e7\u00e3o em Portugu\u00eas do Brasil (Orly: Hist\u00f3rias para Desenhar).",
                    APP_TITLE, MB_OK | MB_ICONWARNING);
                continue;
            }
        }

        /* Extract ORLY\*.MHK directly into data_dir, overwriting the 4-byte SPR_*.MHK stubs */
        if (ok) {
            ScaledProgress sp = { &pw, 150, 850, 1000 };
            ok = cdimg_extract_tree(&info, data_dir, orly_mhk_filter, NULL, L"ORLY",
                                    on_scaled_progress, &sp, err, 1024);
        }

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
    swprintf(data_dir, 1024, L"%s\\orly-data", exe_dir);

    bool force_pick = (GetAsyncKeyState(VK_SHIFT) & 0x8000) != 0 ||
                      (lpCmdLine && wcsstr(lpCmdLine, L"--trocar-iso") != NULL);

    if (force_pick || !is_game_ready(data_dir)) {
        if (!setup_from_disc(exe_dir, data_dir, force_pick)) {
            return 0;
        }
    } else {
        deploy_patch_files(exe_dir, data_dir, err, 1024);
    }

    wchar_t game_exe[1024];
    swprintf(game_exe, 1024, L"%s\\Orly32.exe", data_dir);
    if (!ui_launch_game(game_exe, data_dir, err, 1024)) {
        MessageBoxW(NULL, err, APP_TITLE, MB_OK | MB_ICONERROR);
        return 1;
    }
    return 0;
}

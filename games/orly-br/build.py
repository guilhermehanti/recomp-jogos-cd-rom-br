"""Build Orly PT-BR winspool.dll (x86, -DORLY_BR) + launcher (x64) using clang-cl + lld."""
import os
import shutil
import subprocess
from pathlib import Path

ROOT = Path(__file__).resolve().parent
REPO_ROOT = ROOT.parents[1]
COMMON = REPO_ROOT / "tools" / "cdlauncher"
SRC = ROOT / "src"
BUILD = ROOT / "build"
DIST = ROOT / "dist" / "Orly - Historias para Desenhar"


def find_vcvars(arch: str) -> str:
    bat = "vcvars64.bat" if arch == "x64" else "vcvars32.bat"
    vswhere = Path(r"C:\Program Files (x86)\Microsoft Visual Studio\Installer\vswhere.exe")
    if vswhere.exists():
        inst = subprocess.check_output(
            [str(vswhere), "-latest", "-products", "*", "-requires",
             "Microsoft.VisualStudio.Component.VC.Tools.x86.x64", "-property", "installationPath"],
            text=True,
        ).strip()
        if inst:
            cand = Path(inst) / "VC" / "Auxiliary" / "Build" / bat
            if cand.exists():
                return str(cand)
    for ed in ("Community", "Professional", "Enterprise", "BuildTools"):
        cand = Path(rf"C:\Program Files\Microsoft Visual Studio\2022\{ed}\VC\Auxiliary\Build\{bat}")
        if cand.exists():
            return str(cand)
    raise FileNotFoundError(f"Could not find {bat}")


def get_msvc_env(arch: str) -> dict:
    vcvars = find_vcvars(arch)
    out = subprocess.check_output(f'"{vcvars}" >nul && set', shell=True, text=True)
    env = os.environ.copy()
    for line in out.splitlines():
        if "=" in line:
            k, v = line.split("=", 1)
            env[k] = v
    return env


def build_winspool(env32: dict):
    BUILD.mkdir(parents=True, exist_ok=True)
    out_dll = BUILD / "winspool.dll"
    cmd = [
        "clang-cl", "--target=i686-pc-windows-msvc", "/nologo", "/TC", "/O2", "/MT", "/LD",
        "/DORLY_BR", "/clang:-mstackrealign", "/clang:-Wno-incompatible-function-pointer-types",
        str(SRC / "orlyfix.c"),
        "-fuse-ld=lld", "/link",
        f"/def:{SRC / 'winspool-msvc.def'}",
        f"/out:{out_dll}",
        "kernel32.lib", "user32.lib", "gdi32.lib", "opengl32.lib",
    ]
    subprocess.check_call(cmd, env=env32, cwd=str(BUILD))
    return out_dll


def build_launcher(env64: dict):
    BUILD.mkdir(parents=True, exist_ok=True)
    rc_path = SRC / "orly.rc"
    res_path = BUILD / "orly.res"
    search_path = env64.get("PATH", "") + os.pathsep + env64.get("Path", "")
    rc_exe = shutil.which("rc.exe", path=search_path) or shutil.which("llvm-rc.exe", path=search_path)
    if rc_exe:
        cmd = [rc_exe, "/fo", str(res_path), str(rc_path)] if "llvm-rc" in Path(rc_exe).name.lower() else [
            rc_exe, "/nologo", f"/fo{res_path}", str(rc_path)
        ]
        subprocess.check_call(cmd, env=env64, cwd=str(SRC))
    else:
        res_path = None

    out_exe = BUILD / "Orly - Historias para Desenhar.exe"
    cmd = [
        "clang-cl", "--target=x86_64-pc-windows-msvc", "/nologo", "/O2", "/MT", "/W2",
        str(COMMON / "sha256.c"),
        str(COMMON / "iso_extract.c"),
        str(COMMON / "is3_extract.c"),
        str(COMMON / "launcher_ui.c"),
        str(SRC / "orly_launcher.c"),
        "user32.lib", "gdi32.lib", "comdlg32.lib", "comctl32.lib", "shell32.lib",
        "-fuse-ld=lld", f"/Fe:{out_exe}",
        "/link", "/SUBSYSTEM:WINDOWS",
        *([str(res_path)] if res_path else []),
    ]
    subprocess.check_call(cmd, env=env64, cwd=str(BUILD))
    return out_exe


def stage_dist(exe: Path, dll: Path):
    patch_dir = DIST / "patch"
    lic_dir = DIST / "LICENSES"
    patch_dir.mkdir(parents=True, exist_ok=True)
    lic_dir.mkdir(parents=True, exist_ok=True)

    shutil.copy2(exe, DIST / "Orly - Historias para Desenhar.exe")
    shutil.copy2(dll, patch_dir / "winspool.dll")
    shutil.copy2(ROOT / "orlyfix.ini", patch_dir / "orlyfix.ini")

    cc0_lic = REPO_ROOT / "LICENSES" / "CC0-1.0.txt"
    if cc0_lic.exists():
        shutil.copy2(cc0_lic, lic_dir / "CC0-1.0.txt")

    (DIST / "LEIA-ME.txt").write_text(
        "Orly: Historias para Desenhar (PT-BR) - Edicao Moderna para Windows 10/11\n"
        "==========================================================================\n\n"
        "COMO JOGAR:\n"
        "1. Abra 'Orly - Historias para Desenhar.exe'.\n"
        "2. Na primeira vez, selecione a imagem (.ISO, .CUE ou .BIN) do seu CD original brasileiro.\n"
        "3. O jogo sera extraido automaticamente na pasta 'orly-data' e aberto em seguida.\n"
        "4. Nas proximas vezes, basta abrir 'Orly - Historias para Desenhar.exe' diretamente.\n\n"
        "ATALHOS NO JOGO:\n"
        "- Alt+Enter: alterna entre janela e tela cheia.\n"
        "- F1: pixels nitidos | F2: bilinear | F3: bicubico | F4: sharp bilinear.\n"
        "- Segure Shift ao abrir o executavel para selecionar outra imagem de CD.\n\n"
        "CREDITOS:\n"
        "- Patches base (orlyfix): Matthew Bloch - matthewbloch/orlys-draw-a-story-fixes (CC0).\n"
        "- Adaptacao para a versao brasileira (ORLY_BR) e launcher/extrator nativo de ISO e InstallShield 3.0.\n"
        "- Nenhum arquivo original do jogo esta incluido neste pacote.\n",
        encoding="utf-8",
    )
    print(f"Staged distribution in {DIST}")


def main():
    env32 = get_msvc_env("x86")
    env64 = get_msvc_env("x64")
    dll = build_winspool(env32)
    exe = build_launcher(env64)
    stage_dist(exe, dll)


if __name__ == "__main__":
    main()

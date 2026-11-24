"""Build Oficina de Invencoes launcher (x64) + txtdiag.dll (x86) using clang-cl + lld."""
import os
import shutil
import subprocess
from pathlib import Path

ROOT = Path(__file__).resolve().parent
REPO_ROOT = ROOT.parents[1]
COMMON = REPO_ROOT / "tools" / "cdlauncher"
SRC = ROOT / "src"
BUILD = ROOT / "build"
DIST = ROOT / "dist" / "Oficina de Invencoes"


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


def build_txtdiag(env32: dict):
    BUILD.mkdir(parents=True, exist_ok=True)
    out_dll = BUILD / "txtdiag.dll"
    cmd = [
        "clang-cl", "--target=i686-pc-windows-msvc", "/nologo", "/TC", "/O2", "/MT", "/LD",
        str(SRC / "font-fix.c"),
        "-fuse-ld=lld", "/link",
        f"/def:{SRC / 'trace-text.def'}",
        f"/out:{out_dll}",
        "kernel32.lib", "user32.lib", "gdi32.lib", "winspool.lib",
    ]
    subprocess.check_call(cmd, env=env32, cwd=str(BUILD))
    return out_dll


def build_launcher(env64: dict):
    BUILD.mkdir(parents=True, exist_ok=True)
    rc_path = SRC / "oficina.rc"
    res_path = BUILD / "oficina.res"
    search_path = env64.get("PATH", "") + os.pathsep + env64.get("Path", "")
    rc_exe = shutil.which("rc.exe", path=search_path) or shutil.which("llvm-rc.exe", path=search_path)
    if rc_exe:
        cmd = [rc_exe, "/fo", str(res_path), str(rc_path)] if "llvm-rc" in Path(rc_exe).name.lower() else [
            rc_exe, "/nologo", f"/fo{res_path}", str(rc_path)
        ]
        subprocess.check_call(cmd, env=env64, cwd=str(SRC))
    else:
        res_path = None

    out_exe = BUILD / "Oficina de Invencoes.exe"
    cmd = [
        "clang-cl", "--target=x86_64-pc-windows-msvc", "/nologo", "/O2", "/MT", "/W2",
        str(COMMON / "sha256.c"),
        str(COMMON / "iso_extract.c"),
        str(COMMON / "launcher_ui.c"),
        str(SRC / "oficina_launcher.c"),
        "user32.lib", "gdi32.lib", "comdlg32.lib", "comctl32.lib", "shell32.lib",
        "-fuse-ld=lld", f"/Fe:{out_exe}",
        "/link", "/SUBSYSTEM:WINDOWS",
        *([str(res_path)] if res_path else []),
    ]
    subprocess.check_call(cmd, env=env64, cwd=str(BUILD))
    return out_exe


def stage_dist(exe: Path, txtdiag: Path):
    patch_dir = DIST / "patch"
    lic_dir = DIST / "LICENSES"
    patch_dir.mkdir(parents=True, exist_ok=True)
    lic_dir.mkdir(parents=True, exist_ok=True)

    shutil.copy2(exe, DIST / "Oficina de Invencoes.exe")
    shutil.copy2(txtdiag, patch_dir / "txtdiag.dll")
    shutil.copy2(ROOT / "cdaudio.ini", patch_dir / "cdaudio.ini")

    winmm_candidates = [
        ROOT / "prebuilt" / "winmm.dll",
        Path(r"D:\emulacao pc antigo\oficina-br\game\winmm.dll"),
    ]
    for cand in winmm_candidates:
        if cand.exists():
            shutil.copy2(cand, patch_dir / "winmm.dll")
            break

    mit_lic = REPO_ROOT / "LICENSES" / "MIT-invention-studio-rescue.txt"
    if mit_lic.exists():
        shutil.copy2(mit_lic, lic_dir / "MIT-invention-studio-rescue.txt")

    (DIST / "LEIA-ME.txt").write_text(
        "Oficina de Invencoes (PT-BR) - Edicao Moderna para Windows 10/11\n"
        "=================================================================\n\n"
        "COMO JOGAR:\n"
        "1. Abra 'Oficina de Invencoes.exe'.\n"
        "2. Na primeira vez, selecione a imagem do seu CD original brasileiro:\n"
        "   - Preferencialmente o arquivo .CUE (com o .BIN na mesma pasta) para ter a musica do CD.\n"
        "   - Tambem aceita arquivo .ISO.\n"
        "3. O jogo sera preparado automaticamente na pasta 'oficina-data' e aberto em seguida.\n"
        "4. Nas proximas vezes, basta abrir 'Oficina de Invencoes.exe' diretamente.\n\n"
        "ATALHOS:\n"
        "- Ctrl+I: abre o menu de configuracoes de tela cheia, escala e filtros.\n"
        "- Segure Shift ao abrir o executavel para selecionar outra imagem de CD.\n\n"
        "CREDITOS:\n"
        "- Wrapper CD Audio e escala HD (winmm.dll): leo-dragons/invention-studio-rescue (Licenca MIT).\n"
        "- Adaptacao PT-BR, correcao de fontes (txtdiag.dll), patch de 28 bytes e launcher ISO/CUE.\n"
        "- Nenhum arquivo original do jogo esta incluido neste pacote.\n",
        encoding="utf-8",
    )
    print(f"Staged distribution in {DIST}")


def main():
    env32 = get_msvc_env("x86")
    env64 = get_msvc_env("x64")
    txtdiag = build_txtdiag(env32)
    exe = build_launcher(env64)
    stage_dist(exe, txtdiag)


if __name__ == "__main__":
    main()

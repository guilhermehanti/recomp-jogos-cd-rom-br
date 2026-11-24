"""Incremental parallel builder for bigjob_recomp.exe using clang-cl + lld."""
import os
import shutil
import struct
import subprocess
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
BUILD = ROOT / "build"
OBJ_DIR = BUILD / "recomp_obj"
GEN_DIR = BUILD / "recomp_gen"
VCVARS_CANDIDATES = [
    r"C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat",
    r"C:\Program Files\Microsoft Visual Studio\2022\Professional\VC\Auxiliary\Build\vcvars64.bat",
    r"C:\Program Files\Microsoft Visual Studio\2022\Enterprise\VC\Auxiliary\Build\vcvars64.bat",
    r"C:\Program Files\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat",
    r"C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat",
    r"C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools\VC\Auxiliary\Build\vcvars64.bat",
]


def find_vcvars():
    vswhere = Path(r"C:\Program Files (x86)\Microsoft Visual Studio\Installer\vswhere.exe")
    if vswhere.exists():
        try:
            inst = subprocess.check_output(
                [str(vswhere), "-latest", "-products", "*", "-requires",
                 "Microsoft.VisualStudio.Component.VC.Tools.x86.x64", "-property", "installationPath"],
                text=True,
            ).strip()
            if inst:
                cand = Path(inst) / "VC" / "Auxiliary" / "Build" / "vcvars64.bat"
                if cand.exists():
                    return str(cand)
        except Exception:
            pass
    for c in VCVARS_CANDIDATES:
        if Path(c).exists():
            return c
    raise FileNotFoundError("Could not locate vcvars64.bat (Visual Studio C++ Build Tools)")


def get_msvc_env():
    vcvars = find_vcvars()
    cmd = f'"{vcvars}" >nul && set'
    out = subprocess.check_output(cmd, shell=True, text=True)
    env = os.environ.copy()
    for line in out.splitlines():
        if "=" in line:
            k, v = line.split("=", 1)
            env[k] = v
    return env


def extract_ne_icon(ne_exe: Path, out_ico: Path) -> bool:
    if not ne_exe.exists():
        return False
    data = ne_exe.read_bytes()
    if len(data) < 0x40:
        return False
    ne_off = struct.unpack_from("<I", data, 0x3C)[0]
    if ne_off + 0x26 > len(data) or data[ne_off:ne_off + 2] != b"NE":
        return False
    rsrc_off = ne_off + struct.unpack_from("<H", data, ne_off + 0x24)[0]
    shift = struct.unpack_from("<H", data, rsrc_off)[0]
    pos = rsrc_off + 2
    icons = {}
    groups = {}
    while pos + 8 <= len(data):
        rtype = struct.unpack_from("<H", data, pos)[0]
        if rtype == 0:
            break
        count = struct.unpack_from("<H", data, pos + 2)[0]
        pos += 8
        rt_id = rtype & 0x7FFF
        for _ in range(count):
            off, length, _flags, rid, _h, _u = struct.unpack_from("<HHHHHH", data, pos)
            pos += 12
            blob = data[off << shift : (off << shift) + (length << shift)]
            if rt_id == 3:
                icons[rid & 0x7FFF] = blob
            elif rt_id == 14:
                groups[rid & 0x7FFF] = blob
    if 1 not in groups:
        return False
    grp = groups[1]
    _res, _type, res_count = struct.unpack_from("<HHH", grp, 0)
    entries = []
    for i in range(res_count):
        w, h, colors, _r, planes, bpp, sz, icon_id = struct.unpack_from("<BBBBHHIH", grp, 6 + i * 14)
        if icon_id in icons:
            entries.append((w, h, colors, planes, bpp, icons[icon_id][:sz]))
    if not entries:
        return False
    hdr = struct.pack("<HHH", 0, 1, len(entries))
    dir_bytes = b""
    img_bytes = b""
    cur_off = 6 + 16 * len(entries)
    for w, h, colors, planes, bpp, blob in entries:
        dir_bytes += struct.pack("<BBBBHHII", w, h, colors, 0, planes, bpp, len(blob), cur_off)
        img_bytes += blob
        cur_off += len(blob)
    out_ico.write_bytes(hdr + dir_bytes + img_bytes)
    return True


def build_icon_resource(env):
    ico = BUILD / "bigjob.ico"
    rc  = OBJ_DIR / "bigjob.rc"
    res = OBJ_DIR / "bigjob.res"
    if not ico.exists():
        if not extract_ne_icon(BUILD / "BIGJOBNG.EXE", ico):
            return None
    rc_text = f'1 ICON "{ico.as_posix()}"\n'
    if not rc.exists() or rc.read_text(encoding="utf8") != rc_text:
        rc.write_text(rc_text, encoding="utf8")
    if not res.exists() or res.stat().st_mtime < max(ico.stat().st_mtime, rc.stat().st_mtime):
        search_path = env.get("PATH", "") + os.pathsep + env.get("Path", "")
        rc_exe = shutil.which("rc.exe", path=search_path) or shutil.which("llvm-rc.exe", path=search_path)
        if not rc_exe:
            print("Warning: rc.exe/llvm-rc.exe not found; skipping embedded icon.")
            return None
        cmd = [rc_exe, "/fo", str(res), str(rc)] if "llvm-rc" in Path(rc_exe).name.lower() else [rc_exe, "/nologo", f"/fo{res}", str(rc)]
        subprocess.check_call(cmd, env=env)
    return res


def main():
    OBJ_DIR.mkdir(parents=True, exist_ok=True)
    headers = list((ROOT / "runtime").glob("*.h"))
    hdr_mtime = max(h.stat().st_mtime for h in headers)

    sources = [
        ROOT / "runtime" / "win16_mem.c",
        ROOT / "runtime" / "iso_vfs.c",
        ROOT / "runtime" / "qt_movie.c",
        ROOT / "runtime" / "video_filter.c",
        ROOT / "runtime" / "win16_api.c",
        ROOT / "src" / "recomp_main.c",
        GEN_DIR / "dispatch_table.c",
    ] + sorted(GEN_DIR.glob("seg*.c"))

    to_compile = []
    objs = []
    for src in sources:
        obj = OBJ_DIR / (src.stem + ".obj")
        objs.append(obj)
        src_mtime = max(src.stat().st_mtime, hdr_mtime)
        if not obj.exists() or obj.stat().st_mtime < src_mtime:
            to_compile.append((src, obj))

    env = get_msvc_env()
    if to_compile:
        print(f"Compiling {len(to_compile)}/{len(sources)} changed source files in parallel...")
        def compile_one(pair):
            src, obj = pair
            cmd = [
                "clang-cl", "/c", "/O1", "/MT", "/W2",
                "/wd4102", "/wd4101", "/wd4244", "/wd4146",
                str(src), f"/Fo:{obj}"
            ]
            subprocess.check_call(cmd, env=env)
        with ThreadPoolExecutor(max_workers=min(8, os.cpu_count() or 4)) as ex:
            list(ex.map(compile_one, to_compile))
    else:
        print("All object files up to date.")

    res = build_icon_resource(env)
    exe = BUILD / "bigjob_recomp.exe"
    link_cmd = [
        "clang-cl", "/O1", "/MT",
        *[str(o) for o in objs],
        *([str(res)] if res else []),
        "user32.lib", "gdi32.lib", "winmm.lib", "comdlg32.lib",
        "-fuse-ld=lld", f"/Fe:{exe}",
        "/link", "/SUBSYSTEM:WINDOWS", "/ENTRY:mainCRTStartup",
    ]
    subprocess.check_call(link_cmd, env=env)
    print(f"Linked {exe}")


if __name__ == "__main__":
    main()

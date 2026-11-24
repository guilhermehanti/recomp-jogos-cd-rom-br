"""Map Borland CRT INTs, Borland key-value CS switch tables, 386 ISA subset, WinExec, static _INIT_/_EXIT_, and LoadCursor."""
import json
import re
import struct
from collections import Counter
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
ANALYSIS = ROOT / "analysis"
DISASM = ANALYSIS / "disassembly"
EXE_PATH = ROOT / "build" / "BIGJOBNG.EXE"

LINE_RE = re.compile(r"^\s*([0-9a-f]+):\s+((?:[0-9a-f]{2} )+)\s*(.*)$")


def load_seg(seg_id):
    out = []
    for line in (DISASM / f"seg{seg_id:02d}.asm").read_text(encoding="utf8").splitlines():
        m = LINE_RE.match(line)
        if m:
            out.append((int(m[1], 16), m[2].strip(), m[3].strip()))
    return out


def main():
    inv = json.loads((ANALYSIS / "inventory.json").read_text(encoding="utf8"))
    ne = inv["NE"]["BIGJOBNG.EXE"]
    segs = {s["id"]: s for s in ne["segments"]}
    exe = EXE_PATH.read_bytes()
    refs = json.loads((DISASM / "references.json").read_text(encoding="utf8"))
    cfg = json.loads((ANALYSIS / "recomp-cfg.json").read_text(encoding="utf8"))

    seg_cache = {}
    def get_seg(sid):
        if sid not in seg_cache:
            seg_cache[sid] = load_seg(sid)
        return seg_cache[sid]

    # 1. All INT instructions in seg01..40 with context window
    int_sites = []
    for sid in range(1, 41):
        insts = get_seg(sid)
        for idx, (off, raw, mnem) in enumerate(insts):
            clean = mnem.split(";")[0].strip()
            if re.match(r"^int\b", clean):
                win = [f"{insts[j][0]:04X}: {insts[j][2]}" for j in range(max(0, idx - 6), min(len(insts), idx + 3))]
                int_sites.append({"segment": sid, "offset": f"0x{off:04X}", "inst": clean, "window": win})

    # 2. All 14 jump tables that had bound=None
    jt_details = []
    for jt in cfg["jump_tables"]:
        if jt["bound_from_cmp"] is not None and jt["entries_count"] > 0:
            continue
        sid = jt["segment"]
        site_off = int(jt["site"], 16)
        insts = get_seg(sid)
        s_meta = segs[sid]
        seg_bytes = exe[s_meta["offset"]:s_meta["offset"] + s_meta["size"]]
        win = []
        for idx, (off, raw, mnem) in enumerate(insts):
            if off == site_off:
                win = [f"{insts[j][0]:04X}: {insts[j][2]}" for j in range(max(0, idx - 12), min(len(insts), idx + 5))]
                break
        # Check Borland key-value switch idiom:
        #   movw $count, %cx; movw $tbl_off, %bx; loop: cmpw %cs:(%bx), %ax; je match; ...
        #   match: jmpw *%cs:<2*count>(%bx)
        tbl_base = None
        count = None
        keys = []
        targets = []
        disp = int(jt["table_offset"], 16)
        for line in win:
            m_bx = re.search(r"movw\s+\$0x([0-9a-f]+),\s*%bx", line)
            if m_bx:
                tbl_base = int(m_bx[1], 16)
            m_cx = re.search(r"movw\s+\$0x([0-9a-f]+),\s*%cx", line)
            if m_cx:
                count = int(m_cx[1], 16)
        if tbl_base is not None and count is not None and disp == 2 * count:
            for k in range(count):
                key = struct.unpack_from("<H", seg_bytes, tbl_base + 2 * k)[0]
                tgt = struct.unpack_from("<H", seg_bytes, tbl_base + disp + 2 * k)[0]
                keys.append(key)
                targets.append(f"0x{tgt:04X}")
        jt_details.append({
            "segment": sid,
            "site": jt["site"],
            "disp": jt["table_offset"],
            "detected_base": f"0x{tbl_base:04X}" if tbl_base is not None else None,
            "detected_count": count,
            "keys_hex": [f"0x{k:04X}" for k in keys],
            "targets": targets,
            "window": win,
        })

    # 3. ISA subset across all 40 code segments (unique mnemonics, 32-bit ops, STD sites)
    mnem_counts = Counter()
    std_sites = []
    for sid in range(1, 41):
        insts = get_seg(sid)
        for idx, (off, raw, mnem) in enumerate(insts):
            clean = mnem.split(";")[0].strip()
            op = clean.split()[0] if clean else ""
            if op:
                mnem_counts[op] += 1
            if op == "std":
                win = [f"{insts[j][0]:04X}: {insts[j][2]}" for j in range(max(0, idx - 3), min(len(insts), idx + 6))]
                std_sites.append({"segment": sid, "offset": f"0x{off:04X}", "window": win})

    # 4. WinExec (KERNEL!166) at 22:055C
    winexec_sites = []
    for r in refs:
        if r["target"] == "KERNEL!166":
            sid = r["segment"]
            insts = get_seg(sid)
            for idx, (off, raw, mnem) in enumerate(insts):
                nbytes = len(raw.split())
                if off <= r["site"] < off + nbytes:
                    win = [f"{insts[j][0]:04X}: {insts[j][2]}" for j in range(max(0, idx - 18), min(len(insts), idx + 8))]
                    winexec_sites.append({"segment": sid, "site": f"0x{r['site']:04X}", "window": win})
                    break

    # 5. Static _INIT_/_EXIT_ tables in DGROUP (seg49): look at seg01 bootstrap 1:0075..1:0140 and DGROUP relocs
    seg1 = get_seg(1)
    boot_tail = [f"{off:04X}: {mnem}" for off, _, mnem in seg1 if 0x0075 <= off <= 0x0160]
    dgroup_code_relocs = sorted(
        (r["site"], r["target"])
        for r in refs
        if r["segment"] == 49 and ":" in r["target"] and int(r["target"].split(":")[0]) <= 40
    )
    # Borland _INIT_/_EXIT_ entries are 6 bytes each: (byte type, byte priority, far ptr)
    dgroup_meta = segs[49]
    dgroup_bytes = exe[dgroup_meta["offset"]:dgroup_meta["offset"] + dgroup_meta["size"]]
    init_candidates = []
    for site, tgt in dgroup_code_relocs:
        if site >= 2:
            b_type, b_prio = dgroup_bytes[site - 2], dgroup_bytes[site - 1]
            init_candidates.append({
                "dgroup_offset": f"0x{site - 2:04X}",
                "ptr_offset": f"0x{site:04X}",
                "type_byte": b_type,
                "priority_byte": b_prio,
                "target": tgt,
            })

    # 6. LoadCursor (USER!173) all 7 sites
    loadcursor_sites = []
    for r in refs:
        if r["target"] == "USER!173":
            sid = r["segment"]
            insts = get_seg(sid)
            for idx, (off, raw, mnem) in enumerate(insts):
                nbytes = len(raw.split())
                if off <= r["site"] < off + nbytes:
                    win = [f"{insts[j][0]:04X}: {insts[j][2]}" for j in range(max(0, idx - 8), min(len(insts), idx + 3))]
                    loadcursor_sites.append({"segment": sid, "site": f"0x{r['site']:04X}", "window": win})
                    break

    out = {
        "int_sites": int_sites,
        "unresolved_jump_tables_decoded": jt_details,
        "unique_mnemonics_count": len(mnem_counts),
        "mnemonics_top_40": mnem_counts.most_common(40),
        "all_mnemonics": dict(mnem_counts.most_common()),
        "std_sites": std_sites,
        "winexec_sites": winexec_sites,
        "bootstrap_tail_0075_0160": boot_tail,
        "dgroup_code_relocs": init_candidates,
        "loadcursor_sites": loadcursor_sites,
    }
    (ANALYSIS / "crt-switch-isa.json").write_text(json.dumps(out, indent=2), encoding="utf8")

    print("=== 1. INT SITES ===")
    for s in int_sites:
        print(f"  {s['segment']}:{s['offset']} {s['inst']} | prev: {s['window'][-4:-2]}")
    print("\n=== 2. BORLAND KEY-VALUE JUMP TABLES ===")
    for j in jt_details:
        print(f"  seg{j['segment']}:{j['site']} disp={j['disp']} base={j['detected_base']} count={j['detected_count']} keys={j['keys_hex'][:6]} tgts={j['targets'][:6]}")
    print("\n=== 3. ISA ===")
    print("  unique mnemonics:", len(mnem_counts), "STD sites:", [(s['segment'], s['offset']) for s in std_sites])
    print("\n=== 4. WINEXEC ===")
    for w in winexec_sites:
        for l in w["window"]: print(" ", l)
    print("\n=== 5. BOOTSTRAP TAIL & DGROUP INIT/EXIT ===")
    print("  DGROUP code relocs around 0x20B0..0x2118:")
    for c in init_candidates:
        if int(c["ptr_offset"], 16) >= 0x2000:
            print("  ", c)
    print("\n=== 6. LOADCURSOR ===")
    for c in loadcursor_sites:
        print(f"  seg{c['segment']}:{c['site']} ->", " | ".join(x.split(";")[0].strip() for x in c["window"][-5:-2]))


if __name__ == "__main__":
    main()

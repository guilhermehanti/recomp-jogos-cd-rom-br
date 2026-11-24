"""Map Win16 bootstrap (1:0000), DGROUP/data segments, KERNEL!113 (__AHSHIFT), WIN87EM!1, and x87 usage."""
import json
import re
from collections import Counter, defaultdict
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
ANALYSIS = ROOT / "analysis"
DISASM = ANALYSIS / "disassembly"
EXE_PATH = ROOT / "build" / "BIGJOBNG.EXE"

LINE_RE = re.compile(r"^\s*([0-9a-f]+):\s+((?:[0-9a-f]{2} )+)\s*(.*)$")
X87_RE = re.compile(r"^(f[a-z0-9]+|wait|fwait)\b")


def load_seg_lines(seg_id):
    path = DISASM / f"seg{seg_id:02d}.asm"
    out = []
    for line in path.read_text(encoding="utf8").splitlines():
        m = LINE_RE.match(line)
        if m:
            out.append((int(m[1], 16), m[2].strip(), m[3].strip()))
    return out


def main():
    inv = json.loads((ANALYSIS / "inventory.json").read_text(encoding="utf8"))
    ne = inv["NE"]["BIGJOBNG.EXE"]
    segs = {s["id"]: s for s in ne["segments"]}
    refs = json.loads((DISASM / "references.json").read_text(encoding="utf8"))
    exe = EXE_PATH.read_bytes()

    # 1. Bootstrap 1:0000..1:0140
    seg1 = load_seg_lines(1)
    bootstrap_lines = [f"{off:04X}: {mnem}" for off, _, mnem in seg1 if off <= 0x0120]

    # Check compiler signature in DGROUP (seg49)
    dgroup = segs[49]
    dgroup_bytes = exe[dgroup["offset"]:dgroup["offset"] + dgroup["size"]]
    ascii_strings = [
        (m.start(), m.group().decode("ascii", errors="ignore"))
        for m in re.finditer(rb"[\x20-\x7e]{6,}", dgroup_bytes)
    ]
    compiler_hints = [
        (f"49:{off:04X}", s) for off, s in ascii_strings
        if any(k in s.lower() for k in ("borland", "microsoft", "c++", "runtime", "copyright", "stack", "null", "floating", "heap", "divide"))
    ]

    # 2. All 9 data segments summary
    data_segments = []
    for sid in range(41, 50):
        s = segs[sid]
        seg_refs = [r for r in refs if r["segment"] == sid]
        data_segments.append({
            "segment": sid,
            "file_offset": f"0x{s['offset']:X}",
            "size": s["size"],
            "minalloc": s["minalloc"],
            "bss_bytes": max(0, s["minalloc"] - s["size"]),
            "flags": s["flags"],
            "relocations": len(seg_refs),
        })

    # 3. KERNEL!113 (__AHSHIFT) all 128 sites
    ahshift_refs = [r for r in refs if r["target"] == "KERNEL!113"]
    ahshift_by_seg = Counter(r["segment"] for r in ahshift_refs)
    ahshift_patterns = Counter()
    ahshift_samples = []
    seg_cache = {}
    for r in ahshift_refs:
        sid = r["segment"]
        if sid not in seg_cache:
            seg_cache[sid] = load_seg_lines(sid)
        insts = seg_cache[sid]
        # find instruction containing r["site"]
        match_inst = None
        for idx, (off, raw, mnem) in enumerate(insts):
            nbytes = len(raw.split())
            if off <= r["site"] < off + nbytes:
                match_inst = (idx, off, mnem)
                break
        if match_inst:
            idx, off, mnem = match_inst
            clean = mnem.split(";")[0].strip()
            ahshift_patterns[clean] += 1
            if len(ahshift_samples) < 15:
                win = [f"{insts[j][0]:04X}: {insts[j][2]}" for j in range(max(0, idx - 2), min(len(insts), idx + 3))]
                ahshift_samples.append({"segment": sid, "site": f"0x{r['site']:04X}", "window": win})

    # 4. WIN87EM!1 (__fpMath) all 6 sites
    win87_refs = [r for r in refs if r["target"] == "WIN87EM!1"]
    win87_details = []
    for r in win87_refs:
        sid = r["segment"]
        if sid not in seg_cache:
            seg_cache[sid] = load_seg_lines(sid)
        insts = seg_cache[sid]
        for idx, (off, raw, mnem) in enumerate(insts):
            nbytes = len(raw.split())
            if off <= r["site"] < off + nbytes:
                win = [f"{insts[j][0]:04X}: {insts[j][2]}" for j in range(max(0, idx - 5), min(len(insts), idx + 5))]
                win87_details.append({
                    "segment": sid,
                    "site": f"0x{r['site']:04X}",
                    "source_type": r["source_type"],
                    "flags": r["flags"],
                    "window": win,
                })
                break

    # 5. Scan ALL 40 code segments for x87 instructions
    x87_by_seg = {}
    x87_mnemonics = Counter()
    for sid in range(1, 41):
        insts = seg_cache.get(sid) or load_seg_lines(sid)
        hits = []
        for off, raw, mnem in insts:
            clean = mnem.split(";")[0].strip()
            m = X87_RE.match(clean)
            if m:
                op = m[1]
                # Filter out false positive 'fs' prefix if any (already regex 'f[a-z0-9]+')
                if op in ("fs",):
                    continue
                x87_mnemonics[op] += 1
                hits.append((f"0x{off:04X}", clean))
        if hits:
            x87_by_seg[sid] = {
                "count": len(hits),
                "first_offset": hits[0][0],
                "last_offset": hits[-1][0],
                "sample": hits[:8],
            }

    out = {
        "ne_header": {
            "cs_ip": ne["cs_ip"],
            "ss_sp": ne["ss_sp"],
            "dgroup": ne["dgroup"],
            "flags": ne["flags"],
            "os": ne["os"],
        },
        "compiler_hints": compiler_hints[:30],
        "bootstrap_1_0000": bootstrap_lines,
        "data_segments": data_segments,
        "ahshift_total": len(ahshift_refs),
        "ahshift_by_segment": dict(sorted(ahshift_by_seg.items())),
        "ahshift_instruction_patterns": dict(ahshift_patterns.most_common()),
        "ahshift_samples": ahshift_samples,
        "win87em_total": len(win87_refs),
        "win87em_sites": win87_details,
        "x87_segments": x87_by_seg,
        "x87_mnemonic_counts": dict(x87_mnemonics.most_common()),
    }
    (ANALYSIS / "bootstrap-x87.json").write_text(json.dumps(out, indent=2), encoding="utf8")
    print("NE header:", out["ne_header"])
    print("Compiler hints:", out["compiler_hints"][:10])
    print("AHSHIFT total:", out["ahshift_total"], "patterns:", out["ahshift_instruction_patterns"])
    print("WIN87EM total:", out["win87em_total"])
    print("x87 segments:", {k: v["count"] for k, v in x87_by_seg.items()})
    print("x87 top mnemonics:", list(x87_mnemonics.most_common(15)))


if __name__ == "__main__":
    main()

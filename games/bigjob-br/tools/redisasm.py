"""Resynchronise the linear LLVM listings using reachability-guided BFS from true code seeds.
Never follows fake branches inside ASCII strings or vtable data, and guarantees zero overlapping instructions.
Output: analysis/disassembly-resync/segNN.asm."""
import json
import re
import struct
import subprocess
from collections import deque
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
ANALYSIS = ROOT / "analysis"
SRC = ANALYSIS / "disassembly"
DST = ANALYSIS / "disassembly-resync"
BUILD = ROOT / "build"

LINE_RE = re.compile(r"^\s*([0-9a-f]+):\s+((?:[0-9a-f]{2}[ \t])+)\s*(.*)$")
BR_RE = re.compile(r"^(?:rep\w*\s+|lock\s+|addr32\s+|data32\s+)*(callw|call|jmpw|jmp|j[a-z]+|loop\w*)\s+0x([0-9a-f]+)\b")
RET_RE = re.compile(r"^(?:lretw|lret|retw|ret|iretw|iret|ljmpw|ljmp)\b")
CHUNK = 0x200


def load_linear(seg):
    insts = {}
    for line in (SRC / f"seg{seg:02d}.asm").read_text(encoding="utf8").splitlines():
        m = LINE_RE.match(line)
        if m:
            off = int(m[1], 16)
            n = len(m[2].split())
            insts[off] = (n, line.split(" ; NE ")[0])
    return insts


def objdump_range(seg, start, stop):
    out = subprocess.check_output(
        ["llvm-objdump", "-d", "--mattr=+16bit-mode",
         f"--start-address=0x{start:X}", f"--stop-address=0x{stop:X}",
         str(BUILD / f"seg{seg}.obj")],
        text=True
    )
    res = []
    for line in out.splitlines():
        m = LINE_RE.match(line)
        if m:
            res.append((int(m[1], 16), len(m[2].split()), line))
    return res


def overlay_chain(insts, chain):
    """Insert instructions from chain into insts, removing any overlapping instructions."""
    for off, n, line in chain:
        # Remove any existing instruction [o, o+on) that overlaps [off, off+n)
        to_del = [o for o, (on, _) in insts.items() if not (o + on <= off or o >= off + n)]
        for o in to_del:
            del insts[o]
        insts[off] = (n, line)


def resync_from(seg, target, size, insts):
    chain = objdump_range(seg, target, min(size, target + CHUNK))
    if not chain or chain[0][0] != target:
        return 0
    accepted = []
    for off, n, line in chain:
        accepted.append((off, n, line))
        next_off = off + n
        # Stop as soon as the end of this instruction aligns with an existing instruction boundary
        if next_off in insts:
            break
    overlay_chain(insts, accepted)
    return 1


def main():
    inv = json.loads((ANALYSIS / "inventory.json").read_text(encoding="utf8"))
    ne = inv["NE"]["BIGJOBNG.EXE"]
    seg_meta = {s["id"]: s for s in ne["segments"]}
    exe = (BUILD / "BIGJOBNG.EXE").read_bytes()
    refs = json.loads((SRC / "references.json").read_text(encoding="utf8"))
    cfg = json.loads((ANALYSIS / "recomp-cfg.json").read_text(encoding="utf8"))
    DST.mkdir(exist_ok=True)

    # Identify vtable sites so we never seed inside a vtable
    vtable_sites = set()
    for vt in cfg["vtables"]:
        for e in vt["entries"]:
            off = int(e["offset"], 16)
            for k in range(4):
                vtable_sites.add((vt["segment"], off + k))

    # Build true code seeds per segment
    seeds_by_seg = {s: set() for s in range(1, 41)}
    seeds_by_seg[1].add(0)
    for ent in ne.get("entries", []):
        if ent[1] in seeds_by_seg:
            seeds_by_seg[ent[1]].add(ent[2])

    # Borland _INIT_/_EXIT_ near records in DGROUP (49:20B8..2118)
    s49 = seg_meta[49]
    for rec in range(0x20B8, 0x2118, 6):
        if exe[s49["offset"] + rec] == 0:
            off = struct.unpack_from("<H", exe, s49["offset"] + rec + 2)[0]
            if off:
                seeds_by_seg[1].add(off)

    # Relocated far code targets: ONLY vtable entries or lcall/ljmp targets (never CS string/data offsets!)
    for vt in cfg["vtables"]:
        for e in vt["entries"]:
            ts, to = e["target"].split(":")
            tseg, toff = int(ts), int(to, 16)
            if tseg in seeds_by_seg and (tseg, toff) not in vtable_sites:
                seeds_by_seg[tseg].add(toff)

    for r in refs:
        if ":" in r["target"] and r["source_type"] == 3:
            ts, to = r["target"].split(":")
            tseg, toff = int(ts), int(to, 16)
            if tseg not in seeds_by_seg or (tseg, toff) in vtable_sites:
                continue
            if r["data_segment"]:
                # Far code pointers stored in data segments (e.g., DGROUP _INIT_/_EXIT_ table 1:40F0)
                seeds_by_seg[tseg].add(toff)
            else:
                sseg, ssite = r["segment"], r["site"]
                sraw = exe[seg_meta[sseg]["offset"]:seg_meta[sseg]["offset"] + seg_meta[sseg]["size"]]
                # Direct lcall (0x9A) or ljmp (0xEA)
                if ssite > 0 and sraw[ssite - 1] in (0x9A, 0xEA):
                    seeds_by_seg[tseg].add(toff)

    # Jump table targets from recomp-cfg.json (skip bogus 1:1211 inside x87 CRT data)
    jts_by_seg = {s: {} for s in range(1, 41)}
    for jt in cfg["jump_tables"]:
        s = jt["segment"]
        site = int(jt["site"], 16)
        if (s, site) == (1, 0x1211):
            continue
        jts_by_seg[s][site] = [int(t, 16) for t in jt["targets"]]
        for t in jt["targets"]:
            seeds_by_seg[s].add(int(t, 16))

    # True binary prologues in raw segment bytes
    prologues = (
        bytes.fromhex("45558bec"),        # inc bp; push bp; mov bp, sp
        bytes.fromhex("558bec"),          # push bp; mov bp, sp
        bytes.fromhex("1e558bec"),        # push ds; push bp; mov bp, sp
        bytes.fromhex("8cd89045558bec"),  # mov ax, ds; nop; inc bp; push bp; mov bp, sp
    )
    for s in range(1, 41):
        raw = exe[seg_meta[s]["offset"]:seg_meta[s]["offset"] + seg_meta[s]["size"]]
        for pb in prologues:
            pos = 0
            while True:
                idx = raw.find(pb, pos)
                if idx == -1:
                    break
                if pb == bytes.fromhex("558bec") and idx > 0 and raw[idx - 1] in (0x45, 0x1E):
                    pos = idx + 1
                    continue
                if pb == bytes.fromhex("45558bec") and idx >= 3 and raw[idx - 3:idx] == bytes.fromhex("8cd890"):
                    pos = idx + 1
                    continue
                if (s, idx) not in vtable_sites:
                    seeds_by_seg[s].add(idx)
                pos = idx + 1

    total_resyncs = 0
    for seg in range(1, 41):
        insts = load_linear(seg)
        size = seg_meta[seg]["size"]
        jts = jts_by_seg[seg]

        # Reachability-guided BFS resync: only resync targets reached from true code seeds!
        visited = set()
        queue = deque(sorted(t for t in seeds_by_seg[seg] if 0 <= t < size))
        while queue:
            off = queue.popleft()
            if off in visited or not (0 <= off < size):
                continue
            if off not in insts:
                total_resyncs += resync_from(seg, off, size, insts)
            if off not in insts:
                continue
            visited.add(off)
            n, line = insts[off]
            code = LINE_RE.match(line)[3].split(";")[0].split("#")[0].strip()
            next_off = off + n

            if off in jts:
                for toff in jts[off]:
                    if toff not in visited:
                        queue.append(toff)
                continue

            if RET_RE.match(code):
                continue

            m = BR_RE.match(code)
            if m:
                op, tgt_s = m[1], m[2]
                toff = int(tgt_s, 16)
                if 0 <= toff < size and toff not in visited:
                    queue.append(toff)
                if op in ("jmp", "jmpw"):
                    continue
                if next_off < size and next_off not in visited:
                    queue.append(next_off)
                continue

            if code.startswith(("jmp ", "jmpw ")):
                continue

            if next_off < size and next_off not in visited:
                queue.append(next_off)

        # Final guarantee: ensure every true seed is on an instruction boundary
        for t in sorted(seeds_by_seg[seg]):
            if 0 <= t < size and t not in insts:
                total_resyncs += resync_from(seg, t, size, insts)

        # Re-attach NE relocation annotations
        sites = {}
        for r in refs:
            if r["segment"] == seg:
                sites.setdefault(r["site"], []).append(r["target"])
        lines = []
        for off in sorted(insts):
            n, line = insts[off]
            notes = [f"{s:04X}={t}" for s in range(off, off + n) for t in sites.get(s, [])]
            if notes:
                line += " ; NE " + ", ".join(notes)
            lines.append(line)
        (DST / f"seg{seg:02d}.asm").write_text("\n".join(lines) + "\n", encoding="utf8")

    print(f"Done: reachability-guided resync applied {total_resyncs} clean chains across 40 segments.")


if __name__ == "__main__":
    main()

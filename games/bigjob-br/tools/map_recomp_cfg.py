"""Map function boundaries, jump tables, vtables, indirect calls, and QT stub reachability for BIGJOBNG.EXE."""
import json
import re
import struct
from collections import Counter, defaultdict
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
ANALYSIS = ROOT / "analysis"
DISASM = ANALYSIS / "disassembly"
EXE_PATH = ROOT / "build" / "BIGJOBNG.EXE"

LINE_RE = re.compile(r"^\s*([0-9a-f]+):\s+((?:[0-9a-f]{2}[ \t])+)\s*(.*)$")
CALL_NEAR_RE = re.compile(r"^callw\s+0x([0-9a-f]+)\b")
JMP_NEAR_RE = re.compile(r"^(j\w+|loop\w*)\s+0x([0-9a-f]+)\b")
INDIRECT_RE = re.compile(r"^(lcallw?|callw?|ljmpw?|jmpw?)\s+\*(.+)$")
CS_TABLE_RE = re.compile(r"^jmpw\s+\*%cs:0x([0-9a-f]+)\(([^)]+)\)")
DS_TABLE_RE = re.compile(r"^jmpw\s+\*0x([0-9a-f]+)\(([^)]+)\)")
PROLOGUE_BYTES = (
    bytes.fromhex("558bec"),       # push bp; mov bp, sp
    bytes.fromhex("45558bec"),     # inc bp; push bp; mov bp, sp (Win16 far)
    bytes.fromhex("1e558bec"),     # push ds; push bp; mov bp, sp
    bytes.fromhex("8cd89045558bec"),  # mov ax, ds; nop; inc bp; push bp; mov bp, sp
)


def parse_target(target_str):
    if ":" not in target_str:
        return None
    seg_s, off_s = target_str.split(":", 1)
    if not seg_s.isdigit():
        return None
    return int(seg_s), int(off_s, 16)


def main():
    inv = json.loads((ANALYSIS / "inventory.json").read_text(encoding="utf8"))
    ne = inv["NE"]["BIGJOBNG.EXE"]
    segs_meta = {s["id"]: s for s in ne["segments"]}
    exe = EXE_PATH.read_bytes()
    refs = json.loads((DISASM / "references.json").read_text(encoding="utf8"))
    api_bounds = json.loads((ANALYSIS / "api-boundaries.json").read_text(encoding="utf8"))

    # Separate code vs data segments (flag bit 0: 0=CODE, 1=DATA)
    code_segs = sorted(sid for sid, s in segs_meta.items() if (int(s["flags"], 16) & 1) == 0)
    data_segs = sorted(sid for sid, s in segs_meta.items() if (int(s["flags"], 16) & 1) != 0)

    # Index relocations
    refs_by_seg = defaultdict(list)
    code_reloc_targets = defaultdict(list)  # (tseg, toff) -> list of (sseg, ssite, stype, flags)
    for r in refs:
        refs_by_seg[r["segment"]].append(r)
        parsed = parse_target(r["target"])
        if parsed and parsed[0] in code_segs:
            code_reloc_targets[parsed].append((r["segment"], r["site"], r["source_type"], r["flags"]))

    # Find vtables / far-pointer tables: runs of >= 2 relocations of source_type=3 spaced by 4 bytes
    # pointing to code segments (or mixed with 0000:0000 NULL slots, handled as runs with stride 4)
    vtables = []
    vtable_sites = set()
    for seg_id in sorted(segs_meta.keys()):
        far_code_refs = sorted(
            (r["site"], r["target"])
            for r in refs_by_seg[seg_id]
            if r["source_type"] == 3 and parse_target(r["target"]) is not None and parse_target(r["target"])[0] in code_segs
        )
        # Filter out instruction operands: in code segments, a far call is 9A <off> <seg> (site-1 == 0x9A)
        # or ljmp EA <off> <seg> (site-1 == 0xEA). Pure pointer tables have consecutive entries at +4 (+8 if NULL).
        seg_off = segs_meta[seg_id]["offset"]
        seg_bytes = exe[seg_off:seg_off + segs_meta[seg_id]["size"]]
        non_inst_refs = []
        for site, target in far_code_refs:
            if seg_id in code_segs and site > 0 and seg_bytes[site - 1] in (0x9A, 0xEA):
                # Could be lcall/ljmp; check if part of a 4-byte-aligned table anyway later
                continue
            non_inst_refs.append((site, target))

        i = 0
        while i < len(non_inst_refs):
            run = [non_inst_refs[i]]
            j = i + 1
            while j < len(non_inst_refs):
                gap = non_inst_refs[j][0] - run[-1][0]
                if gap == 4:
                    run.append(non_inst_refs[j])
                    j += 1
                elif gap == 8:
                    # Check if intermediate 4 bytes are zero (NULL vtable slot)
                    mid = run[-1][0] + 4
                    if mid + 4 <= len(seg_bytes) and seg_bytes[mid:mid + 4] == b"\x00\x00\x00\x00":
                        run.append(non_inst_refs[j])
                        j += 1
                    else:
                        break
                else:
                    break
            if len(run) >= 2:
                vtables.append({
                    "segment": seg_id,
                    "is_data_segment": seg_id in data_segs,
                    "start": f"0x{run[0][0]:04X}",
                    "end": f"0x{run[-1][0] + 4:04X}",
                    "entry_count": len(run),
                    "entries": [{"offset": f"0x{s:04X}", "target": t} for s, t in run],
                })
                for s, _ in run:
                    vtable_sites.add((seg_id, s))
            i = j

    # Parse disassembly of all 40 code segments
    near_call_targets = defaultdict(set)
    indirect_calls = []
    jump_tables = []
    seg_stats = []
    all_functions = []

    for seg_id in code_segs:
        asm_path = ANALYSIS / "disassembly-resync" / f"seg{seg_id:02d}.asm"
        lines = asm_path.read_text(encoding="utf8").splitlines()
        seg_off = segs_meta[seg_id]["offset"]
        seg_size = segs_meta[seg_id]["size"]
        seg_bytes = exe[seg_off:seg_off + seg_size]

        insts = []  # (off, raw_bytes_str, mnem)
        inst_by_off = {}
        for line in lines:
            m = LINE_RE.match(line)
            if not m:
                continue
            off = int(m[1], 16)
            raw = m[2].strip()
            mnem = m[3].split(";")[0].strip()
            insts.append((off, raw, mnem))
            inst_by_off[off] = (raw, mnem)

        # Collect near calls, indirect calls, and jump tables
        for idx, (off, raw, mnem) in enumerate(insts):
            cm = CALL_NEAR_RE.match(mnem)
            if cm:
                near_call_targets[seg_id].add(int(cm[1], 16))

            im = INDIRECT_RE.match(mnem)
            if im:
                op, target_expr = im[1], im[2].strip()
                kind = "other_indirect"
                if op.startswith("jmp") and ("%cs:" in target_expr or CS_TABLE_RE.match(mnem)):
                    kind = "cs_jump_table"
                elif op.startswith("jmp") and DS_TABLE_RE.match(mnem):
                    kind = "ds_jump_table"
                elif "0x1546" in target_expr or "0x154e" in target_expr:
                    kind = "quicktime_dispatcher"
                elif op.startswith("lcall"):
                    if "(%bx)" in target_expr or "(%si)" in target_expr or "(%di)" in target_expr:
                        kind = "vtable_or_fptr_lcall"
                    elif "(%bp)" in target_expr:
                        kind = "stack_fptr_lcall"
                    else:
                        kind = "mem_fptr_lcall"
                elif op.startswith("call"):
                    kind = "near_indirect_call"
                elif op.startswith("ljmp"):
                    kind = "far_indirect_jmp"

                indirect_calls.append({
                    "segment": seg_id,
                    "offset": f"0x{off:04X}",
                    "op": op,
                    "expr": target_expr,
                    "kind": kind,
                })

                # Decode CS jump tables directly from segment bytes
                csm = CS_TABLE_RE.match(mnem)
                if csm:
                    tbl_off = int(csm[1], 16)
                    # Check Borland key-value switch idiom first:
                    #   movw $count, %cx; movw $base, %bx; ... jmpw *%cs:<2*count>(%bx)
                    kv_base, kv_count = None, None
                    bound = None
                    for back in range(max(0, idx - 12), idx):
                        bmnem = insts[back][2]
                        bm = re.search(r"cmpw\s+\$0x([0-9a-f]+),", bmnem)
                        if bm:
                            bound = int(bm[1], 16) + 1
                        mbx = re.search(r"movw\s+\$0x([0-9a-f]+),\s*%bx", bmnem)
                        if mbx:
                            kv_base = int(mbx[1], 16)
                        mcx = re.search(r"movw\s+\$0x([0-9a-f]+),\s*%cx", bmnem)
                        if mcx:
                            kv_count = int(mcx[1], 16)
                    if kv_base is not None and kv_count is not None and tbl_off == 2 * kv_count:
                        keys = [struct.unpack_from("<H", seg_bytes, kv_base + 2 * k)[0] for k in range(kv_count)]
                        entries = [struct.unpack_from("<H", seg_bytes, kv_base + tbl_off + 2 * k)[0] for k in range(kv_count)]
                        jump_tables.append({
                            "segment": seg_id,
                            "site": f"0x{off:04X}",
                            "kind": "borland_kv_switch",
                            "table_offset": f"0x{kv_base:04X}",
                            "bound_from_cmp": kv_count,
                            "entries_count": len(entries),
                            "keys": [f"0x{k:04X}" for k in keys],
                            "targets": [f"0x{v:04X}" for v in entries],
                        })
                    else:
                        # Standard indexed jump table
                        entries = []
                        max_entries = bound if (bound is not None and 1 <= bound <= 256) else 64
                        for k in range(max_entries):
                            pos = tbl_off + 2 * k
                            if pos + 2 > seg_size:
                                break
                            val = struct.unpack_from("<H", seg_bytes, pos)[0]
                            if bound is None and (val >= seg_size or (k > 0 and abs(val - entries[0]) > 0x3000)):
                                break
                            entries.append(val)
                        jump_tables.append({
                            "segment": seg_id,
                            "site": f"0x{off:04X}",
                            "kind": "indexed_cs_switch",
                            "table_offset": f"0x{tbl_off:04X}",
                            "bound_from_cmp": bound,
                            "entries_count": len(entries),
                            "targets": [f"0x{v:04X}" for v in entries],
                        })

        # Discover function starts:
        # 1) All near call targets
        # 2) All far relocation targets in this segment (excluding vtable sites themselves)
        # 3) All standard prologues in raw bytes (even if linear disassembly missed them after jump tables!)
        # 4) Entry point 1:0000 and NE exported entries
        func_sources = defaultdict(set)
        for t in near_call_targets[seg_id]:
            if 0 <= t < seg_size:
                func_sources[t].add("near_call")
        for (tseg, toff), callers in code_reloc_targets.items():
            if tseg == seg_id and 0 <= toff < seg_size:
                # Filter out data tables referenced inside code segment (like vtables or jump tables)
                if (seg_id, toff) not in vtable_sites:
                    # Check if target looks like code (prologue, push, mov, sub, jmp, etc.) vs pure data
                    b0 = seg_bytes[toff]
                    if b0 in (0x55, 0x45, 0x1E, 0x8C, 0xB8, 0x56, 0x57, 0x33, 0x31, 0xC8, 0x8B, 0xC4, 0xEB, 0xE9, 0xCB, 0xC3, 0xCA, 0xC2, 0x90, 0xFA, 0xFB, 0xBB):
                        func_sources[toff].add("far_reloc")
        for pbytes in PROLOGUE_BYTES:
            start = 0
            while True:
                pos = seg_bytes.find(pbytes, start)
                if pos == -1:
                    break
                # Avoid matching '55 8b ec' inside '45 55 8b ec' or '1e 55 8b ec'
                if pbytes == bytes.fromhex("558bec") and pos > 0 and seg_bytes[pos - 1] in (0x45, 0x1E):
                    start = pos + 1
                    continue
                if pbytes == bytes.fromhex("45558bec") and pos >= 3 and seg_bytes[pos - 3:pos] == bytes.fromhex("8cd890"):
                    start = pos + 1
                    continue
                # Ignore matches inside vtable regions
                if (seg_id, pos) not in vtable_sites:
                    func_sources[pos].add("prologue_scan")
                start = pos + 1

        if seg_id == 1:
            func_sources[0].add("ne_entry_cs_ip")
        for ent in ne.get("entries", []):
            if ent[1] == seg_id:
                func_sources[ent[2]].add("ne_export")

        sorted_starts = sorted(func_sources.keys())
        seg_funcs = []
        for fi, fstart in enumerate(sorted_starts):
            fend = sorted_starts[fi + 1] if fi + 1 < len(sorted_starts) else seg_size
            b = seg_bytes[fstart:min(fstart + 8, seg_size)]
            is_far_prolog = b.startswith(bytes.fromhex("45558bec")) or b.startswith(bytes.fromhex("8cd89045558bec"))
            seg_funcs.append({
                "segment": seg_id,
                "start": f"0x{fstart:04X}",
                "max_end": f"0x{fend:04X}",
                "sources": sorted(func_sources[fstart]),
                "far_prologue": is_far_prolog,
            })
        all_functions.extend(seg_funcs)
        seg_stats.append({
            "segment": seg_id,
            "size": seg_size,
            "functions": len(seg_funcs),
            "far_prologues": sum(1 for f in seg_funcs if f["far_prologue"]),
            "near_call_targets": len(near_call_targets[seg_id]),
            "cs_jump_tables": sum(1 for jt in jump_tables if jt["segment"] == seg_id),
            "indirect_calls": sum(1 for ic in indirect_calls if ic["segment"] == seg_id),
            "vtables_in_seg": sum(1 for vt in vtables if vt["segment"] == seg_id),
        })

    # Verify the 127 QuickTime stubs in seg01: check if ANY of the 110 unreferenced stubs appear anywhere
    qt_stubs = api_bounds.get("quicktime_dispatch_stubs", [])
    unreferenced_qt = [w for w in qt_stubs if not w.get("callers") and not w.get("near_call_instruction_sites")]
    referenced_qt = [w for w in qt_stubs if w.get("callers") or w.get("near_call_instruction_sites")]

    # Scan ALL 49 segments for any 16-bit LE occurrence of unreferenced stub offsets
    seg_raw_map = {}
    for sid, s in segs_meta.items():
        seg_raw_map[sid] = exe[s["offset"]:s["offset"] + s["size"]]

    qt_word_hits = {}
    for w in unreferenced_qt:
        off_int = int(w["address"].split(":")[1], 16)
        needle = struct.pack("<H", off_int)
        hits = []
        for sid, raw in seg_raw_map.items():
            pos = 0
            while True:
                idx = raw.find(needle, pos)
                if idx == -1:
                    break
                # Check if there's a relocation at idx (or idx+2 for segment 1)
                has_reloc_seg1 = any(
                    r["site"] in (idx, idx + 2) and parse_target(r["target"]) == (1, off_int)
                    for r in refs_by_seg[sid]
                )
                if has_reloc_seg1:
                    hits.append({"segment": sid, "offset": f"0x{idx:04X}", "relocated_to_seg1": True})
                pos = idx + 1
        # Also check if any jump table in seg01 targets this stub
        jt_hits = [jt["site"] for jt in jump_tables if jt["segment"] == 1 and f"0x{off_int:04X}" in jt["targets"]]
        if hits or jt_hits:
            qt_word_hits[f"1:{off_int:04X}"] = {"reloc_hits": hits, "jt_hits": jt_hits}

    # Also check if seg01 has any jump tables or indirect calls targeting the QT stub range (0x0640..0x0C20)
    seg01_indirect_in_qt = [
        ic for ic in indirect_calls if ic["segment"] == 1 and 0x0600 <= int(ic["offset"], 16) <= 0x0C50
    ]
    seg01_jts = [jt for jt in jump_tables if jt["segment"] == 1]

    kind_counts = Counter(ic["kind"] for ic in indirect_calls)
    out = {
        "summary": {
            "code_segments": len(code_segs),
            "data_segments": len(data_segs),
            "total_functions_discovered": len(all_functions),
            "total_far_prologues": sum(1 for f in all_functions if f["far_prologue"]),
            "total_cs_jump_tables": len(jump_tables),
            "total_vtables_or_far_tables": len(vtables),
            "total_indirect_transfers": len(indirect_calls),
            "indirect_by_kind": dict(kind_counts),
            "qt_stubs_total": len(qt_stubs),
            "qt_stubs_referenced": len(referenced_qt),
            "qt_stubs_unreferenced": len(unreferenced_qt),
            "qt_unreferenced_reloc_or_jt_hits": len(qt_word_hits),
            "seg01_jump_tables_count": len(seg01_jts),
        },
        "segments": seg_stats,
        "jump_tables": jump_tables,
        "vtables": vtables,
        "indirect_calls": indirect_calls,
        "qt_unreferenced_hits": qt_word_hits,
        "seg01_qt_range_indirect_calls": seg01_indirect_in_qt,
        "functions": all_functions,
    }
    (ANALYSIS / "recomp-cfg.json").write_text(json.dumps(out, indent=2), encoding="utf8")
    print(json.dumps(out["summary"], indent=2))


if __name__ == "__main__":
    main()

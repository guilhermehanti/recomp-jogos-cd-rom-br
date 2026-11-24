"""Map GDI/USER graphics & resource boundaries, NE embedded resources, and all 4,744 PIC headers/formats."""
import json
import re
import struct
from collections import Counter, defaultdict
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
ANALYSIS = ROOT / "analysis"
DISASM = ANALYSIS / "disassembly"
EXE_PATH = ROOT / "build" / "BIGJOBNG.EXE"

LINE_RE = re.compile(r"^\s*([0-9a-f]+):\s+((?:[0-9a-f]{2} )+)\s*(.*)$")

GDI_TARGETS = {
    "GDI!439": "StretchDIBits",
    "GDI!360": "CreatePalette",
    "USER!282": "SelectPalette",
    "USER!283": "RealizePalette",
    "GDI!57": "CreateFontIndirect",
    "USER!85": "DrawText",
    "GDI!91": "GetTextExtent",
    "GDI!2": "SetBkMode",
    "GDI!9": "SetTextColor",
    "USER!221": "ScrollDC",
    "USER!81": "FillRect",
    "GDI!64": "CreateRectRgn",
    "GDI!44": "SelectClipRgn",
    "GDI!77": "GetClipBox",
    "GDI!53": "CreateDC",
    "GDI!68": "DeleteDC",
    "GDI!38": "Escape",
    "GDI!377": "StartDoc",
    "GDI!378": "EndDoc",
    "USER!173": "LoadCursor",
    "USER!174": "LoadIcon",
    "USER!176": "LoadString",
    "USER!177": "LoadAccelerators",
    "USER!178": "TranslateAccelerator",
}

RT_NAMES = {
    1: "RT_CURSOR",
    2: "RT_BITMAP",
    3: "RT_ICON",
    4: "RT_MENU",
    5: "RT_DIALOG",
    6: "RT_STRING",
    9: "RT_ACCELERATOR",
    12: "RT_GROUP_CURSOR",
    14: "RT_GROUP_ICON",
    16: "RT_VERSION",
}


def load_seg_lines(seg_id):
    path = DISASM / f"seg{seg_id:02d}.asm"
    out = []
    for line in path.read_text(encoding="utf8").splitlines():
        m = LINE_RE.match(line)
        if m:
            out.append((int(m[1], 16), m[2].strip(), m[3].strip()))
    return out


def parse_ne_resources(exe, ne):
    shift = ne["resource_shift"]
    res_summary = []
    strings = {}
    accelerators = []
    for rtype_key, entries in ne["resources"].items():
        if str(rtype_key).isdigit():
            rt_num = int(rtype_key) & 0x7FFF
            rt_name = RT_NAMES.get(rt_num, f"TYPE_{rt_num}")
        else:
            rt_num = None
            rt_name = str(rtype_key)
        for ent in entries:
            off_s, len_s, flags, rid_raw = ent[0], ent[1], ent[2], ent[3]
            rid_val = (rid_raw & 0x7FFF) if isinstance(rid_raw, int) else rid_raw
            off = off_s << shift
            size = len_s << shift
            raw = exe[off:off + size]
            res_summary.append({
                "type": rt_name,
                "id": rid_val,
                "offset": f"0x{off:X}",
                "size": size,
                "flags": f"0x{flags:04X}",
            })
            if rt_num == 6 and isinstance(rid_val, int):
                pos = 0
                base_id = (rid_val - 1) * 16
                for i in range(16):
                    if pos >= len(raw):
                        break
                    slen = raw[pos]
                    pos += 1
                    sbytes = raw[pos:pos + slen]
                    pos += slen
                    if slen > 0:
                        strings[base_id + i] = sbytes.decode("cp1252", errors="replace")
            elif rt_num == 9:
                pos = 0
                while pos + 5 <= len(raw):
                    aflags, event, cmd_id = struct.unpack_from("<BHH", raw, pos)
                    pos += 5
                    accelerators.append({
                        "resource_id": rid_val,
                        "flags": f"0x{aflags:02X}",
                        "key": f"0x{event:04X}",
                        "ascii": chr(event) if 32 <= event < 127 else None,
                        "cmd_id": cmd_id,
                    })
                    if aflags & 0x80:
                        break
    return res_summary, strings, accelerators


def scan_pics(inv):
    iso_path = Path(inv["iso"])
    files = {f["path"]: f for f in inv["files"]}
    pic_summary = {}
    with iso_path.open("rb") as f:
        for pic_name in ("BIGJOBCD.PIC", "BIGJOBHD.PIC"):
            meta = files[pic_name]
            base = meta["lba"] * 2048
            f.seek(base + 0x66)
            count, = struct.unpack(">H", f.read(2))
            offsets = struct.unpack(f">{count + 1}I", f.read((count + 1) * 4))
            # Index table after offsets: names (32 bytes? let's check flags offset from check_pic_auto.py: 108 + 46*count)
            # In check_pic_auto.py: offsets is at 104 (0x68), length 4*(count+1) -> ends at 108 + 4*count.
            # Wait: 46*count - 4*count = 42*count bytes of per-entry metadata before flags!
            f.seek(base + 108 + 46 * count)
            idx_flags = struct.unpack(f">{count}H", f.read(2 * count))

            flag_counter = Counter(idx_flags)
            magic_counter = Counter()
            flag_to_magic = defaultdict(Counter)
            sample_by_flag = {}

            for i in range(count):
                off = offsets[i]
                size = offsets[i + 1] - off
                f.seek(base + off)
                hdr = f.read(min(32, size))
                # Typical MMFW Picture header: w(BE16), h(BE16), ... let's inspect first 12 bytes
                if len(hdr) >= 12:
                    w, h, u4, u6, u8, u10 = struct.unpack_from(">6H", hdr, 0)
                    sig = f"u4=0x{u4:04X},u6=0x{u6:04X},u8=0x{u8:04X},u10=0x{u10:04X}"
                else:
                    w = h = 0
                    sig = f"short({len(hdr)})"
                # Also check 0x0055 / 0x0056 marker position (often at +4 or +8 or +10 or +12)
                m_u4 = u4 if len(hdr) >= 6 else None
                magic_counter[f"0x{m_u4:04X}" if m_u4 is not None else "short"] += 1
                flag_to_magic[f"0x{idx_flags[i]:04X}"][f"0x{m_u4:04X}" if m_u4 is not None else "short"] += 1
                if idx_flags[i] not in sample_by_flag:
                    sample_by_flag[idx_flags[i]] = {
                        "index": i,
                        "offset": f"0x{off:X}",
                        "size": size,
                        "w": w,
                        "h": h,
                        "hdr16_hex": hdr[:16].hex(),
                        "hdr32_hex": hdr[:32].hex(),
                    }

            pic_summary[pic_name] = {
                "count": count,
                "index_flags_distribution": {f"0x{k:04X}": v for k, v in sorted(flag_counter.items())},
                "hdr_word_at_4_distribution": dict(magic_counter.most_common(20)),
                "flag_to_hdr_word4": {k: dict(v.most_common(10)) for k, v in sorted(flag_to_magic.items())},
                "samples_by_flag": {f"0x{k:04X}": v for k, v in sorted(sample_by_flag.items())},
            }
    return pic_summary


def main():
    inv = json.loads((ANALYSIS / "inventory.json").read_text(encoding="utf8"))
    ne = inv["NE"]["BIGJOBNG.EXE"]
    exe = EXE_PATH.read_bytes()
    refs = json.loads((DISASM / "references.json").read_text(encoding="utf8"))

    # 1. GDI/USER call sites
    seg_cache = {}
    gdi_calls = defaultdict(list)
    for r in refs:
        if r["target"] not in GDI_TARGETS:
            continue
        sid = r["segment"]
        if sid not in seg_cache:
            seg_cache[sid] = load_seg_lines(sid)
        insts = seg_cache[sid]
        for idx, (off, raw, mnem) in enumerate(insts):
            nbytes = len(raw.split())
            if off <= r["site"] < off + nbytes:
                win = [f"{insts[j][0]:04X}: {insts[j][2]}" for j in range(max(0, idx - 12), min(len(insts), idx + 4))]
                gdi_calls[f"{r['target']} ({GDI_TARGETS[r['target']]})"].append({
                    "segment": sid,
                    "site": f"0x{r['site']:04X}",
                    "window": win,
                })
                break

    # 2. NE resources
    res_summary, strings, accelerators = parse_ne_resources(exe, ne)

    # 3. PIC headers across all 4,744 entries
    pic_summary = scan_pics(inv)

    out = {
        "gdi_user_sites": dict(sorted(gdi_calls.items())),
        "ne_resources_count": len(res_summary),
        "ne_resources_by_type": dict(Counter(r["type"] for r in res_summary)),
        "ne_resources": res_summary,
        "ne_strings": strings,
        "ne_accelerators": accelerators,
        "pic_formats": pic_summary,
    }
    (ANALYSIS / "gdi-pic-contracts.json").write_text(json.dumps(out, indent=2, ensure_ascii=False), encoding="utf8")
    print("NE resources by type:", out["ne_resources_by_type"])
    print("NE strings count:", len(strings))
    for k, v in sorted(strings.items()):
        print(f"  STR {k}: {v!r}")
    print("NE accelerators:", accelerators)
    for pname, pdata in pic_summary.items():
        print(f"\n{pname} (count={pdata['count']}):")
        print("  index flags:", pdata["index_flags_distribution"])
        print("  hdr+4:", pdata["hdr_word_at_4_distribution"])
        print("  samples:", {k: (v["w"], v["h"], v["size"], v["hdr16_hex"]) for k, v in pdata["samples_by_flag"].items()})


if __name__ == "__main__":
    main()

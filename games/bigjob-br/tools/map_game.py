"""Map original Brazilian media and NE references without running the game."""
import argparse
from collections import Counter
import hashlib
import json
from pathlib import Path
import struct


def map_script(data):
    if data[:18] != b"MMScript File\0\x01\x01MM":
        raise ValueError("Unexpected MMS header")
    blocks = {}
    pending = [18]
    while pending:
        start = pending.pop()
        if start in blocks:
            continue
        size, count = struct.unpack_from(">HH", data, start)
        end = start + 4 + size
        if size != count * 12 or end > len(data):
            raise ValueError(f"Invalid MMS block at {start:#x}")
        records = []
        for offset in range(start + 4, end, 12):
            words = struct.unpack_from(">6H", data, offset)
            record = {"offset": offset, "words": words}
            target = (words[2] << 16) | words[3]
            if words[0] & 0xc000 == 0x8000 and 18 <= target <= len(data) - 4:
                record["offset_candidate"] = target
            if words[0] & 0xc000 == 0x8000 and end <= target <= len(data) - 4:
                length, entries = struct.unpack_from(">HH", data, target)
                if length == entries * 12 and target + 4 + length <= len(data):
                    record["candidate_block"] = target
                    pending.append(target)
            records.append(record)
        blocks[start] = {"start": start, "end": end, "records": records}
    ordered = [blocks[start] for start in sorted(blocks)]
    cursor = 18
    gaps = []
    for block in ordered:
        if block["start"] < cursor:
            raise ValueError("Overlapping MMS blocks")
        if block["start"] > cursor:
            gaps.append([cursor, block["start"]])
        cursor = block["end"]
    if cursor < len(data):
        gaps.append([cursor, len(data)])
    counts = Counter(f"{r['words'][0]:04X}:{r['words'][1]:04X}"
                     for block in ordered for r in block["records"])
    opaque_refs = [
        {"record": record["offset"], "target": target}
        for block in ordered for record in block["records"]
        if (target := record.get("offset_candidate")) is not None
        and any(start <= target < end for start, end in gaps)
    ]
    by_offset = {record["offset"]: record for block in ordered for record in block["records"]}
    table_refs = {}
    for reference in opaque_refs:
        target = reference["target"]
        origin = reference["record"]
        table_refs.setdefault(target, []).append(
            {"record": origin, "type": by_offset[origin]["words"][0]})
    tables = []
    for start, end in gaps:
        cursor = start
        while cursor < end:
            size, count = struct.unpack_from(">HH", data, cursor)
            tail = cursor + 4 + count * 12
            limit = cursor + 4 + size
            if not (cursor + 4 <= tail <= limit <= end) or cursor not in table_refs:
                raise ValueError(f"Unrecognized MMS table at {cursor:#x}")
            records = []
            for offset in range(cursor + 4, tail, 12):
                words = struct.unpack_from(">6H", data, offset)
                record = {"offset": offset, "words": words}
                if words[0] == 0xc003:
                    pos = tail + words[3]
                    stop = data.find(b"\0", pos, limit)
                    if pos >= limit or stop < 0:
                        raise ValueError(f"Invalid MMS string reference at {offset:#x}")
                    record["string_offset"] = pos
                    record["string"] = data[pos:stop].decode("cp1252", "replace")
                records.append(record)
            tables.append({"start": cursor, "end": limit, "tail_start": tail,
                           "references": table_refs[cursor], "records": records})
            cursor = limit
    return {"blocks": ordered, "gaps": gaps, "record_counts": counts,
            "opaque_refs": opaque_refs, "tables_with_strings": tables}


def map_bank(image, source, index):
    base = source["lba"] * 2048
    count = index["count"]
    offsets = index["offsets"]
    name_start = 108 + 10 * count
    if offsets[-1] != source["bytes"] or name_start + 32 * count > offsets[0]:
        raise ValueError(f"Invalid table in {source['path']}")
    image.seek(base + name_start)
    names = image.read(32 * count)
    assets = []
    for i, (start, end) in enumerate(zip(offsets, offsets[1:])):
        image.seek(base + start)
        preview = image.read(min(8, end - start))
        assets.append({
            "index": i,
            "name": names[i * 32:(i + 1) * 32].split(b"\0")[0].decode("cp1252", "replace"),
            "offset": start,
            "size": end - start,
            "header_hex": preview.hex(),
        })
    return assets


def map_binary(data, metadata):
    segments = []
    for segment in metadata["segments"]:
        base, size = segment["offset"], segment["size"]
        code = data[base:base + size]
        calls = []
        for source_type, flags, site, target_a, target_b, module, label in segment["fixups"]:
            if source_type != 3 or flags & 4:
                continue
            visited = set()
            while site != 0xffff:
                if site in visited or site + 4 > size:
                    raise ValueError(f"Invalid NE fixup chain in segment {segment['id']}")
                visited.add(site)
                if not int(segment["flags"], 16) & 1 and site and code[site - 1] == 0x9a:
                    kind = flags & 3
                    target = ({"segment": target_a, "offset": target_b} if kind == 0
                              and target_a != 0xff else {"module": module, "symbol": label}
                              if kind in (1, 2) else {"raw": [kind, target_a, target_b]})
                    calls.append({"offset": site - 1, "target": target})
                site = struct.unpack_from("<H", code, site)[0]
        segments.append({**segment, "candidate_far_calls": sorted(calls, key=lambda c: c["offset"])})
    return {**metadata, "segments": segments}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("output", type=Path)
    args = parser.parse_args()
    inventory_path = Path(__file__).resolve().parents[1] / "analysis/inventory.json"
    inventory = json.loads(inventory_path.read_text(encoding="utf8"))
    sources = {entry["path"]: entry for entry in inventory["files"]}
    with Path(inventory["iso"]).open("rb") as image:
        def read(name):
            entry = sources[name]
            image.seek(entry["lba"] * 2048)
            data = image.read(entry["bytes"])
            if len(data) != entry["bytes"]:
                raise ValueError(f"Incomplete extent: {name}")
            return data

        banks = {name: map_bank(image, sources[name], metadata)
                 for name, metadata in inventory["media"].items() if name.endswith((".PIC", ".SND"))}
        script = read("BIGJOB.MMS")
        binaries = {}
        for name in ("BIGJOB.EXE", "BIGJOBNG.EXE"):
            data = read(name)
            if hashlib.sha256(data).hexdigest().lower() != sources[name]["sha256"].lower():
                raise ValueError(f"Inventory hash mismatch: {name}")
            binaries[name] = map_binary(data, inventory["NE"][name])
        dat = read("BIGJOB.DAT")
        signature, low, high = struct.unpack_from(">3H", dat)
        count = high - low + 1
        if signature != 0x4d4d or len(dat) != 6 + count * (10 * 140 + 2):
            raise ValueError("Unexpected DAT layout")
        report = {
            "mms": map_script(script), "banks": banks,
            "movies": inventory["media"]["BIGJOB.MOV"]["movies"],
            "executables": binaries,
            "dat": {"low": low, "high": high,
                    "tables": [6 + count * 140 * i for i in range(10)],
                    "final_table": 6 + count * 140 * 10},
        }
    with args.output.open("x", encoding="utf8") as file:
        json.dump(report, file, ensure_ascii=False, separators=(",", ":"))
        file.write("\n")


if __name__ == "__main__":
    main()

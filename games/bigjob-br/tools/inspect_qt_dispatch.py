"""Disassemble embedded NE QuickTime entrypoints as data via LLVM."""
import io
import json
import re
import struct
import subprocess
from contextlib import redirect_stdout
from pathlib import Path

with redirect_stdout(io.StringIO()):
    from inspect_qt_payload import decode, installer

ROOT = Path(__file__).resolve().parents[1]


def word(data, pos):
    return struct.unpack_from("<H", data, pos)[0]


def coff(code):
    header = struct.pack("<HHIIIHH", 0x14C, 1, 0, 0, 0, 0, 0)
    section = struct.pack("<8sIIIIIIHHI", b".text", 0, 0, len(code), 60, 0, 0, 0, 0, 0x60000020)
    return header + section + code


for name, offset in (("qtim", 0x6DB00), ("qtimcmgr", 0x48400),
                     ("qcmc", 0x4AB00), ("navg", 0x54000)):
    data = decode(installer[offset:])
    ne, = struct.unpack_from("<I", data, 0x3C)
    segment_table = ne + word(data, ne + 0x22)
    alignment = word(data, ne + 0x32)
    segments = []
    for index in range(word(data, ne + 0x1C)):
        sector, length, flags, minimum = struct.unpack_from("<4H", data, segment_table + 8 * index)
        begin = sector << alignment
        segments.append((begin, length or 65536, flags))
    position = ne + word(data, ne + 4)
    ordinal = 1
    exports = {}
    while count := data[position]:
        segment = data[position + 1]
        position += 2
        for _ in range(count):
            if segment == 255:
                exports[ordinal] = (data[position + 3], word(data, position + 4))
                position += 6
            elif segment:
                exports[ordinal] = (segment, word(data, position + 1))
                position += 3
            ordinal += 1
    print(name, exports.get(2), len(segments))
    if name == "qtim":
        automatic_data = word(data, ne + 0x0e)
        begin, length, flags = segments[automatic_data - 1]
        print("DGROUP", automatic_data, hex(begin), hex(flags))
        targets = {}
        cursor = begin + length
        count = word(data, cursor)
        for i in range(count):
            source_type, relocation_type, site, target_segment, target_offset = struct.unpack_from("<BBHHH", data, cursor + 2 + i * 8)
            seen = set()
            while site != 0xFFFF and site not in seen and site + 2 <= length:
                seen.add(site)
                if relocation_type & 3 == 0:
                    targets[site] = (target_segment, target_offset)
                if relocation_type & 4:
                    break
                site = word(data, begin + site)
        for selector in (7, 11, 12, 28, 41, 47, 49, 53, 55, 56, 57, 58):
            site = 0x180 + selector * 4
            target_segment, _ = targets[site + 2]
            print(f"QTIM/{selector:02X}: {automatic_data}:{site:04X} -> {target_segment}:{word(data, begin + site):04X}")
    if name in ("qcmc", "navg"):
        segment, table = {"qcmc": (3, 0x26F0), "navg": (2, 0x2760)}[name]
        begin, _, _ = segments[segment - 1]
        for selector in (0x01, 0x05, 0x0B, 0x13, 0x14):
            site = table + selector * 2
            print(f"{name.upper()}/{selector:02X}: {segment}:{site:04X} -> {segment}:{word(data, begin + site):04X}")
    folder = ROOT / "build" / "qt-analysis" / name
    folder.mkdir(parents=True, exist_ok=True)
    for index, (begin, length, flags) in enumerate(segments, 1):
        if flags & 1:
            continue
        obj = folder / f"{index:02}.obj"
        obj.write_bytes(coff(data[begin:begin + length]))
        listing = subprocess.check_output(["llvm-objdump", "-d", "--mattr=+16bit-mode", str(obj)], text=True)
        if flags & 0x100:
            records = begin + length
            notes = {}
            for row in range(word(data, records)):
                source, kind, site, target, value = struct.unpack_from("<BBHHH", data, records + 2 + row * 8)
                seen = set()
                while site != 0xFFFF and site not in seen and site + 2 <= length:
                    seen.add(site)
                    if kind & 3 == 0:
                        if target == 255:
                            label = str(exports.get(value))
                        else:
                            label = f"{target}:{value:04x}"
                    elif kind & 3 in (1, 2):
                        references = ne + word(data, ne + 0x28)
                        imported = ne + word(data, ne + 0x2a)
                        start = imported + word(data, references + (target - 1) * 2)
                        module = data[start + 1:start + 1 + data[start]].decode("ascii", "replace")
                        label = f"{module}!{value}"
                    else:
                        label = "OS"
                    notes[site] = label
                    if kind & 4:
                        break
                    site = word(data, begin + site)
            expanded = []
            for line in listing.splitlines():
                match = re.match(r"\s*([0-9a-f]+):\s+((?:[0-9a-f]{2} )+)", line)
                if match:
                    addr = int(match[1], 16)
                    fixups = [f"{site:04x}={notes[site]}" for site in range(addr, addr + len(match[2].split())) if site in notes]
                    if fixups:
                        line += " ; NE " + ", ".join(fixups)
                expanded.append(line)
            listing = "\n".join(expanded) + "\n"
        obj.with_suffix(".asm").write_text(listing, encoding="utf8")
    (folder / "exports.json").write_text(json.dumps(exports, indent=2), encoding="utf8")

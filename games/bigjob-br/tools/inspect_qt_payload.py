"""Identify embedded QuickTime modules using only reads from the CD image."""
import json
import hashlib
import re
import struct
from pathlib import Path

root = Path(__file__).resolve().parents[1]
inventory = json.loads((root / "analysis/inventory.json").read_text(encoding="utf-8-sig"))
entry = next(item for item in inventory["files"] if item["path"] == "QTW/QTINSTAL.EXE")
with open(inventory["iso"], "rb") as image:
    image.seek(entry["lba"] * 2048)
    installer = image.read(entry["bytes"])


def decode(block):
    size, = struct.unpack_from("<I", block, 10)
    ring = bytearray(b" " * 4096)
    cursor = 4080
    source = 14
    output = bytearray()
    while len(output) < size:
        mask = block[source]
        source += 1
        for bit in range(8):
            if mask & (1 << bit):
                values = (block[source],)
                source += 1
            else:
                low, high = block[source:source + 2]
                source += 2
                start = low | ((high & 240) << 4)
                values = (ring[(start + delta) & 4095] for delta in range((high & 15) + 3))
            for value in values:
                output.append(value)
                ring[cursor] = value
                cursor = (cursor + 1) & 4095
                if len(output) == size:
                    return output


for match in re.finditer(bytes.fromhex("535a444488f0273341"), installer):
    offset = match.start()
    payload = decode(installer[offset:])
    if payload[:2] != b"MZ":
        continue
    header, = struct.unpack_from("<I", payload, 0x3C)
    if payload[header:header + 2] != b"NE":
        continue
    names, = struct.unpack_from("<H", payload, header + 0x26)
    start = header + names
    name = bytes(payload[start + 1:start + 1 + payload[start]])
    print(f"{offset:#08x} {len(payload):>8} {name.decode('ascii', errors='replace')}")
    if name in (b"QTIM", b"QTIMCMGR", b"QTNOTIFY"):
        nonresident, = struct.unpack_from("<I", payload, header + 0x2C)
        for position in (start, nonresident):
            while position and payload[position]:
                length = payload[position]
                label = bytes(payload[position + 1:position + 1 + length])
                ordinal, = struct.unpack_from("<H", payload, position + 1 + length)
                if ordinal and any(token in label.upper() for token in (b"ENTRYPOINT", b"CMGR", b"CLEANUP")):
                    print(ordinal, label.decode("ascii", errors="replace"))
                position += length + 3
        print(hashlib.sha256(payload).hexdigest())

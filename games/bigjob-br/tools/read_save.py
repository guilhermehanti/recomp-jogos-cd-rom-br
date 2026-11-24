"""Read-only, bounded decoder for BIGJOB.SAV, recovered statically from NE 6.

No original executable is loaded or executed. Names remain raw bytes because the
Windows code page is a presentation choice, not part of the container format.
"""
from dataclasses import dataclass
import argparse
import json
from pathlib import Path
import struct


@dataclass(frozen=True)
class Card:
    tool: int
    name: bytes
    flags: bytes
    progress: bytes
    counters: bytes


def read_save(data: bytes, *, max_cards: int = 50) -> tuple[Card, ...]:
    if len(data) < 8:
        raise ValueError("truncated header")
    if data[:2] == b"II":
        order = "<"
    elif data[:2] == b"MM":
        order = ">"
    else:
        raise ValueError("invalid endian marker")
    magic, version, count = struct.unpack_from(order + "HHh", data, 2)
    if magic != 0xF0AD:
        raise ValueError("invalid magic")
    if version != 10:
        raise ValueError("unsupported save version")
    if not 0 <= count <= max_cards:
        raise ValueError("card count exceeds decoder policy")
    if len(data) != 8 + count * 77:
        raise ValueError("truncated records or trailing bytes")
    cards = []
    for index in range(count):
        start = 8 + index * 77
        tool = struct.unpack_from(order + "h", data, start)[0]
        if not 0 <= tool <= 12:
            raise ValueError(f"invalid tool in card {index}")
        record = data[start:start + 77]
        cards.append(Card(tool, record[2:14], record[14:50],
                          record[50:68], record[68:77]))
    return tuple(cards)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("save", type=Path)
    args = parser.parse_args()
    cards = read_save(args.save.read_bytes())
    print(json.dumps([{"tool": c.tool, "name_hex": c.name.hex(),
                       "flags": list(c.flags), "progress": list(c.progress),
                       "counters": list(c.counters)} for c in cards], indent=2))


if __name__ == "__main__":
    main()

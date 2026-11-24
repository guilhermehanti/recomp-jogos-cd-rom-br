"""Render the first MMFW Pictures v0 resource using PCRecomp's loaded index."""
import argparse
import json
from pathlib import Path
import struct
import subprocess
import zlib


def render(root, output, index=0):
    report = json.loads((root / "analysis/inventory.json").read_text(encoding="utf8"))
    bank = next(row for row in report["files"] if row["path"] == "BIGJOBHD.PIC")
    base = bank["lba"] * 2048
    loaded = subprocess.check_output(
        [str(root / "build/pic_open_auto.exe"), report["iso"], str(base), str(bank["bytes"])],
        text=True,
    )
    count, *offsets = map(int, loaded.splitlines())
    if len(offsets) != count + 1:
        raise ValueError("Incomplete PIC index")
    if not 0 <= index < count:
        raise ValueError('Resource index outside bank')
    with Path(report["iso"]).open("rb") as iso:
        def read(offset, size):
            if not 0 <= offset <= offset + size <= bank["bytes"]:
                raise ValueError("Read outside the PIC bank")
            iso.seek(base + offset)
            data = iso.read(size)
            if len(data) != size:
                raise ValueError("Truncated PIC bank")
            return data

        def word(offset):
            return struct.unpack(">H", read(offset, 2))[0]

        picture = read(offsets[index], offsets[index + 1] - offsets[index])
        width, height, kind = struct.unpack_from(">HHH", picture)
        if not (1 <= width <= 640 and 1 <= height <= 480 and kind in (0x54, 0x55, 0x56)):
            raise ValueError("Unexpected picture format")
        palette_table = 108 + 52 * count
        palette_id = word(palette_table - 2 * count + 2 * index)
        palette_count = word(palette_table)
        if not palette_id < palette_count or palette_table + 2 + 1024 * palette_count > offsets[0]:
            raise ValueError("Invalid palette table")
        palette = read(palette_table + 2 + 1024 * palette_id, 1024)

    row_format = "I" if kind == 0x55 else "H"
    rows = [*struct.unpack_from(f">{height}{row_format}", picture, 8), len(picture)]
    if rows[0] != 8 + struct.calcsize(row_format) * height:
        raise ValueError("Invalid row table")
    scanlines = bytearray()
    for start, end in zip(rows, rows[1:]):
        if not rows[0] <= start < end <= len(picture):
            raise ValueError("Invalid row span")
        row = bytearray()
        while len(row) < width and start < end:
            code = picture[start]
            start += 1
            if code < 128 and start < end:
                row.extend(bytes([picture[start]]) * (code + 1))
                start += 1
            elif 128 <= code < 255 and start + code - 126 <= end:
                length = code - 126
                row.extend(picture[start:start + length])
                start += length
            else:
                raise ValueError("Invalid RLE packet")
        if len(row) != width:
            raise ValueError(f"Invalid decoded row width: {len(row)}")
        scanlines.extend(b"\0" + row)

    def chunk(kind, data):
        return (struct.pack(">I", len(data)) + kind + data
                + struct.pack(">I", zlib.crc32(kind + data)))

    rgb = b"".join(palette[i + 1:i + 4] for i in range(0, 1024, 4))
    png = (b"\x89PNG\r\n\x1a\n"
           + chunk(b"IHDR", struct.pack(">IIBBBBB", width, height, 8, 3, 0, 0, 0))
           + chunk(b"PLTE", rgb)
           + (chunk(b"tRNS", bytes([0])) if kind == 0x56 else b"")
           + chunk(b"IDAT", zlib.compress(scanlines))
           + chunk(b"IEND", b""))
    with output.open("xb") as file:
        file.write(png)
    print(f"{width}x{height}, palette {palette_id}/{palette_count}, offsets {offsets[index:index + 2]}")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("output", type=Path)
    parser.add_argument("--index", type=int, default=0)
    arguments = parser.parse_args()
    render(Path(__file__).resolve().parents[1], arguments.output, arguments.index)




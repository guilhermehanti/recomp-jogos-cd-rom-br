"""Locate and validate the Brazilian workshop curtain sequence in the CD image."""
import argparse
import json
import struct
from pathlib import Path


def inspect_curtain(script):
    if not script.startswith(b"MMScript File\0"):
        raise ValueError("Invalid MMS header")
    signature = bytes.fromhex("c002 000d 00ca 0000 0077 00d2 c006 000e")
    candidates = []
    cursor = 0
    while (position := script.find(signature, cursor)) >= 0:
        cursor = position + 1
        rows = []
        for frame in range(1, 7):
            start = position + (frame - 1) * 36
            record = script[start - 12:start + 24]
            if (start < 12 or len(record) != 36
                    or record[:4] != bytes.fromhex("c002 000c")
                    or record[12:28] != signature):
                break
            delay = struct.unpack_from(">h", record, 4)[0]
            y, x = struct.unpack_from(">HH", record, 20)
            ordinal = struct.unpack_from(">H", record, 30)[0]
            if ordinal != frame:
                break
            rows.append({"offset": start, "frame": ordinal, "x": x, "y": y,
                         "raw_000c_value": delay})
        if len(rows) == 6:
            candidates.append(rows)
    if len(candidates) != 1:
        raise ValueError(f"Expected one complete curtain sequence, found {len(candidates)}")
    return candidates[0]


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    root = Path(__file__).resolve().parents[1]
    parser.add_argument("--inventory", type=Path, default=root / "analysis/inventory.json")
    args = parser.parse_args()
    inventory = json.loads(args.inventory.read_text(encoding="utf8"))
    entry = next(row for row in inventory["files"] if row["path"] == "BIGJOB.MMS")
    with Path(inventory["iso"]).open("rb") as image:
        image.seek(entry["lba"] * 2048)
        script = image.read(entry["bytes"])
    if len(script) != entry["bytes"]:
        raise ValueError("Incomplete MMS extent")
    print(json.dumps(inspect_curtain(script), indent=2))

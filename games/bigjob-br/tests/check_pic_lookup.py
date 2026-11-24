from pathlib import Path
import hashlib
import json
import struct
import subprocess

root = Path(__file__).resolve().parents[1]
db = json.loads((root / "analysis/inventory.json").read_text(encoding="utf8"))
files = {row["path"]: row for row in db["files"]}
binary = files["BIGJOBNG.EXE"]
segment = db["NE"]["BIGJOBNG.EXE"]["segments"][33]
request, expected = [], []
with Path(db["iso"]).open("rb") as iso:
    iso.seek(binary["lba"] * 2048)
    executable = iso.read(binary["bytes"])
    assert hashlib.sha256(executable).hexdigest() == binary["sha256"]
    origin = segment["offset"]
    actual = executable[origin + 0x147B:origin + 0x149D]
    assert actual == bytes.fromhex(
        "8b460a40c1e00203d8268b5702268b078b5ef88b760ac1e60203de262b07261b5702"
    )
    for name in ("BIGJOBCD.PIC", "BIGJOBHD.PIC"):
        iso.seek(files[name]["lba"] * 2048 + 102)
        count, = struct.unpack(">H", iso.read(2))
        table = struct.unpack(f">{count + 1}I", iso.read(4 * (count + 1)))
        reference = db["media"][name]["offsets"]
        assert len(reference) == count + 1
        request.append(" ".join(map(str, (count, *table))))
        expected.extend(["invalid", *(f"{a} {b-a}" for a,b in zip(reference, reference[1:])), "invalid"])
    # Borrow from the low word, and zero-length record, respectively.
    request.extend(["1 65535 65537", "1 65536 65536"])
    expected.extend(["invalid", "65535 2", "invalid", "invalid", "65536 0", "invalid"])

result = subprocess.run([str(root / "build/pic_lookup.exe")], input="\n".join(request),
                        text=True, capture_output=True, check=True)
assert result.stdout.splitlines() == expected
print("PASS: 4744 disc entries, 4 disc bounds checks, and 2 synthetic cases.")


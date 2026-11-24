"""Verify static evidence against the immutable Brazilian CD image."""
import hashlib
import json
from pathlib import Path
import struct

from trace_mms import runtime_words


def main():
    root = Path(__file__).resolve().parents[1]
    inventory = json.loads((root / "analysis/inventory.json").read_text(encoding="utf-8-sig"))
    mapping = json.loads((root / "analysis/mapa-jogo-v3.json").read_text(encoding="utf-8-sig"))
    files = {item["path"]: item for item in inventory["files"]}
    verified = []
    with Path(inventory["iso"]).open("rb") as image:
        def read(name):
            entry = files[name]
            image.seek(entry["lba"] * 2048)
            data = image.read(entry["bytes"])
            assert len(data) == entry["bytes"], name
            digest = hashlib.sha256(data).hexdigest()
            if "sha256" in entry:
                assert digest.lower() == entry["sha256"].lower(), name
            verified.append({"path": name, "size": len(data), "sha256": digest})
            return data

        executable = read("BIGJOBNG.EXE")
        assert executable == (root / "build/BIGJOBNG.EXE").read_bytes()
        script = read("BIGJOB.MMS")
        count = 0
        for block in mapping["mms"]["blocks"] + mapping["mms"]["tables_with_strings"]:
            for record in block["records"]:
                raw = script[record["offset"]:record["offset"] + 12]
                assert list(struct.unpack(">6H", raw)) == record["words"]
                loaded = b"".join(raw[index:index + 4][::-1] for index in range(0, 12, 4))
                assert list(struct.unpack("<6H", loaded)) == runtime_words(record["words"])
                count += 1
        cabin = next(record for block in mapping["mms"]["blocks"]
                     for record in block["records"] if record["offset"] == 0x23aff)
        assert runtime_words(cabin["words"])[0:6:2] == [17, 4, 231]
        movie_entry = files["BIGJOB.MOV"]
        image.seek(movie_entry["lba"] * 2048)
        header = image.read(4 + 58 * 4)
        assert struct.unpack_from(">I", header)[0] == 58
        assert list(struct.unpack_from(">58I", header, 4)) == [movie["moov"] for movie in mapping["movies"]]
    segment = mapping["executables"]["BIGJOBNG.EXE"]["segments"][48]
    award = list(struct.unpack_from("<36H", executable, segment["offset"] + 0x204))
    selectors = [[14, 6, 52, 47, 38, 39, 1], [10, 28, 50, 53, 37, 33, 30],
                 [3, 12, 16, 42, 26, 45, 35], [20, 18, 8], [2, 9, 19, 22],
                 [value for value in award if value != 0xffff]]
    all_movies = [value for group in selectors for value in group]
    assert len(all_movies) == 58 and sorted(all_movies) == list(range(58))
    result = {"verified_files": verified, "records_compared_to_iso": count,
              "movie_header_entries_verified": 58, "movie_selection_partition_verified": True,
              "cabin_callback_4_verified": True, "original_code_executed": False}
    (root / "analysis/behavior-verification.json").write_text(json.dumps(result, indent=2), encoding="utf8")
    print(json.dumps(result, ensure_ascii=False, indent=2))


if __name__ == "__main__":
    main()

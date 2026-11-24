"""Decode MMS control flow using the original loader's DWORD byte reversal."""
import json
from pathlib import Path
import struct


def runtime_words(words):
    disk = struct.pack(">6H", *words)
    loaded = b"".join(disk[pos:pos + 4][::-1] for pos in range(0, 12, 4))
    return list(struct.unpack("<6H", loaded))


def main():
    root = Path(__file__).resolve().parents[1]
    mapping = json.loads((root / "analysis/mapa-jogo-v3.json").read_text(encoding="utf-8-sig"))
    blocks = {block["start"]: block for block in mapping["mms"]["blocks"]}
    entries = blocks[18]["records"]
    results = []
    for entry in entries:
        target = entry.get("candidate_block")
        if target not in blocks or entry["words"][0] != 0x8003:
            continue
        block = blocks[target]
        records = []
        labels = {}
        for index, record in enumerate(block["records"]):
            words = runtime_words(record["words"])
            item = {"index": index, "offset": record["offset"], "disk_words": record["words"],
                    "runtime_words_before_reference_resolution": words, "opcode": words[0]}
            if words[0] == 12:
                duration = words[3] - 0x10000 if words[3] >= 0x8000 else words[3]
                item.update(label=words[2], duration_signed=duration)
                labels.setdefault(words[2], []).append(index)
            if words[0] == 16:
                sound = words[2]
                bank = sound >> 14
                sound_index = sound & 0x3fff
                item.update(sound_id=sound, bank=bank, sound_index=sound_index)
                bank_name = {0: "BIGJOB.SND", 1: "BIGVOX.SND"}.get(bank)
                if bank_name and sound_index < len(mapping["banks"][bank_name]):
                    item["sound_name"] = mapping["banks"][bank_name][sound_index]["name"]
                    item["sound_bank"] = bank_name
            if words[0] == 17:
                item["command_slot"] = words[2]
            if words[0] == 18:
                item["jump_label"] = words[2]
            if words[0] == 19:
                item.update(condition_object=words[2], jump_label=words[3])
            records.append(item)
        for item in records:
            if "jump_label" in item:
                item["target_indices"] = labels.get(item["jump_label"], [])
                item["jump_to_end"] = item["jump_label"] == 0xffff
        results.append({"root_id": entry["words"][1], "block": target, "labels": labels,
                        "records": records})
    cabin = next(script for script in results if script["root_id"] == 0x111)
    callback = next(record for record in cabin["records"] if record["offset"] == 0x23aff)
    assert callback["command_slot"] == 4
    assert runtime_words([0xc002, 12, 0xffff, 1, 0, 0])[:4] == [12, 0xc002, 1, 0xffff]
    output = root / "analysis/mms-control-flow.json"
    output.write_text(json.dumps(results, ensure_ascii=False, indent=2), encoding="utf8")
    print(f"Decoded {len(results)} root scripts. Verified cabin command slot 4 at 0x23AFF.")
    print(f"Sound triggers: {sum('sound_id' in r for s in results for r in s['records'])}")


if __name__ == "__main__":
    main()

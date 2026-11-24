"""Annotate static LLVM listings with expanded NE relocation chains."""
import json
from pathlib import Path
import re
import struct
import subprocess


def main():
    root = Path(__file__).resolve().parents[1]
    mapping = json.loads((root / "analysis/mapa-jogo-v3.json").read_text(encoding="utf-8-sig"))
    binary = (root / "build/BIGJOBNG.EXE").read_bytes()
    output = root / "analysis/disassembly"
    output.mkdir(exist_ok=True)
    references = []
    for segment in mapping["executables"]["BIGJOBNG.EXE"]["segments"]:
        data = binary[segment["offset"]:segment["offset"] + segment["size"]]
        sites = {}
        for source_type, flags, site, target_segment, target_offset, module, label in segment["fixups"]:
            visited = set()
            while site != 0xffff:
                if site in visited or site + 2 > len(data):
                    raise ValueError(f"Invalid relocation chain {segment['id']}:{site:04X}")
                visited.add(site)
                target = (f"{target_segment}:{target_offset:04X}" if flags & 3 == 0
                          else f"{module}!{label}")
                sites.setdefault(site, []).append(target)
                references.append({"segment": segment["id"], "site": site,
                                   "source_type": source_type, "flags": flags,
                                   "target": target, "data_segment": bool(int(segment["flags"], 16) & 1)})
                if flags & 4:
                    break
                site = struct.unpack_from("<H", data, site)[0]
        obj = root / f"build/seg{segment['id']}.obj"
        if not obj.exists():
            continue
        listing = subprocess.check_output(["llvm-objdump", "-d", "--mattr=+16bit-mode", str(obj)], text=True)
        lines = []
        for line in listing.splitlines():
            match = re.match(r"\s*([0-9a-f]+):\s+((?:[0-9a-f]{2} )+)", line)
            if match:
                start = int(match[1], 16)
                end = start + len(match[2].split())
                notes = [f"{site:04X}={target}" for site in range(start, end)
                         for target in sites.get(site, [])]
                if notes:
                    line += " ; NE " + ", ".join(notes)
            lines.append(line)
        (output / f"seg{segment['id']:02d}.asm").write_text("\n".join(lines) + "\n", encoding="utf8")
    (output / "references.json").write_text(json.dumps(references, indent=2), encoding="utf8")
    assert any(r["segment"] == 2 and r["site"] == 0x2265 and r["target"] == "33:30BF" for r in references)
    print(f"Validated {len(references)} relocation sites. Listings in {output}")


if __name__ == "__main__":
    main()

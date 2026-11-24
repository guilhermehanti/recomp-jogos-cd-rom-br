"""Index statically visible entries, calls and guards for the native audio audit."""
import json
import re
from collections import defaultdict
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
ANALYSIS = ROOT / "analysis"
LINE = re.compile(r"^\s*([0-9a-f]+):\s+(?:[0-9a-f]{2} +)+\s*(.*)$")
EDGE = re.compile(r"^(j\w+|callw|loop\w*)\s+0x([0-9a-f]+)\b")


def read_listings():
    listings = {}
    incoming = defaultdict(list)
    for path in sorted((ANALYSIS / "disassembly").glob("seg*.asm")):
        segment = int(path.stem[3:])
        instructions = {}
        for line in path.read_text().splitlines():
            match = LINE.match(line)
            if not match:
                continue
            offset, instruction = int(match[1], 16), match[2]
            instructions[offset] = instruction
            edge = EDGE.match(instruction)
            if edge:
                incoming[segment, int(edge[2], 16)].append((offset, edge[1]))
        listings[segment] = instructions
    return listings, incoming


def main():
    sites = json.loads((ANALYSIS / "native-audio-closure.json").read_text(encoding="utf8"))["calls"]
    refs = json.loads((ANALYSIS / "disassembly/references.json").read_text())
    listings, incoming = read_listings()
    target = ANALYSIS / "34-AUDIO-EVENTOS.md"
    marker = "\n## Vínculos demonstrados\n"
    previous = target.read_text(encoding="utf8") if target.exists() else ""
    semantic = previous.split(marker, 1)[1].rstrip() if marker in previous else ""
    proved = {site for line in semantic.splitlines() if line.startswith("| ")
              for site in re.findall(r"\b\d+:[0-9A-F]{4}\b", line.split("|")[1])}
    interfaces = {"5:0A3A", "30:228C", "36:291E"}
    no_producer = {"10:13D7"}
    pending = len(sites) - len(proved - interfaces) - len(interfaces)
    output = ["# Evidência de entrada das chamadas nativas de áudio", "",
              f"Situação atual: {len(proved - interfaces)} sites com gatilho local descrito, {len(interfaces)} interfaces e {len(no_producer)} sem produtor direto identificado. Pendências de classificação: {pending - len(no_producer)}.", "",
              "A tabela cobre os 85 pontos catalogados. A entrada é a âncora local de prólogo. Referências NE podem ser ponteiros armazenados para despacho posterior. A janela lista saltos locais anteriores ao site e seus destinos, sem assumir que todo salto mostrado domina a chamada. A comprovação do evento da interface exige seguir o controlador proprietário.", "",
              "| Site | Situação | Entrada | Entradas diretas observadas | Saltos locais na janela anterior |",
              "|---|---|---|---|---|"]
    for row in sites:
        seg = row["segment"]
        site = int(row["offset"], 16)
        anchor = row["function_anchor"]
        if (seg, site) == (9, 0x1AFC):
            anchor = "9:145E"
        entrance = int(anchor.split(":")[1], 16)
        listing = listings[seg]
        # The linear listing folds 145E and 145F into the trailing jump-table word.
        assert site in listing and (entrance in listing or (seg, entrance) == (9, 0x145E))
        near = [f"{seg}:{caller:04X} ({kind})" for caller, kind in incoming[seg, entrance]]
        far = [f"{item['segment']}:{item['site']:04X} (NE)" for item in refs if item["target"] == anchor]
        candidates = [(offset, EDGE.match(instruction)) for offset, instruction in listing.items()
                      if max(entrance, site - 0x80) <= offset < site]
        guards = [f"{offset:04X} {match[1]} {int(match[2],16):04X}" for offset, match in candidates
                  if match and not match[1].startswith(("call", "jmp"))]
        address = f"{seg}:{site:04X}"
        status = "Pendente"
        if address in interfaces:
            status = "Interface"
        elif address in proved:
            status = "Gatilho local descrito"
        elif address in no_producer:
            status = "Sem produtor direto identificado"
        output.append(f"| {address} | {status} | {anchor} | {', '.join(near+far) or '—'} | {', '.join(guards) or '—'} |")
    actual = {f"{row['segment']}:{row['offset']}" for row in sites}
    assert proved <= actual and interfaces <= actual and no_producer <= actual
    if semantic:
        output.append(marker + semantic)
    target.write_text("\n".join(output) + "\n", encoding="utf8")


if __name__ == "__main__":
    main()

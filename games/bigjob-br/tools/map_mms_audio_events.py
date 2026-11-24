"""Catalog every MMS audio instruction and its structural predecessors."""
import json
import re
import struct
from collections import deque
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
ANALYSIS = ROOT / "analysis"


def graph(records):
    labels = {}
    for index, record in enumerate(records):
        if "label" in record:
            labels.setdefault(record["label"], index)
    edges = [[] for _ in records]
    for index, record in enumerate(records):
        opcode = record["opcode"]
        if opcode != 18 and index + 1 < len(records):
            edges[index].append((index + 1, "slot ocupado" if opcode == 19 else "avanço"))
        if opcode in (18, 19):
            label = record["jump_label"]
            if label != 0xffff and label in labels:
                edges[index].append((labels[label], "slot ocioso" if opcode == 19 else "salto"))
    return labels, edges


def backwards(index, predecessors):
    visited = {index}
    queue = deque([index])
    while queue:
        for source in predecessors[queue.popleft()]:
            if source not in visited:
                visited.add(source)
                queue.append(source)
    return visited


def main():
    scripts = json.loads((ANALYSIS / "mms-control-flow.json").read_text(encoding="utf8"))
    rows = []
    for script in scripts:
        records = script["records"]
        labels, edges = graph(records)
        reverse = [[] for _ in records]
        for source, successors in enumerate(edges):
            for target, _ in successors:
                reverse[target].append(source)
        roots = {index: label for label, index in labels.items() if label != 0xffff}
        default = next(index for index, record in enumerate(records) if record["opcode"] >= 12)
        for index, record in enumerate(records):
            if record["opcode"] != 16:
                continue
            reachable = backwards(index, reverse)
            # Stop at the first selector on each reverse path. An entry can
            # retain the runtime's prior slot state if no selector intervenes.
            seen = {index}
            pending = deque([index])
            selectors = set()
            inherited = set()
            while pending:
                current = pending.popleft()
                if records[current]["opcode"] == 15:
                    selectors.add(current)
                    continue
                if current in roots or current == default:
                    inherited.add(current)
                for parent in reverse[current]:
                    if parent not in seen:
                        seen.add(parent)
                        pending.append(parent)
            rows.append({
                "root_id": script["root_id"], "index": index, "offset": record["offset"],
                "sound_id": record["sound_id"], "sound_name": record.get("sound_name"),
                "first_frame_path": default in reachable,
                "candidate_entry_labels": sorted(label for source, label in roots.items()
                                                 if source in reachable),
                "selector_predecessors": [
                    {"offset": records[source]["offset"],
                     "runtime_words": records[source]["runtime_words_before_reference_resolution"]}
                    for source in sorted(selectors)],
                "entry_can_retain_slot": bool(inherited),
                "preceding_slot_tests": [
                    {"offset": records[source]["offset"],
                     "slot": records[source]["condition_object"],
                     "edges_to_sound": [condition for target, condition in edges[source]
                                        if target in reachable]}
                    for source in sorted(reachable) if records[source]["opcode"] == 19],
            })
    assert len(rows) == 441
    assert len({(row["root_id"], row["index"]) for row in rows}) == len(rows)
    mapping = json.loads((ANALYSIS / "mapa-jogo-v3.json").read_text(encoding="utf-8-sig"))
    segment = mapping["executables"]["BIGJOBNG.EXE"]["segments"][48]
    executable = (ROOT / "build/BIGJOBNG.EXE").read_bytes()
    help_main = struct.unpack_from("<25H", executable, segment["offset"] + 0x3a2)
    help_alternate = struct.unpack_from("<25H", executable, segment["offset"] + 0x3d4)
    help_rows = [
        {"scene_argument": scene, "default_virtual_48_root": root, "alternate_root": alternative}
        for scene, (root, alternative) in enumerate(zip(help_main, help_alternate))
    ]
    constructor_refs = []
    # This exact argument setup encodes a packed ID before the object's far pointer.
    # Other callers use dynamic registers, WORD pushes, or table operands.
    for path in sorted((ANALYSIS / "disassembly").glob("seg*.asm")):
        lines = path.read_text().splitlines()
        for index in range(3, len(lines)):
            call = re.search(r"^\s*([0-9a-f]+):.*NE [A-F0-9]+=30:04DA\b", lines[index])
            literal = re.search(r"\bpushl\s+\$0x([0-9a-f]+)\b", lines[index - 3])
            if not call or not literal:
                continue
            if not all(re.search(r"\bpushw\s+", line) for line in lines[index - 2:index]):
                continue
            value = int(literal[1], 16)
            constructor_refs.append({
                "segment": int(path.stem[3:]), "call_offset": int(call[1], 16),
                "packed_argument": value, "root_id": value & 0xffff,
                "setup": [line.strip() for line in lines[index - 3:index + 1]],
            })
    specific_sources = {
        "03:1E0A": [0x171, 0x172],
        "03:11E7": list(struct.unpack_from("<6H", executable, segment["offset"] + 0x258)),
        "08:0E44-0F2F": [0x141, 0x142, 0x143, 0x145, 0x146, 0x147, 0x148, 0x14F, 0x153],
        "08:02D8": [0x144],
        "08:12DA-1389": [0x149, 0x14A, 0x14B, 0x14C, 0x14D, 0x14E, 0x150, 0x151, 0x152],
        "09:1EB5": [0x18D, 0x18E, 0x18F],
        "11:0694-06DC": [0x124, 0x126, 0x130, 0x13A],
        "17:22C4": [0x11C, 0x11D, 0x11E, 0x11F],
        "18:38A2": [0x184, 0x185],
        "23:1F70": [0xD9, 0x133, 0x134, 0x135],
    }
    mapped = {ref["root_id"] for ref in constructor_refs} | set(help_main) | set(help_alternate)
    mapped.update(root for ids in specific_sources.values() for root in ids)
    audio_roots = {row["root_id"] for row in rows}
    assert len(audio_roots) == 98 and audio_roots <= mapped
    sequence_roots = set().union(*(specific_sources[key] for key in (
        "08:0E44-0F2F", "08:02D8", "08:12DA-1389", "11:0694-06DC")))
    entries = [
        ("Montagem na oficina ou CS", sequence_roots, [1], "05:0DE5-0DEB"),
        ("Interação com quatro regiões da oficina", [0x141, 0x14F], [2, 3],
         "08:0F86,1068,114A,122C"),
        ("Ajuda padrão", set(help_main) | {0x18D, 0x18E, 0x18F}, [1, 3],
         "05:0F87,0F9F"),
        ("Ajuda alternativa", set(help_alternate), [1, 3], "05:13F9,140D,1479,14A9"),
        ("CP, total de certificados no perfil", [0x173], [6, 8, 10, 12],
         "09:1944-1989,1E68"),
        ("Oficina, segundo roteiro de apresentação", [0x17D], [1], "08:19F0,1D01,1D15"),
        ("Confirmação, mudança de seleção", [0x113], [2, 3, 4, 5],
         "03:2595,28BD,2920,29AF,2A14;08:1F9B,2295,22F8,2387,23EC"),
        ("Atividades da base, conclusão", [0x112], [1], "05:17AF,1829,1A52-1A81"),
        ("Atividades da base, falha", [0x116], [1], "05:1871,18E0,1A9B-1AA5"),
        ("FM, captura da pessoa no alvo", [0x1B5], [1], "17:2199,21CE,228C,229C"),
        ("FL, pré-atividade de explosão", [0x1B6], [1], "16:0BE1,0C42,137A"),
        ("Oficina, montagem do cartaz", [0x13C], [1], "08:0B2F,0BB3"),
        ("SB, seleção de jato por tipo de célula", [0xD9, 0x133, 0x134, 0x135], [1],
         "23:1F70,2320,2434"),
        ("Oficina/CS, seleção pelo wrapper da animação",
         sequence_roots - {0x141, 0x142, 0x14F, 0x13A}, [2], "05:0D77,0DCF,0DDD;30:280C"),
        ("Oficina/CS, sorteio pelo wrapper da animação",
         [0x141, 0x142, 0x14F, 0x13A], [2, 3], "05:0DB3,0DCF,0DDD;30:280C"),
        ("Oficina, passos do personagem em ciclo de espera", [0x154], list(range(2, 11)),
         "08:0DB7,0E0F,2547-25A8"),
        ("Oficina, diálogo de saída e continuação", [0x11B], [1, 2, 5, 7],
         "08:13BF,141F,1433,15D9,16A6"),
        ("FL, início de condução", [0x117], [1, 2, 4], "16:0C8B,13AF,16E1,1773"),
        ("FL, reação variável durante movimento", [0x117], [6, 7, 8, 9],
         "16:32BE-32F0,34A3-34E7"),
        ("FM, sequência de resgate por direção", [0x11C, 0x11D, 0x11E, 0x11F], [1],
         "17:22C4-244B"),
        ("GW, tutorial escolhido pelo virtual +48", [0x184, 0x185], [1, 3],
         "18:38A2;05:0EF1,0F87,0F9F"),
        ("CP, controlador principal", [0x173], [1, 2], "09:02BE,129B,1BDE,1CDC"),
        ("TC, variante de abertura", [0x136], [1, 2], "25:0A3E,0A7D,0AAA"),
        ("TC, controlador do formulário", [0x121], [1, 2, 3, 4, 5],
         "25:2672,2B0E,2C33,2CC1,2DE7,2E6A"),
        ("Impressão, instruções", [0x132], [1], "26:05A8,0644"),
        ("SB, início do MMS de seleção", [0x139], [1], "20:0BC1,0C86"),
        ("SB, início de MMS adicional", [0x1CA], [1], "20:0EDA,0F9F"),
        ("Painel de recompensa, roteiro pelo ID de progresso", specific_sources["03:11E7"],
         [1, 99], "03:11E7,09D9,09F2"),
        ("MMS do painel em 3:1E0A", [0x171, 0x172], [1], "03:1E0A,1E5D"),
        ("MMS do controlador de opções", [0x114], [1, 2, 3, 4, 5],
         "05:20FC,218C,223A,22B5,2363,23D0"),
        ("Cabine, medidor em 50%, 100% ou 109%", [0x111], [2, 10, 11], "12:067D-06C1;13:10BF"),
        ("Cabine, distância em terreno de limite 5", [0x111], [15, 16, 17, 18],
         "12:0947,0957,1086-10D9,278F;15:0621-067F"),
        ("Cabine, distância em outro terreno", [0x111], list(range(19, 27)),
         "12:0965,0975,11F1-12B5,27E5;15:0682-06A7"),
        ("Cabine, colisão com seletor 0", [0x111], [27, 30], "12:0F13,0F2D,2662;14:25FE"),
        ("Cabine, abastecimento em célula de posto", [0x111], [60, 80, 90],
         "12:10F5,1111,112D,283C;14:2B53"),
        ("Cabine, entrada em célula de filme", [0x111], [5, 40, 41, 42, 43, 44],
         "12:1149-11D5,290D;14:29BC"),
        ("Cabine, método virtual +34 de retomada", [0x111], [50], "12:12D1,2DFD,30E2"),
        ("Cabine, ativação do controlador", [0x111], [1], "12:2DE6-2DF0"),
        ("CT, inicialização ou retomada do controlador", [0x1A5], [1, 3, 50],
         "11:0DFC,1751,176F-177E"),
        ("Cabine, tanque esgotado", [0x111], [400], "12:2AD8"),
        ("Memória, acerto", [0x1A4], [8, 10, 12, 14, 16], "27:1481"),
        ("Memória, erro", [0x1A4], [18, 20, 22, 24], "27:1512"),
        ("Memória, último par", [0x1A4], [26, 28], "27:11E4,16E3"),
        ("TC, seleção com cartão", [0x119], [2], "25:09D7,1586,1656"),
        ("TC, seleção sem cartão ou comando 2", [0x119], [7], "25:09D7,1586,1656,0F67"),
        ("TC, comando 3", [0x119], [9], "25:1060"),
        ("TC, comando 6", [0x119], [13], "25:1468"),
        ("TC, comando 9 sem cartão", [0x119], [17], "25:13F7"),
        ("TC, confirmação válida", [0x120], [1], "25:136A-1385"),
        ("CT, ciclo rejeitado", [0x1A5], [9, 25], "10:2997"),
        ("CT, fases do ciclo", [0x1A5], [12, 13], "10:272B;11:186E"),
    ]
    for row in rows:
        row["mapped_native_entry_paths"] = [
            {"native_event": name, "source": source, "labels": reachable_labels}
            for name, roots, labels, source in entries
            if row["root_id"] in roots
            and (reachable_labels := sorted(set(labels) & set(row["candidate_entry_labels"])))
        ]
    with_native_entry = sum(bool(row["mapped_native_entry_paths"]) for row in rows)
    (ANALYSIS / "mms-audio-event-map.json").write_text(
        json.dumps(rows, indent=2, ensure_ascii=False) + "\n", encoding="utf8")
    (ANALYSIS / "mms-native-root-sources.json").write_text(
        json.dumps({"help_selection": help_rows, "literal_constructor_calls": constructor_refs,
                    "specific_sources": specific_sources, "covered_audio_roots": sorted(audio_roots),
                    "sound_sites_with_mapped_native_entry_path": with_native_entry,
                    "sound_sites_without_mapped_native_entry_path": len(rows) - with_native_entry},
                   indent=2, ensure_ascii=False) + "\n", encoding="utf8")
    print(len(rows), sum(row["first_frame_path"] for row in rows),
          sum(bool(row["candidate_entry_labels"]) for row in rows), len(constructor_refs), with_native_entry)


if __name__ == "__main__":
    main()

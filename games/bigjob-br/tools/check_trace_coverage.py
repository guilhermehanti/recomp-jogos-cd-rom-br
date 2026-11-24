"""Compare the event inventories by identity, rather than by row count alone."""
import json
from pathlib import Path
import re

analysis = Path(__file__).resolve().parents[1] / "analysis"


def load(name):
    return json.loads((analysis / name).read_text(encoding="utf-8"))


native = load("native-audio-closure.json")["calls"]
expected_native = {(str(row["segment"]), row["offset"].upper()) for row in native}
native_report = (analysis / "34-AUDIO-EVENTOS.md").read_text(encoding="utf-8")
reported_native = set(re.findall(r"^\|\s*(\d+):([0-9A-F]{4})\s*\|", native_report, re.M))
assert len(native) == len(expected_native) == 85
assert reported_native == expected_native, (expected_native - reported_native, reported_native - expected_native)

mms = load("mms-audio-event-map.json")
identity = {(row["root_id"], row["index"], row["offset"]) for row in mms}
assert len(identity) == len(mms) == 441
expected_mms = {
    (script["root_id"], record["index"], record["offset"])
    for script in load("mms-control-flow.json")
    for record in script["records"]
    if record["opcode"] == 0x10
}
assert identity == expected_mms, (expected_mms - identity, identity - expected_mms)
assert all(row["candidate_entry_labels"] for row in mms)
print("Inventários conferidos: 85 chamadas nativas e 441 ocorrências MMS.")
print("A igualdade dos inventários verifica cobertura de endereços, sem inferir alcance pela interface.")

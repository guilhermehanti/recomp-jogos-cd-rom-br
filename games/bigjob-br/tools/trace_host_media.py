"""Capture statically used media boundaries, without loading original DLLs."""
import json
import re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]


def main():
    api = json.loads((ROOT / 'analysis/api-boundaries.json').read_text())
    listings = {}
    for p in (ROOT / 'analysis/disassembly').glob('seg*.asm'):
        listings[int(p.stem[3:])] = p.read_text().splitlines()
    def window(segment, site):
        lines = listings[segment]
        i = next(i for i, line in enumerate(lines)
                 if re.match(r'\s*' + f'{site:x}' + r':', line))
        return lines[max(0, i - 24):i + 12]
    widths = {'068A': 4, '06B2': 0, '06BC': 0, '075C': 4, '07DE': 12,
              '081A': 12, '082E': 6, '0856': 4, '086A': 4, '0874': 14,
              '087E': 0, '0888': 0, '0A7A': 10, '0AB2': 8,
              '0B06': 14, '0B76': 14, '0B84': 6}
    used = []
    for stub in api['quicktime_dispatch_stubs']:
        if not (stub['callers'] or stub['near_call_instruction_sites']):
            continue
        row = dict(stub)
        row['observed_argument_bytes'] = widths[stub['address'].split(':')[1]]
        row['evidence'] = [dict(segment=r['segment'], fixup_site=r['site'],
                                listing=window(r['segment'], r['site'] - 1))
                           for r in stub['callers']]
        row['evidence'] += [dict(segment=1, instruction_site=site,
                                 listing=window(1, int(site.split(':')[1], 16)))
                            for site in stub['near_call_instruction_sites']]
        used.append(row)
    assert len(used) == 17
    slices = {}
    for segment, start, end in [(39, 0x8e, 0x1c7), (39, 0x1de, 0x246),
                                (39, 0x26b, 0x380), (39, 0x380, 0x3e2),
                                (39, 0x43d, 0x696), (39, 0x24f2, 0x26c2),
                                (36, 0x23a7, 0x23ea)]:
        slices[f'{segment}:{start:04X}-{end:04X}'] = [
            line for line in listings[segment]
            if (m := re.match(r'\s*([0-9a-f]+):', line))
            and start <= int(m[1], 16) < end]
    result = dict(method='static relocation references, near-call sites and original assembly',
                  quicktime_total_stubs=127, referenced_stubs=used,
                  audio_source_slices=slices,
                  limitations=['110 wrappers have no direct references in this inventory; dynamic absence is not proved.',
                               'Observed argument widths are caller evidence, not SDK function identities.',
                               'These artifacts do not implement playback.'])
    (ROOT / 'analysis/host-media-contracts.json').write_text(json.dumps(result, indent=2), encoding='utf8')
    print(json.dumps(dict(qt_stubs=127, statically_referenced=len(used), audio_spans=len(slices))))


if __name__ == '__main__':
    main()

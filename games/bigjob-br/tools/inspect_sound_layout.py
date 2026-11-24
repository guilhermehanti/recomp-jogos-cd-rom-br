"""Read MMFW sound headers from the immutable ISO; emit metadata only."""
import collections
import json
import struct
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]


def main():
    inventory = json.loads((ROOT / 'analysis/inventory.json').read_text(encoding='utf-8-sig'))
    mapping = json.loads((ROOT / 'analysis/mapa-jogo-v3.json').read_text(encoding='utf-8-sig'))
    sources = {row['path']: row for row in inventory['files']}
    rows = []
    with Path(inventory['iso']).open('rb') as stream:
        for bank in ('BIGJOB.SND', 'BIGVOX.SND'):
            source = sources[bank]
            count=len(mapping['banks'][bank])
            stream.seek(source['lba']*2048+108+8*count)
            index_flags=struct.unpack('>'+str(count)+'H',stream.read(2*count))
            stream.seek(source['lba']*2048+108+4*count)
            index_timing=struct.unpack('>'+str(count)+'I',stream.read(4*count))
            for asset in mapping['banks'][bank]:
                start, size = asset['offset'], asset['size']
                assert 0 <= start < start + size <= source['bytes'] and size >= 6
                stream.seek(source['lba'] * 2048 + start)
                header = stream.read(6)
                assert len(header) == 6
                flags, timing = struct.unpack('>HI', header)
                assert flags==index_flags[asset['index']]
                assert timing==index_timing[asset['index']]
                rate = {2: 22050, 3: 44100}.get(flags & 3, 11025)
                rows.append(dict(bank=bank, index=asset['index'], name=asset['name'],
                                 offset=start, size=size, flags=flags,
                                 timing_field=timing, rate_by_native_36_23A7=rate,
                                 payload_bytes=size - 6, header=header.hex()))
    assert len(rows) == 962
    counts = dict(collections.Counter(f'{r["flags"]:04X}' for r in rows))
    result = dict(method='six-byte headers only; no conversion, playback or game execution',
                  rate_evidence='36:23A7 masks flags with3; default11025,2=22050,3=44100',
                  counts=counts, entries=rows,
                  cautions=['timing_field is not recomputed from payload length',
                            'Rate getter does not itself prove resampling behavior.',
                            'Upper flag bits need their actual consumers, not guessed codec names.'])
    (ROOT / 'analysis/sound-layout.json').write_text(json.dumps(result, indent=2), encoding='utf8')
    print(json.dumps(dict(entries=len(rows), flags=counts,
                         exceptions=[r for r in rows if r['flags'] != 0x12])))


if __name__ == '__main__':
    main()

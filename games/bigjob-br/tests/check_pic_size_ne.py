from collections import Counter
from pathlib import Path
import hashlib
import json
import struct
import subprocess

root = Path(__file__).resolve().parents[1]
db = json.loads((root / 'analysis/inventory.json').read_text(encoding='utf8'))
files = {row['path']: row for row in db['files']}
exe = files['BIGJOBNG.EXE']
origin = db['NE']['BIGJOBNG.EXE']['segments'][33]['offset']
requests, expected = [], []
with Path(db['iso']).open('rb') as image:
    image.seek(exe['lba'] * 2048)
    binary = image.read(exe['bytes'])
    assert hashlib.sha256(binary).hexdigest() == exe['sha256']
    code = binary[origin + 0x140d:origin + 0x14fd]
    assert code[:4] == bytes.fromhex('45558bec') and code[-5:] == bytes.fromhex('5f5ec94dcb')
    print('NE function SHA-256:', hashlib.sha256(code).hexdigest())
    for name in ('BIGJOBCD.PIC', 'BIGJOBHD.PIC'):
        base = files[name]['lba'] * 2048
        image.seek(base + 102)
        count, = struct.unpack('>H', image.read(2))
        offsets = struct.unpack(f'>{count + 1}I', image.read((count + 1) * 4))
        image.seek(base + 108 + 46 * count)
        flags = struct.unpack(f'>{count}H', image.read(2 * count))
        reference = db['media'][name]['offsets']
        # Modes 0 and 2 match the comparison field by construction.
        for mode in (0, 2):
            for i, (a, b) in enumerate(zip(offsets, offsets[1:])):
                requests.append(f'{mode} {flags[i]} {a} {b} 0 0 0')
                expected.append(str(reference[i + 1] - reference[i]))
        print(name, count, 'flag values:', sorted(Counter(flags).items())[:12])
# Synthetic: explicit size, aligned scanline size, signed height, 32-bit borrow.
for row, answer in [('1 8 7 9 111 5 4', 111), ('1 4 7 9 111 5 4', 32),
                    ('1 4 7 9 111 5 65535', 4294967288), ('0 0 65535 65537 0 0 0', 2)]:
    requests.append(row)
    expected.append(str(answer))
result = subprocess.run([str(root / 'build/pic_size_ne.exe')], input='\n'.join(requests),
                        text=True, capture_output=True, check=True)
assert result.stdout.splitlines() == expected
print(f'PASS: {len(expected)} checks')


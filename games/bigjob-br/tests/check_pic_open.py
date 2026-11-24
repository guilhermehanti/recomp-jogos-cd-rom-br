import json,subprocess
from pathlib import Path
w=Path(__file__).resolve().parents[1]
j=json.loads((w/'analysis/inventory.json').read_text(encoding='utf8'))
for f in j['files']:
    n=f['path']
    if n not in ('BIGJOBCD.PIC','BIGJOBHD.PIC'):continue
    args=[str(w/'build/pic_open_auto.exe'),j['iso'],str(f['lba']*2048),str(f['bytes'])]
    r=subprocess.run(args,capture_output=True,text=True)
    assert r.returncode==0,(n,r.returncode,r.stderr)
    actual=list(map(int,r.stdout.splitlines()))
    expected=j['media'][n]
    assert actual==[expected['count'],*expected['offsets']],(n,actual[:6])
    print(f'{n}: {expected["count"]} entries and EOF offset OK')

from tempfile import TemporaryDirectory
with TemporaryDirectory() as tmp:
    bad=Path(tmp)/'bad.pic'
    for name,data in [('bad magic',bytes(128)),('truncated header',b'MMFW Pictures')]:
        bad.write_bytes(data)
        r=subprocess.run([str(w/'build/pic_open_auto.exe'),str(bad),'0',str(len(data))],
                         capture_output=True,text=True)
        assert r.returncode==4 and not r.stdout,(name,r.returncode)
        print(f'{name}: rejected')

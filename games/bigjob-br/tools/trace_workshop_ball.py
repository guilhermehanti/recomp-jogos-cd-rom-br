"""Static proof bundle for the workshop basketball interaction."""
import hashlib
import json
import re
import struct
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]


def main():
    binary=(ROOT/'build/BIGJOBNG.EXE').read_bytes()
    digest=hashlib.sha256(binary).hexdigest()
    assert digest=='e483ac5986faea03f683168b6b05eae1e7ca9a5eed8449b0e880849362d65a1a'
    mapping=json.loads((ROOT/'analysis/mapa-jogo-v3.json').read_text(encoding='utf-8-sig'))
    segments=mapping['executables']['BIGJOBNG.EXE']['segments']
    code=segments[7]['offset']; data=segments[48]['offset']
    cases=list(struct.unpack_from('<4H',binary,code+0x34ac))
    assert cases==[0x31f5,0x2ed6,0x3076,0x333c]
    words=list(struct.unpack_from('<18H',binary,data+0x586))
    assert words[-1]==0x4311
    frames={str(typ):list(struct.unpack_from('<'+str(count)+'H',binary,data+off))
            for typ,off,count in [(0,0x49e,8),(1,0x4ae,30),(2,0x526,30),(3,0x4ea,30)]}
    hoop_frames=list(struct.unpack_from('<18H',binary,data+0x562))
    refs=json.loads((ROOT/'analysis/disassembly/references.json').read_text())
    assert any(r['segment']==8 and r['site']==0x47e6 and r['target']=='8:4118' for r in refs)
    assert any(r['segment']==8 and r['site']==0x4786 and r['target']=='8:45EA' for r in refs)
    listing=(ROOT/'analysis/disassembly/seg08.asm').read_text().splitlines()
    spans={}
    for start,end in [(0x1570,0x15ce),(0x25b1,0x2ab1),(0x2eaf,0x34b4),
                      (0x34b4,0x3631),(0x3631,0x4118),(0x4118,0x4782)]:
        spans[f'8:{start:04X}-{end:04X}']=[line for line in listing
            if(m:=re.match(r'\s*([0-9a-f]+):',line))and start<=int(m[1],16)<end]
    result=dict(executable_sha256=digest,scene=0,construction='8:15A5 ->8:25B1; scene.+7E',
                update='8:47D2 slot+14 ->8:4118',mouse='8:458F ->vtable4782 slot+4 ->45EA ->34B4',
                type_dispatch=cases,score_sound_choices=words,ball_frame_tables=frames,
                hoop_frame_table=hoop_frames,source_spans=spans,
                limitations=['No original code execution; event timing must be compared after implementation.'])
    (ROOT/'analysis/workshop-ball.json').write_text(json.dumps(result,indent=2),encoding='utf8')
    print(json.dumps(dict(scene=0,types=len(cases),score_choices=len(words),source_spans=len(spans))))


if __name__=='__main__':
    main()

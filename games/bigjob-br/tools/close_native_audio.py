"""Audit all 85 native audio call sites, without executing original code."""
import json,re,struct,collections
from pathlib import Path
R=Path(__file__).resolve().parents[1]
def main():
 d=json.loads((R/'analysis/media-catalogue.json').read_text(encoding='utf8')); sounds={x['packed_id']:x for x in d['sounds']}
 mapping=json.loads((R/'analysis/mapa-jogo-v3.json').read_text(encoding='utf-8-sig'))
 exe=(R/'build/BIGJOBNG.EXE').read_bytes(); ds=next(x for x in mapping['executables']['BIGJOBNG.EXE']['segments'] if x['id']==49)
 def table(off,n): return list(struct.unpack_from('<'+str(n)+'H',exe,ds['offset']+off))
 special={
 (2,0x3540):([0,0x4346],'36:1710(this.+1E); field initially0 at2:2E89; setter2:3454 copies BP+0A to+1E and BP+0C to+1C; concrete caller2:326A packs00034346 (ID4346, otherword3); player this.+20; generic setter remains parameterized'),
 (2,0x35a2):([0,0x4346],'36:1710(this.+1E); field initially0 at2:2E89; setter2:3454 copies BP+0A to+1E and BP+0C to+1C; concrete caller2:326A packs00034346 (ID4346, otherword3); player this.+20; generic setter remains parameterized'),
 (3,0x2708):([0x4156],'embedded sound this.+A0 constructed3:2554-2561 with4156; player this.+6E'),
 (4,0x2846):([0x4312,0x4352,0x4351,0x4353,0x4354,0x4355,0x4356,0x4357,0x4358,0x4359,0x435a],'ctor4:2627 copies BP+A table to+3A and BP+E count to+3E; scheduler4:2528->25E3->2806 random modulo count; callers8:1670 workshop DS048E count8 [4312,4352,4351,4352,4353,4354,4355,4356];9:04E8 CPscene4 DS05B2 count10 [4351..435A];11:0826 CSscene3 DS0636 count10 [4351..435A]'),
 (5,0xa3a):([0,0x4169,0x428e,0x4018,0x400d,0x439c,0x439d,0x428b],'forwarded packed ID BP+0A; remaining parameters BP+0C/+0E/+10; player this.+22'),
 (5,0x202b):([0x4137],'embedded sound this.+9A constructed5:1E94-1EA1 with4137; player this.+68'),
 (6,0x2c4f):(table(0x43e,4),'DS:043E[random()%4], selection 6:2C33-2C42'),
 (7,0xea0):([0x42fe,0x42ff,0x4300],'DX switch over selected object WORD0; default4300; 7:0E54-0E7F'),
 (8,0x3b55):([0x432c],'sound reference this.+CA; constructors 8:31BD-31DC/3483-34A2 use432C; player this.+74'),
 (8,0x3bcb):(table(0x586,18),'DS:0586[2:0012(00110000)] at8:3BA2-3BB4; helper2:0012 is inclusive0..17, so18 entries including4311 atDS05A8. Reached on basket score after first-score flagE8 clears and random inclusive0..100 is less than25; 8:3B8F-3BA0'),
 (8,0x437a):([0x4317,0x431a,0x4316,0x4319,0x4318],'this.+F8 chooses +BA versus +BE; BA constructors8:2F4B,30EB,324F,33B1; BE8:2F91; player this.+42'),
 (8,0x43e3):([0x431b,0x431e,0x431d],'reference this.+C2; constructor8:2FD7/3131/3295/33F7; player this.+42'),
 (8,0x4521):([0x4320,0x4322,0x431f,0x4321],'reference this.+C6; constructor8:301D/3177/32DB/343D; player this.+42'),
 (15,0xb90):([1,2,3],'WORD this.+76 initialized3 at15:0A36; overwritten15:0EA3/0F17 by15:0F20: +7A=0/1/2 maps3/2/1, otherwiseFFFF; play guarded by +78 and +5C; nonstandard+7A preservesFFFF hazard'),
 (15,0xc00):([0x4371,0x402e],'12:308E(this.+8C) nonzero chooses4371, zero402E; 15:0BA6-0BC3'),
 (15,0xc8b):([0x4372,0x402f],'12:308E(this.+8C) nonzero chooses4372, zero402F; 15:0C3B-0C58'),
 (20,0x3b4):([0x4018,0x43a0],'WORD this.+4E !=FFFF, this.+5C=0 and player.+18=0; writers20:01D1/02C1=4018,04F0=43A0; parameter500; player this.+4A'),
 (20,0x504):([0x43a0],'immediate store this.+4E=43A0 at20:04F0, then push WORD +4E at04FB'),
 (21,0x1d2f):([0x401c,0x401d],'sound object this.+10A or +114 selected at21:1D0F-1D24; initialized 21:0176-0198 by36:1710'),
 (23,0xdf3):(table(0xc92,3),'DS:0C92[DS:0C8E]; selector writes23:2741/2749/2751=0/1/2; parameter7FFF; player this.+39A'),
 (23,0x1585):(table(0xddc,6),'DS:0DDC[event-19], event19..24 checked23:1527-153D; constructed by36:1710'),
 (28,0xd71):(table(0x10d4,4),'DS:10D4/10D6/10D8/10DA selected by block.+2A; state argument2/4 only; switch28:0D07-0D49; branch default reachesD51 without assigning local (preserved original hazard)'),
 (30,0x228c):(None,'MMS sound object supplied BP+0A; player resolved by30:0D87; +51 selects parameter1/7FFF; IDs in441 MMS trigger records'),
 (36,0x291e):(None,'wrapper36:28F7 constructs36:1710 using BP+0A WORD; player BP+06; forwards parameters BP+0C/+0E/+10')}
 wrapper_callers=[]
 for path in sorted((R/'analysis/disassembly').glob('seg*.asm')):
  ls=path.read_text().splitlines()
  for i,l in enumerate(ls):
   if '=5:0A1E' in l or ('callw\t0xa1e ' in l and path.stem=='seg05'):
    wrapper_callers.append(dict(segment=int(path.stem[3:]),site=l.strip(),arguments=ls[max(0,i-18):i]))
 rows=[]
 for x in d['native_audio_calls']:
  s=x['segment']; off=int(x['site'].split(':')[0],16); lines=(R/('analysis/disassembly/seg%02d.asm'%s)).read_text().splitlines(); i=next(i for i,l in enumerate(lines) if l.strip()==x['site']); w=lines[max(0,i-45):i]
  # Reset at the final pushed DWORD zero: excludes unrelated immediates of preceding calls.
  resets=[j for j,l in enumerate(x['argument_window']) if re.search(r'pushl\s+\$0x0\b',l)]
  arg=x['argument_window'][resets[-1]:] if resets else x['argument_window']
  literals=[int(m[1],16) for l in arg if (m:=re.search(r'pushl\s+\$0x([0-9a-f]+)',l)) and int(m[1],16)>0xffff]
  ids=[literals[-1]&0xffff] if literals else None; source='packed immediate low WORD; high WORD is execution parameter, not bank'
  if (s,off) in special: ids,source=special[s,off]
  if (s,off)==(5,0xa3a): source+='; caller2:2463 uses command WORD+26 as ID and WORD+28 as parameter; app211A->+4DA[+566] is the scene receiver, NOT the sound ID. Command ctor2:23D8 copies BP+A/+C; caller6:0294 supplies4169 on first successful progress,12:1CEE supplies0000 horn.5:3254 forwards generic scene+68 unlessFFFF; its direct ctor caller2:095B suppliesFFFF, disabling that branch. Other direct callers9:25DB ID428E credits,17:1B3D ID4018 FM,25:0C9A ID400D TC,25:0CE3/0F55/10D6 IDs439C/439D from36:17DE(embedded+13A/+130), initialized25:003F-0061,26:31B3/3782 ID428B jukebox; all caller windows persisted separately'
  ids=list(dict.fromkeys(ids)) if ids else []
  refs=[dict(packed_id=n,packed_hex=f'{n:04X}',bank=n>>14,index=n&0x3fff,name=sounds.get(n,{}).get('name'),bank_file=sounds.get(n,{}).get('bank_file')) for n in ids]
  status='resolved_literal' if ids and 'immediate low' in source else 'resolved_selection' if ids else 'source_traced_remaining_producers'
  if (s,off) in [(2,0x3540),(2,0x35a2),(5,0xa3a),(30,0x228c),(36,0x291e)]: status='resolved_symbolic_interface'
  # Local function identity is safe even for shared segments; no scene inferred from segment number.
  starts=[l for l in lines[:i] if re.search(r'\bincw\s+%bp',l)]
  fun=starts[-1].strip().split(':')[0] if starts else '?'
  owners=[]
  for p in sorted((R/'analysis').glob('*.md')):
   if p.name=='26-AUDIO-NATIVO.md': continue
   for line in p.read_text(encoding='utf8').splitlines():
    if re.search(r'(?<!\d)'+str(s)+':0*'+format(off,'X')+r'\b',line,re.I): owners.append(dict(document=p.name,evidence=line[:1000]))
  row=dict(segment=s,offset=f'{off:04X}',callee=re.search(r'=36:(28F7|2937)',x['site'])[1],function_anchor=f'{s}:{int(fun,16):04X}' if fun!='?' else '?',status=status,source=source,sounds=refs,owner_status='requires caller chain; function anchor is not scene attribution',existing_activity_evidence=owners,remaining_proof=['Owning activity and event reachability require a concrete caller/virtual-table chain.']+([] if ids or status=='resolved_symbolic_interface' else ['Enumerate source object constructor/field writers and state selector to obtain concrete packed IDs.']),argument_window=x['argument_window'],extended_window=w)
  rows.append(row)
 assert len(rows)==85 and len({(r['segment'],r['offset']) for r in rows})==85
 counts=dict(collections.Counter(r['status'] for r in rows)); result=dict(shared_wrapper_5_0A1E_callers=wrapper_callers,method='static only; no original execution or compilation',abi='28F7(player farptr, packed_id WORD, parameter WORD, remaining parameters); 2937 takes a sound reference farptr instead of packed_id',counts=counts,calls=rows,limitations=['Resource resolution is separate from owner and event reachability.','No shared segment is assigned wholesale to a scene.','Resolved selections preserve candidate sets; selection conditions are in source.'])
 (R/'analysis/native-audio-closure.json').write_text(json.dumps(result,ensure_ascii=False,indent=2),encoding='utf8')
 md=['# Audio nativo: auditoria das 85 chamadas','', 'Analise estatica exclusivamente; nenhum original executado e nenhuma compilacao. `native-audio-closure.json` preserva todas as 85 chamadas, janela original e janela ampliada, ancora de funcao, fonte dos argumentos, IDs concretos e nomes do banco.','', 'Contagens: '+json.dumps(counts)+'. Resolucao de recurso nao e prova de alcance UI. A coluna owner mantem a funcao concreta: nao atribui segmentos compartilhados inteiros a cenas. Os registros individuais preservam mencoes ja existentes nos relatorios e as provas ainda necessarias.','', '36:28F7 recebe player farptr em BP+6 e ID WORD em BP+A; a palavra seguinte e parametro. 36:2937 recebe referencia de som farptr em BP+A. Nao interpretar ponteiros como IDs. O banco vem de `id>>14`; indice `id&3FFF`.','', '## Todas as chamadas','', '| Site | Funcao local | Status | Recursos ou fonte |','|---|---|---|---|']
 for r in rows:
  resources='; '.join(f"{a['packed_hex']} = {a['bank_file']}[{a['index']}] {a['name']}" for a in r['sounds'])
  md.append(f"|{r['segment']}:{r['offset']}|{r['function_anchor']}|{r['status']}|{resources or r['source']}|")
 md+=['','## Provas de selecao adicionais','']
 for r in rows:
  if r['status'] in ('resolved_selection','resolved_symbolic_interface'):md.append(f"- {r['segment']}:{r['offset']}: {r['source']}.")
 md+=['','## Limites que impedem declarar fechamento total','','Os registros `source_traced_remaining_producers` identificam campos, tabelas ou wrappers, mas ainda requerem enumeracao dos produtores e associacao ao estado da atividade. Mesmo os literais fecham apenas recurso/argumento: o caminho de owner/evento precisa ser demonstrado antes de implementar disparos por atividade. O JSON conserva essa lacuna em cada chamada. `resolved_symbolic_interface` fecha a origem de parametro/objeto e conserva o contrato reutilizavel; IDs listados nesses registros sao concretos conhecidos, sem afirmar que um contrato generico limita seus futuros argumentos a esse conjunto. A ancora de funcao e obtida do prologo local e nao equivale a um nome de classe ou cena.','', 'Reproducao: `python tools/close_native_audio.py`. A rotina apenas le arquivos existentes e escreve estes dois artefatos. Nao altera `catalogue_media.py` nem `media-catalogue.json`.']
 (R/'analysis/26-AUDIO-NATIVO.md').write_text('\n'.join(md)+'\n',encoding='utf8');print(counts)
if __name__=='__main__':main()

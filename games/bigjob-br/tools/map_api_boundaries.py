"""Static external-boundary inventory. Never executes the original binary."""
import ast, json, re, struct, collections
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
refs=json.loads((ROOT/'analysis/disassembly/references.json').read_text())
mp=json.loads((ROOT/'analysis/mapa-jogo-v3.json').read_text(encoding='utf-8-sig'))
binary=(ROOT/'build/BIGJOBNG.EXE').read_bytes()
tree=ast.parse((ROOT/'build/pcrecomp-official/tools/ne/win16.py').read_text())
seed={};purge={}
proto={}
for node in tree.body:
    if isinstance(node,ast.Assign):
        for target in node.targets:
            if isinstance(target,ast.Name) and target.id in ('_SEED','PURGE'):
                value=ast.literal_eval(node.value)
                if target.id=='_SEED': seed=value
                else: purge=value
    if isinstance(node,ast.Expr) and isinstance(node.value,ast.Call):
        f=node.value.func
        if isinstance(f,ast.Attribute) and isinstance(f.value,ast.Name) and f.value.id=='PURGE' and f.attr=='update':
            if isinstance(node.value.args[0],ast.Dict): purge.update(ast.literal_eval(node.value.args[0]))
for node in tree.body:
    if isinstance(node,ast.Assign) and any(isinstance(x,ast.Name) and x.id=='_PROTO' for x in node.targets):
        expr=ast.fix_missing_locations(ast.Expression(node.value))
        proto=eval(compile(expr,'<local-prototype-table>','eval'),{'__builtins__':{},'_W':2,'_D':4})
        purge.update({tuple(k.split('.',1)):sum(v) for k,v in proto.items()})
purge[('USER','WSPRINTF')]=0  # cdecl variadic: caller cleanup, documented in win16.py
# Names confirmed by earlier static project reports, kept distinct from toolbox seed.
extra={'USER!176':'LoadString','MMSYSTEM!404':'waveOutOpen','MMSYSTEM!405':'waveOutClose','MMSYSTEM!406':'waveOutPrepareHeader','MMSYSTEM!407':'waveOutUnprepareHeader','MMSYSTEM!408':'waveOutWrite','MMSYSTEM!411':'waveOutReset','MMSYSTEM!607':'timeGetTime'}
groups=collections.defaultdict(list);osfix=[]
for r in refs:
    if r['flags']&3 in (1,2) and '!' in r['target'] and not r['target'].startswith('!'):
        groups[r['target']].append(r)
    elif r['flags']&3==3: osfix.append(r)
rows=[]
official=json.loads((ROOT/'analysis/win16-api-evidence.json').read_text(encoding='utf8'))
for target,sites in sorted(groups.items(),key=lambda x:(x[0].split('!')[0],int(x[0].split('!')[1]))):
    module,ordinal=target.split('!');name=seed.get(module,{}).get(int(ordinal)) or extra.get(target)
    ev=official[target]; definition=ev['definition']; declaration=definition.split('#',1)[0].strip()
    equate=re.match(r'\d+\s+equate\s+(\S+)\s+(\S+)',declaration)
    fn=re.match(r'\d+\s+(pascal|cdecl|varargs)\s+((?:-\S+\s+)*)?(\w+)\s*\(([^)]*)\)',declaration)
    old_name=name
    if equate:
        name=equate[1]; args=[]; widths=[]; convention='constant'; cleanup=None; value=int(equate[2],0)
    elif fn:
        convention=fn[1]; name=fn[3]; args=fn[4].split(); widths=[2 if a in ('word','s_word') else 4 for a in args]
        cleanup=sum(widths) if convention=='pascal' else 0; value=None
    else:
        raise ValueError((target,declaration))
    rows.append(dict(target=target,name=name,name_evidence=ev['url'],reference_declaration=definition,previous_local_name=old_name,name_corrected=bool(old_name and old_name.lower()!=name.lower()),kind='constant' if equate else 'function',constant_value=value,calling_convention=convention,purge_bytes=cleanup,argument_types=args,argument_widths=widths,return16=bool(fn and '-ret16' in (fn[2] or '')),register_context=bool(fn and '-register' in (fn[2] or '')),stack_signature_complete=bool(fn and convention=='pascal' and '-register' not in (fn[2] or '')),sites=sites))
seg=mp['executables']['BIGJOBNG.EXE']['segments'];ds=seg[48];data=binary[ds['offset']:ds['offset']+ds['size']]
strings={f'{off:04X}':data[off:data.index(0,off)].decode('ascii') for off in (0x1556,0x155f,0x156b,0x1578,0x1584,0x1594,0x159c,0x159f,0x15a2,0x15a8,0x15b8,0x15be,0x15cb,0x15d4,0x15e1,0x15e5)}
s1=binary[seg[0]['offset']:seg[0]['offset']+seg[0]['size']]
listing=(ROOT/'analysis/disassembly/seg01.asm').read_text()
near_calls=collections.defaultdict(list)
for line in listing.splitlines():
    mm=re.match(r'\s*([0-9a-f]+):.*\bcallw\s+0x([0-9a-f]+)',line)
    if mm: near_calls[int(mm[2],16)].append(f'1:{int(mm[1],16):04X}')
stubs=[]
for match in re.finditer(b'\x1e\xbb(..)(?:\xb8(..))?\xff\x1e([\x46\x4e])\x15\x1f\xcb',s1,re.S):
    off=match.start();selector=struct.unpack('<H',match[1])[0];slot=0x1500+match[3][0]
    callers=[r for r in refs if r['target']==f'1:{off:04X}']
    stubs.append(dict(address=f'1:{off:04X}',selector_bx=selector,selector_ax=struct.unpack('<H',match[2])[0] if match[2] else None,dispatcher_slot=f'DS:{slot:04X}',external_name=None,argument_bytes=None,near_call_instruction_sites=near_calls[off],callers=callers))
assert len(rows)==110 and sum(len(x['sites']) for x in rows)==402
assert len(osfix)==98 and len(stubs)==127
for fixup in osfix:
    raw_offset=seg[fixup['segment']-1]['offset']+fixup['site']
    fixup['original_bytes']=binary[raw_offset:raw_offset+8].hex()
    fixup['classification']='x87 WAIT compatibility fixup' if binary[raw_offset]==0x9b else 'x87 NOP compatibility fixup' if binary[raw_offset]==0x90 else 'requires individual opcode check'
    fixup['loader_reference']='https://github.com/wine-mirror/wine/blob/master/dlls/krnl386.exe16/ne_segment.c#L234'
assert all(x['classification'].startswith('x87 ') for x in osfix)
assert strings['1556']=='QTIM.DLL' and strings['156B']=='QTIMCMGR.DLL'
result=dict(scope='BIGJOBNG.EXE expanded NE relocation sites; not dynamic reachability',imports=rows,os_fixup_sites=osfix,dgroup_dynamic_strings=strings,quicktime_dispatch_stubs=stubs,limitations=['Name/purge from local seed is supporting evidence, not verified full prototype.','Null purge means unknown, never zero.','QuickTime BX selectors are dispatcher indices, not DLL export ordinals.','Far pointers, DS and callback lifetimes require host adapter; no 32-bit ABI substitution.'])
result['limitations']=['All110 ordinal identities compared to Wine primary specification; aggregate signatures do not specify pointed structure layouts.','KERNEL!113 is a loader constant __AHSHIFT=3, not a function.','InitTask and WIN87EM register ABI require CPU context.','QuickTime BX selectors are dispatcher indices, not DLL export ordinals.','Far pointers, DS and callback lifetimes require a host adapter.']
result['dynamic_ordinal_evidence']=json.loads((ROOT/'analysis/win16-dynamic-api-evidence.json').read_text(encoding='utf8'))
(ROOT/'analysis/api-boundaries.json').write_text(json.dumps(result,indent=2),encoding='utf-8')
lines=['# Fronteiras externas e ABI Win16','','Analise estatica de BIGJOBNG.EXE. Nenhum jogo, compilacao ou DLL foi executado. Segmentos decimais, offsets hexadecimais. `api-boundaries.json` registra cada site de relocacao expandida, incluindo tipo, flags e local em dados; um site nao e necessariamente uma chamada alcancavel.','','## Inventario completo dos imports estaticos','',f'{len(rows)} pares modulo/ordinal; {sum(len(x["sites"]) for x in rows)} sites expandidos; {len(osfix)} sites de fixup OS separados. Neste inventario NG, os sites de import somam 402; as cadeias internas nao devem ser confundidas com imports adicionais. A tabela abaixo e do NG. WinG na variante regular exige inventario separado, ja registrado em 02.','','| Import | Nome sustentado localmente | Purge conhecido (bytes) | Sites |','|---|---|---:|---:|']
for row in rows: lines.append(f'| {row["target"]} | {row["name"] or "nao resolvido"} | {row["purge_bytes"] if row["purge_bytes"] is not None else "?"} | {len(row["sites"])} |')
lines += ['','Nome vem da tabela seed de `build/pcrecomp-official/tools/ne/win16.py`, salvo LoadString/waveOut/timeGetTime ja demonstrados em 02/14. A seed nao cobre todos os ordinais. Purge e tamanho agregado da pilha, nao assinatura completa nem validacao deste binario. Nao substituir `?` por zero. Todos os sites individuais ficam no JSON.','','## Contrato de adaptacao','','O modulo local descreve Win16 PASCAL: WORD/int/handle de 2 bytes; LONG/DWORD/far pointer de 4; retorno far de 4 bytes separado do purge. O adapter deve traduzir seletor:offset e copiar estruturas no layout Win16, sem passar ponteiros emulados diretamente a APIs nativas. Resultados DWORD/far podem usar DX:AX; a carga de GetProcAddress em 1:024F-0252 comprova isso. DS deve ser preservado conforme wrapper. Nao generalizar PASCAL ao wsprintf variadico nem ao WIN87EM ou despacho QuickTime. `stack_signature_complete` trata apenas convencao e larguras; nao certifica layouts apontados, callbacks ou implementacao do host.','','A fronteira de arquivos inclui KERNEL!74,81,83,84,85 e ordinais de perfil 128/129, alem de recursos e lookup de modulos. As rotinas Borland internas podem usar DOS diretamente: este inventario cobre imports NE, nao afirma ausencia de outras operacoes de arquivo. USER cobre janela, eventos, timers, cursores, recursos e callbacks; GDI cobre objetos, DIB e documento. Para impressao, 34:2140 chama GDI!377 com estrutura local de 10 bytes conforme 23: o nome/prototipo continua pendente nesta evidencia local. A submissao nao comprova spool concluido. MMSYSTEM!404..411 e 607 sao a fronteira waveOut/tempo, nao MCI. Ver 26 para fechamento de buffers/callbacks.','','## DLLs dinamicas e QuickTime','','Os bytes de DGROUP49 fecham os nomes reais; deixam de ser dependencia apenas presumida.','','| Operacao / chamada | String exata e armazenamento | Resultado |','|---|---|---|','| 1:0237 LoadLibrary; 024A GetProcAddress | 49:1556 QTIM.DLL; 155F _EntryPoint | modulo DS:154A; far dispatcher DS:1546 |','| 1:02B9 LoadLibrary; 02CC GetProcAddress | 49:156B QTIMCMGR.DLL; 1578 _EntryPoint | modulo DS:1552; dispatcher DS:154E |','| 1:02E1 GetProcAddress; 02FC chamada indireta | 49:1584 _CMgrInitialize | inicializacao sem pushes de argumento entre resolucao e chamada |','| 1:04CE GetProcAddress | 49:15A8 _CMgrTerminate | terminacao dinamica |','| 1:035F LoadLibrary; 0375/0388/039B GetProcAddress | ver.dll; #6, #7, #11 | tres far pointers locais -0E/-0A/-06 |','| 1:053E GetModuleHandle; 0550 GetProcAddress | QTIM; _CLEANUPTASK | rotina opcional de limpeza |','| 1:0570 GetModuleHandle; 0589 LoadLibrary; 059A/05AD GetProcAddress | toolhelp.dll; #63, #64 | notificacoes; ver janela em seg01 |','','`#6` etc. sao strings ASCII realmente passadas como ponteiros far; nao afirmar que o chamador passa ordinal numerico de segmento zero. A interpretacao pelo loader faz parte do contrato a resolver. A checagem LoadLibrary compara AX com 0x20; falha instala fallback 1:0624/1:0A4A e zera modulo, em vez de continuar com ponteiro nulo. 1:029A/0340 incrementam contadores; 1:047E/04B2 fazem liberacao ao chegar a zero.','','Os wrappers QuickTime salvam DS, carregam BX com seletor (CMgr tambem carrega AX=1 nos wrappers encontrados) e fazem `lcall *DS:1546` ou `*DS:154E`, restaurando DS antes de `lret`. O JSON extrai 103 wrappers QTIM e 24 CMgr pelo padrao de bytes e associa referencias internas relocadas e calls near de seg01 a cada um. Esses seletores NAO sao os ordinais exportados pela DLL. Exemplos: 1:07DE usa BX=29h (abertura do filme por offset demonstrada em14); o par modulo/nome da rotina externa continua `_EntryPoint`, com protocolo de despacho. Nao atribuir nomes SDK aos demais seletores sem mapa compativel desta versao.','','A stack de cada seletor, seus retornos e estruturas permanecem explicitamente desconhecidos. 1:0624 possui inicializacao tardia e salto ao dispatcher; no erro retorna DX=8000h/AX=8001h. A substituicao nativa precisa reproduzir erro, refcount e ciclo do controller do segmento33, mesmo se o host dispensar QuickTime antigo.','','## Reproducao e limites','','`python tools/map_api_boundaries.py` usa apenas JSON, ASM/bytes e AST da tabela local; nao importa nem executa o original. A contagem completa verifica a superficie estatica, nao certifica todas as assinaturas. Pendencias concretas: nomes dos ordinais fora da seed; prototipos/estruturas e callback ABI por API; protocolo dos seletores QuickTime; semantica dos ordinais ver/toolhelp; classificacao dos fixups OS; equivalencia da variante regular. Estes itens impedem declarar a fronteira host pronta para execucao integral.']
lines=[line for line in lines if not any(s in line for s in ('Nome vem da tabela seed','Para impressao, 34:2140','Pendencias concretas: nomes dos ordinais'))]
lines=[line.replace('toolhelp.dll; #63, #64 | notificacoes; ver janela em seg01','QTNOTIFY; depois toolhelp.dll; #63, #64 | enumeracao de tarefas, TaskFirst/TaskNext') for line in lines]
lines += ['','## Identidades e assinaturas verificadas com fonte primaria','',
'Os110 pares modulo/ordinal foram confrontados com as especificacoes publicas do Wine, consultadas via GitHub somente para leitura. `win16-api-evidence.json` guarda URL e declaracao de cada entrada efetivamente importada. Nenhum codigo externo foi instalado ou executado. O gerador preserva o nome local anterior para tornar as divergencias auditaveis.', '',
'Tres erros materiais da seed local foram corrigidos: GDI!53 e CreateDC (quatro ponteiros far,16bytes), nao CreateBitmap; GDI!439 e StretchDIBits (32bytes), nao SetDIBitsToDevice; KERNEL!113 e a constante de loader __AHSHIFT=3, nao uma funcao. Os128 sites dessa constante nao sao128 chamadas de sistema. A fronteira de impressao confirma GDI!377 StartDoc e !378 EndDoc. A estrutura DOCINFO Win16 e os erros precisam acompanhar o pipeline descrito em23.', '',
'A assinatura registrada separa word/s_word (2bytes), long e ponteiros far (4bytes), convencao, retorno16 e contexto de registradores. Declaracao pascal comum fecha largura e limpeza da pilha; nao autoriza converter estruturas Win16 em estruturas nativas por cast. Variadicos e entrypoints register conservam tratamento proprio. GDI!439 precisa de adaptacao de BITMAPINFO, pixels indexados e ROP; GDI!53 de strings e DEVMODE; funcoes USER com callbacks exigem thunks e vida util.', '',
'Fontes: [GDI](https://github.com/wine-mirror/wine/blob/master/dlls/gdi.exe16/gdi.exe16.spec), [KERNEL](https://github.com/wine-mirror/wine/blob/master/dlls/krnl386.exe16/krnl386.exe16.spec), [USER](https://github.com/wine-mirror/wine/blob/master/dlls/user.exe16/user.exe16.spec), [MMSYSTEM](https://github.com/wine-mirror/wine/blob/master/dlls/mmsystem.dll16/mmsystem.dll16.spec), [WIN87EM](https://github.com/wine-mirror/wine/blob/master/dlls/win87em.dll16/win87em.dll16.spec).', '',
'O fechamento de identidade dos imports estaticos esta concluido. A traducao integral das estruturas e dos127 seletores QuickTime continua separada: a contagem nao prova essa implementacao nem execucao do jogo. `python tools/map_api_boundaries.py` reproduz a tabela a partir das evidencias locais.', '',
'Os98 fixups OS sao todos tipo5/flags7 e apontam a87 bytes9B (WAIT x87) e11 bytes90 (NOP), conferidos individualmente nos bytes originais. O [loader NE do Wine](https://github.com/wine-mirror/wine/blob/master/dlls/krnl386.exe16/ne_segment.c#L234) classifica tipo7 como compatibilidade do emulador de ponto flutuante e preserva as instrucoes para usar FPU. Portanto nao existem98 APIs desconhecidas nesse conjunto. Uma traducao estatica deve manter a semantica x87 e o arredondamento; nao aplicar adicao numerica a essas instrucoes como se fossem relocacoes comuns. Os bytes de cada site foram acrescentados ao JSON.', '',
'## Ordinais dinamicos identificados', '',
'VER!6=GetFileVersionInfoSize, !7=GetFileVersionInfo e !11=VerQueryValue. TOOLHELP!63=TaskFirst e !64=TaskNext. As cinco declaracoes e URLs primarias estao em win16-dynamic-api-evidence.json. O fluxo1:0570 procura QTNOTIFY; somente quando esse modulo existe carrega toolhelp.dll por1:0584. Inicializa estrutura de40bytes em1:05C8, percorre TaskFirst/TaskNext em1:05D7/05EB e compara o modulo+0A ao handle QTNOTIFY. Ao encontrar, pode postar mensagem0400 para a tarefa+4, dependendo do retorno de _CLEANUPTASK, e usa DirectedYield. Portanto esta rotina enumera a tarefa auxiliar de QuickTime; nao instala um callback de notificacao ToolHelp.', '',
'As fontes sao [VER](https://github.com/wine-mirror/wine/blob/master/dlls/ver.dll16/ver.dll16.spec) e [TOOLHELP](https://github.com/wine-mirror/wine/blob/master/dlls/toolhelp.dll16/toolhelp.dll16.spec). A existencia das DLLs externas nao foi pressuposta nem testada por carregamento.']
(ROOT/'analysis/28-FRONTEIRAS-APIS.md').write_text('\n'.join(lines)+'\n',encoding='utf-8')
print(json.dumps({'imports':len(rows),'sites':sum(len(r['sites']) for r in rows),'os_sites':len(osfix),'qt_wrappers':len(stubs),'known_names':sum(r['name'] is not None for r in rows),'function_sites':sum(len(r['sites']) for r in rows if r['kind']=='function'),'constant_sites':sum(len(r['sites']) for r in rows if r['kind']=='constant')}))


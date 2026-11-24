# Fronteiras externas e ABI Win16

Analise estatica de BIGJOBNG.EXE. Nenhum jogo, compilacao ou DLL foi executado. Segmentos decimais, offsets hexadecimais. `api-boundaries.json` registra cada site de relocacao expandida, incluindo tipo, flags e local em dados; um site nao e necessariamente uma chamada alcancavel.

## Inventario completo dos imports estaticos

110 pares modulo/ordinal; 402 sites expandidos; 98 sites de fixup OS separados. Neste inventario NG, os sites de import somam 402; as cadeias internas nao devem ser confundidas com imports adicionais. A tabela abaixo e do NG. WinG na variante regular exige inventario separado, ja registrado em 02.

| Import | Nome sustentado localmente | Purge conhecido (bytes) | Sites |
|---|---|---:|---:|
| GDI!2 | SetBkMode | 4 | 1 |
| GDI!3 | SetMapMode | 4 | 1 |
| GDI!9 | SetTextColor | 6 | 1 |
| GDI!11 | SetWindowOrg | 6 | 1 |
| GDI!12 | SetWindowExt | 6 | 1 |
| GDI!14 | SetViewportExt | 6 | 1 |
| GDI!38 | Escape | 14 | 3 |
| GDI!44 | SelectClipRgn | 4 | 1 |
| GDI!45 | SelectObject | 4 | 6 |
| GDI!53 | CreateDC | 16 | 2 |
| GDI!57 | CreateFontIndirect | 4 | 2 |
| GDI!64 | CreateRectRgn | 8 | 1 |
| GDI!67 | DPtoLP | 8 | 2 |
| GDI!68 | DeleteDC | 2 | 3 |
| GDI!69 | DeleteObject | 2 | 6 |
| GDI!77 | GetClipBox | 6 | 1 |
| GDI!80 | GetDeviceCaps | 4 | 3 |
| GDI!87 | GetStockObject | 2 | 7 |
| GDI!91 | GetTextExtent | 8 | 1 |
| GDI!97 | GetWindowOrg | 2 | 1 |
| GDI!128 | MulDiv | 6 | 11 |
| GDI!360 | CreatePalette | 4 | 2 |
| GDI!377 | StartDoc | 6 | 1 |
| GDI!378 | EndDoc | 2 | 1 |
| GDI!439 | StretchDIBits | 32 | 3 |
| KERNEL!3 | GetVersion | 0 | 1 |
| KERNEL!15 | GlobalAlloc | 6 | 5 |
| KERNEL!16 | GlobalReAlloc | 8 | 1 |
| KERNEL!17 | GlobalFree | 2 | 5 |
| KERNEL!18 | GlobalLock | 2 | 7 |
| KERNEL!19 | GlobalUnlock | 2 | 7 |
| KERNEL!20 | GlobalSize | 2 | 1 |
| KERNEL!21 | GlobalHandle | 2 | 4 |
| KERNEL!25 | GlobalCompact | 4 | 1 |
| KERNEL!30 | WaitEvent | 2 | 1 |
| KERNEL!36 | GetCurrentTask | 0 | 1 |
| KERNEL!47 | GetModuleHandle | 4 | 2 |
| KERNEL!48 | GetModuleUsage | 2 | 1 |
| KERNEL!49 | GetModuleFileName | 8 | 3 |
| KERNEL!50 | GetProcAddress | 6 | 10 |
| KERNEL!51 | MakeProcInstance | 6 | 3 |
| KERNEL!52 | FreeProcInstance | 4 | 5 |
| KERNEL!58 | GetProfileString | 18 | 1 |
| KERNEL!60 | FindResource | 10 | 1 |
| KERNEL!74 | OpenFile | 10 | 2 |
| KERNEL!81 | _lclose | 2 | 1 |
| KERNEL!83 | _lcreat | 6 | 1 |
| KERNEL!84 | _llseek | 8 | 2 |
| KERNEL!85 | _lopen | 6 | 2 |
| KERNEL!91 | InitTask | 0 | 1 |
| KERNEL!95 | LoadLibrary | 4 | 5 |
| KERNEL!96 | FreeLibrary | 2 | 6 |
| KERNEL!107 | SetErrorMode | 2 | 3 |
| KERNEL!113 | __AHSHIFT | ? | 128 |
| KERNEL!128 | GetPrivateProfileString | 22 | 1 |
| KERNEL!129 | WritePrivateProfileString | 16 | 2 |
| KERNEL!132 | GetWinFlags | 0 | 2 |
| KERNEL!134 | GetWindowsDirectory | 6 | 3 |
| KERNEL!150 | DirectedYield | 2 | 1 |
| KERNEL!166 | WinExec | 6 | 1 |
| KERNEL!348 | hmemcpy | 12 | 2 |
| KERNEL!349 | _hread | 10 | 1 |
| KERNEL!350 | _hwrite | 10 | 2 |
| MMSYSTEM!404 | waveOutOpen | 22 | 1 |
| MMSYSTEM!405 | waveOutClose | 2 | 2 |
| MMSYSTEM!406 | waveOutPrepareHeader | 8 | 1 |
| MMSYSTEM!407 | waveOutUnprepareHeader | 8 | 1 |
| MMSYSTEM!408 | waveOutWrite | 8 | 1 |
| MMSYSTEM!411 | waveOutReset | 2 | 1 |
| MMSYSTEM!607 | timeGetTime | 0 | 1 |
| USER!1 | MessageBox | 12 | 6 |
| USER!5 | InitApp | 2 | 1 |
| USER!6 | PostQuitMessage | 2 | 2 |
| USER!18 | SetCapture | 2 | 1 |
| USER!19 | ReleaseCapture | 0 | 1 |
| USER!39 | BeginPaint | 6 | 2 |
| USER!40 | EndPaint | 6 | 2 |
| USER!41 | CreateWindow | 30 | 5 |
| USER!42 | ShowWindow | 4 | 3 |
| USER!53 | DestroyWindow | 2 | 1 |
| USER!57 | RegisterClass | 4 | 5 |
| USER!66 | GetDC | 2 | 4 |
| USER!68 | ReleaseDC | 4 | 2 |
| USER!69 | SetCursor | 2 | 1 |
| USER!71 | ShowCursor | 2 | 2 |
| USER!81 | FillRect | 8 | 1 |
| USER!85 | DrawText | 14 | 2 |
| USER!102 | AdjustWindowRect | 10 | 1 |
| USER!107 | DefWindowProc | 10 | 3 |
| USER!109 | PeekMessage | 12 | 3 |
| USER!113 | TranslateMessage | 4 | 1 |
| USER!114 | DispatchMessage | 4 | 1 |
| USER!116 | PostAppMessage | 10 | 1 |
| USER!125 | InvalidateRect | 8 | 1 |
| USER!135 | GetWindowLong | 4 | 2 |
| USER!136 | SetWindowLong | 8 | 5 |
| USER!173 | LoadCursor | 6 | 7 |
| USER!174 | LoadIcon | 6 | 2 |
| USER!176 | LoadString | 10 | 4 |
| USER!177 | LoadAccelerators | 6 | 2 |
| USER!178 | TranslateAccelerator | 8 | 1 |
| USER!179 | GetSystemMetrics | 2 | 5 |
| USER!221 | ScrollDC | 20 | 4 |
| USER!225 | EnumTaskWindows | 10 | 1 |
| USER!229 | GetTopWindow | 2 | 1 |
| USER!282 | SelectPalette | 6 | 6 |
| USER!283 | RealizePalette | 2 | 3 |
| USER!420 | _wsprintf | 0 | 3 |
| USER!483 | SystemParametersInfo | 10 | 1 |
| WIN87EM!1 | __fpMath | 0 | 6 |


## Contrato de adaptacao

O modulo local descreve Win16 PASCAL: WORD/int/handle de 2 bytes; LONG/DWORD/far pointer de 4; retorno far de 4 bytes separado do purge. O adapter deve traduzir seletor:offset e copiar estruturas no layout Win16, sem passar ponteiros emulados diretamente a APIs nativas. Resultados DWORD/far podem usar DX:AX; a carga de GetProcAddress em 1:024F-0252 comprova isso. DS deve ser preservado conforme wrapper. Nao generalizar PASCAL ao wsprintf variadico nem ao WIN87EM ou despacho QuickTime. `stack_signature_complete` trata apenas convencao e larguras; nao certifica layouts apontados, callbacks ou implementacao do host.


## DLLs dinamicas e QuickTime

Os bytes de DGROUP49 fecham os nomes reais; deixam de ser dependencia apenas presumida.

| Operacao / chamada | String exata e armazenamento | Resultado |
|---|---|---|
| 1:0237 LoadLibrary; 024A GetProcAddress | 49:1556 QTIM.DLL; 155F _EntryPoint | modulo DS:154A; far dispatcher DS:1546 |
| 1:02B9 LoadLibrary; 02CC GetProcAddress | 49:156B QTIMCMGR.DLL; 1578 _EntryPoint | modulo DS:1552; dispatcher DS:154E |
| 1:02E1 GetProcAddress; 02FC chamada indireta | 49:1584 _CMgrInitialize | inicializacao sem pushes de argumento entre resolucao e chamada |
| 1:04CE GetProcAddress | 49:15A8 _CMgrTerminate | terminacao dinamica |
| 1:035F LoadLibrary; 0375/0388/039B GetProcAddress | ver.dll; #6, #7, #11 | tres far pointers locais -0E/-0A/-06 |
| 1:053E GetModuleHandle; 0550 GetProcAddress | QTIM; _CLEANUPTASK | rotina opcional de limpeza |
| 1:0570 GetModuleHandle; 0589 LoadLibrary; 059A/05AD GetProcAddress | QTNOTIFY; depois toolhelp.dll; #63, #64 | enumeracao de tarefas, TaskFirst/TaskNext |

`#6` etc. sao strings ASCII realmente passadas como ponteiros far; nao afirmar que o chamador passa ordinal numerico de segmento zero. A interpretacao pelo loader faz parte do contrato a resolver. A checagem LoadLibrary compara AX com 0x20; falha instala fallback 1:0624/1:0A4A e zera modulo, em vez de continuar com ponteiro nulo. 1:029A/0340 incrementam contadores; 1:047E/04B2 fazem liberacao ao chegar a zero.

Os wrappers QuickTime salvam DS, carregam BX com seletor (CMgr tambem carrega AX=1 nos wrappers encontrados) e fazem `lcall *DS:1546` ou `*DS:154E`, restaurando DS antes de `lret`. O JSON extrai 103 wrappers QTIM e 24 CMgr pelo padrao de bytes e associa referencias internas relocadas e calls near de seg01 a cada um. Esses seletores NAO sao os ordinais exportados pela DLL. Exemplos: 1:07DE usa BX=29h (abertura do filme por offset demonstrada em14); o par modulo/nome da rotina externa continua `_EntryPoint`, com protocolo de despacho. Nao atribuir nomes SDK aos demais seletores sem mapa compativel desta versao.

A stack de cada seletor, seus retornos e estruturas permanecem explicitamente desconhecidos. 1:0624 possui inicializacao tardia e salto ao dispatcher; no erro retorna DX=8000h/AX=8001h. A substituicao nativa precisa reproduzir erro, refcount e ciclo do controller do segmento33, mesmo se o host dispensar QuickTime antigo.

## Reproducao e limites


## Identidades e assinaturas verificadas com fonte primaria

Os110 pares modulo/ordinal foram confrontados com as especificacoes publicas do Wine, consultadas via GitHub somente para leitura. `win16-api-evidence.json` guarda URL e declaracao de cada entrada efetivamente importada. Nenhum codigo externo foi instalado ou executado. O gerador preserva o nome local anterior para tornar as divergencias auditaveis.

Tres erros materiais da seed local foram corrigidos: GDI!53 e CreateDC (quatro ponteiros far,16bytes), nao CreateBitmap; GDI!439 e StretchDIBits (32bytes), nao SetDIBitsToDevice; KERNEL!113 e a constante de loader __AHSHIFT=3, nao uma funcao. Os128 sites dessa constante nao sao128 chamadas de sistema. A fronteira de impressao confirma GDI!377 StartDoc e !378 EndDoc. A estrutura DOCINFO Win16 e os erros precisam acompanhar o pipeline descrito em23.

A assinatura registrada separa word/s_word (2bytes), long e ponteiros far (4bytes), convencao, retorno16 e contexto de registradores. Declaracao pascal comum fecha largura e limpeza da pilha; nao autoriza converter estruturas Win16 em estruturas nativas por cast. Variadicos e entrypoints register conservam tratamento proprio. GDI!439 precisa de adaptacao de BITMAPINFO, pixels indexados e ROP; GDI!53 de strings e DEVMODE; funcoes USER com callbacks exigem thunks e vida util.

Fontes: [GDI](https://github.com/wine-mirror/wine/blob/master/dlls/gdi.exe16/gdi.exe16.spec), [KERNEL](https://github.com/wine-mirror/wine/blob/master/dlls/krnl386.exe16/krnl386.exe16.spec), [USER](https://github.com/wine-mirror/wine/blob/master/dlls/user.exe16/user.exe16.spec), [MMSYSTEM](https://github.com/wine-mirror/wine/blob/master/dlls/mmsystem.dll16/mmsystem.dll16.spec), [WIN87EM](https://github.com/wine-mirror/wine/blob/master/dlls/win87em.dll16/win87em.dll16.spec).

O fechamento de identidade dos imports estaticos esta concluido. A traducao integral das estruturas e dos127 seletores QuickTime continua separada: a contagem nao prova essa implementacao nem execucao do jogo. `python tools/map_api_boundaries.py` reproduz a tabela a partir das evidencias locais.

Os98 fixups OS sao todos tipo5/flags7 e apontam a87 bytes9B (WAIT x87) e11 bytes90 (NOP), conferidos individualmente nos bytes originais. O [loader NE do Wine](https://github.com/wine-mirror/wine/blob/master/dlls/krnl386.exe16/ne_segment.c#L234) classifica tipo7 como compatibilidade do emulador de ponto flutuante e preserva as instrucoes para usar FPU. Portanto nao existem98 APIs desconhecidas nesse conjunto. Uma traducao estatica deve manter a semantica x87 e o arredondamento; nao aplicar adicao numerica a essas instrucoes como se fossem relocacoes comuns. Os bytes de cada site foram acrescentados ao JSON.

## Ordinais dinamicos identificados

VER!6=GetFileVersionInfoSize, !7=GetFileVersionInfo e !11=VerQueryValue. TOOLHELP!63=TaskFirst e !64=TaskNext. As cinco declaracoes e URLs primarias estao em win16-dynamic-api-evidence.json. O fluxo1:0570 procura QTNOTIFY; somente quando esse modulo existe carrega toolhelp.dll por1:0584. Inicializa estrutura de40bytes em1:05C8, percorre TaskFirst/TaskNext em1:05D7/05EB e compara o modulo+0A ao handle QTNOTIFY. Ao encontrar, pode postar mensagem0400 para a tarefa+4, dependendo do retorno de _CLEANUPTASK, e usa DirectedYield. Portanto esta rotina enumera a tarefa auxiliar de QuickTime; nao instala um callback de notificacao ToolHelp.

As fontes sao [VER](https://github.com/wine-mirror/wine/blob/master/dlls/ver.dll16/ver.dll16.spec) e [TOOLHELP](https://github.com/wine-mirror/wine/blob/master/dlls/toolhelp.dll16/toolhelp.dll16.spec). A existencia das DLLs externas nao foi pressuposta nem testada por carregamento.

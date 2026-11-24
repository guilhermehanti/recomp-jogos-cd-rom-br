# CFG estático: funções, jump tables, vtables e chamadas indiretas

Análise estática integral dos 49 segmentos (40 de código, 9 de dados) de `BIGJOBNG.EXE`, gerada por `python tools/map_recomp_cfg.py` e gravada em `analysis/recomp-cfg.json`. Os endereços usam segmento decimal e offset hexadecimal.

## Sumário estrutural para recompilação

| Métrica | Quantidade | Observação para o lifter/runtime |
|---|---:|---|
| Segmentos de código / dados | 40 / 9 | Código: `seg01`–`seg40`; dados: `seg41`–`seg49` (`DGROUP = 49`) |
| Funções descobertas | 3.336 | União de alvos `callw`, alvos de relocações far de código, prólogos binários e exports NE |
| Prólogos far Win16 (`inc bp; push bp; mov bp, sp`) | 2.924 | Convenção Borland C++ 1994 large/far model |
| Jump tables em CS (`jmpw *%cs:...`) | 99 | Decodificadas diretamente dos bytes de código; evita erro de disassembly linear (ex.: `9:145E`) |
| Vtables / tabelas de ponteiros far | 274 | Sequências de relocações tipo 3 espaçadas de 4 bytes (com suporte a slots nulos de 4 bytes) |
| Transferências indiretas totais | 2.016 | Classificadas por padrão abaixo |
| Stubs QuickTime em `seg01` | 127 | 17 referenciados, 110 sem referência direta e **0** hits em relocações, vtables ou jump tables |

## Classificação das 2.016 transferências indiretas

| Categoria | Quantidade | Papel no runtime recompilado |
|---|---:|---|
| `vtable_or_fptr_lcall` | 1.665 | Despacho virtual C++ (`lcall *%es:off(%bx)` / `%si` / `%di`) resolvido pela tabela de 274 vtables e mapa `(seg, off) -> fn` |
| `quicktime_dispatcher` | 129 | `lcall *DS:1546` (`QTIM!_EntryPoint`) e `lcall *DS:154E` (`QTIMCMGR!_EntryPoint`) nos stubs de `seg01` |
| `cs_jump_table` | 99 | `switch` compilado em `CS`; alvos decodificados em `recomp-cfg.json` |
| `near_indirect_call` | 59 | Chamadas indiretas near no mesmo segmento (majoritariamente CRT/runtime) |
| `stack_fptr_lcall` | 55 | Ponteiro far passado na pilha (`lcall *off(%bp)`), como callbacks de animação/cena |
| `mem_fptr_lcall` | 6 | Ponteiros far globais em `DGROUP`: `1:2FEB` (`DS:1D58`), `1:3F96/3FB0/3FB4` (`DS:1F12/1F16/1F1A`), `40:1FB9` (`DS:2376`), `40:24E3` (`DS:237A`) |
| `far_indirect_jmp` | 2 | `37:1F5E` (`ljmp *%es:4(%bx)`) e `37:1F6B` (`ljmp *%es:8(%bx)`) — delegação/thunk far |
| `other_indirect` | 1 | `1:3275` (`jmpw *0x20A8`) — handler indireto do CRT de ponto flutuante (`WIN87EM`) |

## Fechamento da superfície QuickTime (17 vs. 110 stubs)

O cruzamento de `api-boundaries.json` com todas as 13.487 relocações, 274 vtables e 99 jump tables confirma:

- **17 stubs** de `seg01` possuem referências diretas (12 `QTIM` + 5 `QTIMCMGR`), com corpos e contratos já verificados em [30-VIDEO-FRONTEIRA.md](30-VIDEO-FRONTEIRA.md) e [36-QUICKTIME.md](36-QUICKTIME.md).
- Os **110 stubs restantes** têm **zero** referências em relocações para `seg01` e **zero** entradas em jump tables de `seg01`. Portanto, a fronteira QuickTime para o recomp está fechada nos 17 stubs (ou, alternativamente, na substituição do player no segmento 33).

## Implicações para o recompilador estático

1. **Despacho indireto**: um tradutor `(seg16, off16) -> fn_ptr` cobrindo as 3.336 entradas descobertas (mais os 3 exports NE `33:3722`, `38:3178`, `38:33BC`) resolve as 1.665 chamadas virtuais, 55 chamadas via pilha e 6 chamadas via `DGROUP` sem emular instrução a instrução.
2. **Jump tables**: os 99 blocos `jmpw *%cs:...` devem ser emitidos como `switch` nativos em C usando os alvos já extraídos em `recomp-cfg.json`, evitando que bytes de tabela sejam interpretados como código pelo lifter.
3. **Verificação de funções órfãs**: nenhuma das 99 jump tables nem das 274 vtables aponta para `10:139A` (áudio `4367`) nem para `27:1753` (Memória rótulos `30`/`32`), reforçando a classificação de código não alcançável estaticamente.

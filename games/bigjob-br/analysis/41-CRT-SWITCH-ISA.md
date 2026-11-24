# CRT Borland, switches chave-valor, ISA 386, inicializadores estáticos e cursores

Fechamento dos 5 últimos pontos estáticos pré-recompilação de `BIGJOBNG.EXE`, gerado por `python tools/map_crt_switch_isa.py` e gravado em `analysis/crt-switch-isa.json` (com atualização de `analysis/recomp-cfg.json`).

## 1. Todas as 17 instruções `INT` (exclusivamente em `seg01`, CRT Borland)

| Endereço | Instrução | Subfunção DOS (`AH`/`AX`) | Função CRT / contrato no runtime |
|---|---|---|---|
| `1:005F` | `int 0x21` | `AH=30h` | Get DOS Version (bootstrap, grava em `DS:0042`) |
| `1:00E4`, `1:00F0` | `int 0x21` | `AH=4Ch` | Terminate Process (`_exit`) |
| `1:1E62` | `int 0x21` | `AX=4400h` | IOCTL Get Device Info (`isatty`) |
| `1:1E8D` | `int 0x21` | `AH=42h` | `lseek` |
| `1:1EAA` | `int 0x21` | `AH=41h` | `unlink` / `remove` |
| `1:2F31`, `2F3A`, `2F44`, `2F4D` | `int 0x21` | `AH=2Fh`, `1Ah`, `4Eh`, `1Ah` | `findfirst` (salva/define/restaura DTA + FindFirst) |
| `1:2F6A`, `2F73`, `2F77`, `2F80` | `int 0x21` | `AH=2Fh`, `1Ah`, `4Fh`, `1Ah` | `findnext` (salva/define/restaura DTA + FindNext) |
| `1:2FA3` | `int 0x21` | `AH=56h` | `rename` |
| `1:3003` | `int 0x21` | `AH=40h` | `_write` |
| `1:4264` | `int 0x23` | `INT 23h` | `raise(SIGINT)` no `switch` de sinais `1:4258` |

## 2. Fechamento das 99 jump tables em CS (13 switches chave-valor Borland)

Todas as 13 jump tables que usam o idioma Borland de busca linear chave-valor (`movw $count, %cx; movw $base, %bx; ... jmpw *%cs:<2*count>(%bx)`) foram decodificadas e integradas em `analysis/recomp-cfg.json`:

| Site | Base em CS | Entradas | Papel |
|---|---|---:|---|
| `1:3371`, `1:36BA` | `0x34D1`, `0x3793` | 16, 16 | Despacho interno do CRT |
| `1:4258` | `0x429B` | 6 | `signal` / `raise` (`2, 4, 8, 11, 15, 22`) |
| `6:05EF` | `0x0627` | 9 | Tradutor de IDs de progresso de atividades (`4, 6, 12, 15, 18, 19, 23, 31, 32`) |
| `6:298F`, `6:2CA1` | `0x2ACB`, `0x2CD1` | 10, 11 | Despacho de controles/teclas CG |
| `12:0C2A` | `0x0D47` | 5 | Teclado da cabine (`1, 2, 3, R, r`) |
| `17:1A7F` | `0x1B68` | 9 | Símbolos de grade FM (`-, ., _, a, e, f, ...`) |
| `21:033F` | `0x053C` | 10 | Teclado SG (`E, F, H, L, W, e, f, h, l, w`) |
| `23:1134` | `0x11AE` | 9 | Teclado SB (`$, %, 1, 2, 3, 4, ...`) |
| `36:0B6F` | `0x0D88` | 8 | Carregador de blocos MMS (`8000, C000..C004, ...`) |
| `38:31EB` | `0x3355` | 7 | `WndProc` principal (`WM_DESTROY`, `WM_CLOSE`, `WM_NCACTIVATE`, `WM_CHAR`, `WM_COMMAND`, `WM_QUERYNEWPALETTE`, ...) |
| `38:342F` | `0x3533` | 9 | `WndProc` canvas (`WM_DESTROY`, `WM_MOVE`, `WM_PAINT`, `WM_SETCURSOR`, `WM_MOUSEMOVE`, `WM_LBUTTONDOWN`, ...) |

Com isso, **99 de 99** jump tables em CS estão decodificadas (`0` com `entries_count == 0`).

## 3. Subconjunto ISA 386 e relocações NE

- **ISA**: 200 mnemônicos únicos nos 40 segmentos de código; **15.337** instruções 32-bit (386, prefixo `0x66`: `eax`..`edi`, `pushl`, `cmpl`, `movl`, etc.). O lifter deve modelar registradores de 32 bits completos em modo 16-bit.
- **`STD` (`DF=1`)**: apenas **2** ocorrências em todo o executável, ambas em `seg01` (`1:1375`, `1:181D`, `memmove` reverso do CRT).
- **Tipos de relocação NE (13.487 registros)**: apenas 8 combinações `(source_type, flags)`: `(3,0)=10947` (far ptr interno), `(2,4)=1944` (selector additive), `(3,1)=272` (far ptr import ordinal), `(5,1)=129` (offset import ordinal: 128 `__AHSHIFT` + 1 `WIN87EM!1`), `(5,7)=98` (OS fixup x87 em `seg01`), `(2,0)=56` (selector interno), `(5,0)=40` (offset interno), `(2,1)=1` (selector `WIN87EM!1`).

## 4. `WinExec` (`KERNEL!166`) em `22:055C`

- `22:053E` testa a existência do protetor de tela **`BIGSAVER.SCR`** via `OpenFile(..., OF_EXIST=0x4000)`. Se ausente, exibe `"Unable to locate BIGSAVER.SCR"` (`22:03D3`). Se presente, chama `WinExec(..., SW_SHOW=5)` em `22:055C` (erro `< 0x20` exibe `"Unable to execute BIGSAVER.SCR"`, `22:03F7`). Não faz parte do loop principal de jogo.

## 5. Tabela `_INIT_` / `_EXIT_` C++ em `DGROUP` e `LoadCursor` (`USER!173`)

### Construtores e destrutores estáticos C++ (`49:20B8`–`49:2112`)
O CRT Borland percorre esta tabela de registros de 6 bytes `(type, priority, far_ptr)` em `DGROUP` antes de `WinMain` e no encerramento:

- **11 construtores estáticos** (`type=1, prio=32`, `49:20B8`–`49:20FA`):
  `2:15CC`, `14:3180`, `15:1BAE`, `19:24B8`, `19:2662`, `20:08B7`, `26:2F6A`, `29:18D3`, `30:04B0`, `37:13CA`, `39:22EF`.
- **2 destrutores estáticos** (`49:2106`–`49:2112`):
  `1:40F0` (prio 9) e `30:04BF` (prio 32).

> [!IMPORTANT]
> O runtime do recomp precisa executar os 11 construtores estáticos acima antes de entrar em `WinMain` (ou executar o loop `_INIT_` original de `seg01`), garantindo a inicialização de todos os objetos globais C++.

### `LoadCursor` (`USER!173`, 7 sites)
- **Cursores padrão do sistema (`hInstance = 0`)**: `0x7F00` (`IDC_ARROW` em `2:024F`, `2:02AE`, `32:07B4`, `38:3713`), `0x7F01` (`IDC_IBEAM` em `32:07DA`), `0x7F02` (`IDC_WAIT` em `32:07CA`).
- **Cursores embutidos no NE (`RT_CURSOR` IDs `1..11`)**: carregados dinamicamente por `32:0807` com `hInstance` do módulo.

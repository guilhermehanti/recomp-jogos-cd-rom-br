# Bootstrap Win16, memória segmentada e fronteira x87

Análise estática de `BIGJOBNG.EXE`, gerada por `python tools/map_bootstrap_x87.py` e gravada em `analysis/bootstrap-x87.json`.

## Compilador e cabeçalho NE

- **Compilador identificado**: `Borland C++ - Copyright 1994 Borland Intl.` (`49:004B`, com mensagens de runtime em `49:1E9E`–`49:205F`).
- **Ponto de entrada**: `CS:IP = 1:0000`, `SS:SP = 49:0000`, `DGROUP = 49`, flags NE `0x030A`, sistema operacional `2` (Windows 16-bit).

## Sequência de bootstrap (`1:0000`–`1:00B5`)

1. `1:0006`: chama `40:02D7` com argumento `0` (inicialização preliminar de segmento/CRT).
2. `1:000F`: chama `KERNEL!91` (`InitTask`). Se `AX == 0`, aborta em `1:00E6`.
3. `1:001B`–`1:0032`: grava registradores retornados por `InitTask` em `DGROUP`: `ES -> DS:0038` (hInstance/PSP), `DI -> DS:003A` (hInstance), `SI -> DS:003C` (hPrevInstance), `BX -> DS:003E` (cmdLine off), `DX -> DS:0040` (nCmdShow).
4. `1:003A`–`1:0043`: zera a área BSS de `DGROUP` de `49:2118` até `49:240E` (758 bytes) via `rep stosb`.
5. `1:0048`: chama `KERNEL!30` (`WaitEvent(0)`).
6. `1:0051`: chama `USER!5` (`InitApp(hInstance)`). Se `AX == 0`, aborta em `1:00E6`.
7. `1:005D`–`1:0061`: `INT 21h / AH=30h` (versão DOS), guarda em `DS:0042`.
8. `1:0064`–`1:0077`: chama `KERNEL!132` (`GetWinFlags`), testa modo protegido (`0x0001`) e FPU/80x87 (`0x0400`).
9. Em seguida inicializa o CRT (incluindo `WIN87EM` quando presente) e transfere para `WinMain` / aplicação `2:190A`.

> [!NOTE]
> Para o recompilador, `1:0000` não precisa emular `INT 21h` nem `InitTask`/`WaitEvent`/`InitApp` reais: basta o runtime preencher os campos `DS:0038..0042`, zerar `49:2118..240E` e chamar o corpo inicializador da aplicação (ou manter shims triviais para esses 4 imports).

## Segmentos de dados (`seg41`–`seg49`)

| Segmento | Tamanho em disco | `minalloc` | BSS (zerado) | Relocações | Papel |
|---:|---:|---:|---:|---:|---|
| 41–48 | Variável | Igual ao disco | 0 | Variável | Tabelas estáticas far (ex.: tabelas de tema da cabine em `seg44`, strings/tabelas constantes) |
| 49 (`DGROUP`) | 9.230 bytes | 9.230 bytes | + pilha/heap local | 102 | Globais (`DS:0000..240E`), dispatchers QuickTime (`1546`/`154E`), áudio (`222C..2368`), tema (`02A0`) |

Os valores exatos de offset/tamanho de cada um dos 9 segmentos de dados estão em `analysis/bootstrap-x87.json`.

## Aritmética de ponteiros huge: `KERNEL!113` (`__AHSHIFT`)

Todos os **128 sites** de `KERNEL!113` no executável seguem **exatamente o mesmo padrão de instrução**:

- `movw $__AHSHIFT, %cx` (codificado no disco como `movw $0xFFFF, %cx` com relocação `source_type=5, flags=1`).
- No Win16 em modo protegido, `__AHSHIFT = 3` (incremento de seletor `1 << 3 = 8`, ou `__AHINCR = 8`), usado pelo compilador Borland C++ para cruzar fronteiras de 64 KB em ponteiros `huge` (`GlobalAlloc` > 64 KB, como buffers de imagens PIC e tabelas grandes).
- **Contrato no runtime do recomp**: manter uma tabela de seletores virtuais onde alocações > 64 KB recebem seletores consecutivos com passo `8` (`1 << 3`) mapeando fatias contíguas de 64 KB do buffer linear alocado no host. Com isso, todas as 128 operações com `__AHSHIFT` funcionam sem reescrita manual.

## Fronteira x87 e `WIN87EM!1` (`__fpMath`)

A varredura dos 40 segmentos de código demonstra um resultado decisivo para a complexidade do recomp:

1. **Todos os 98 fixups OS** (87 `WAIT` + 11 `NOP`) estão concentrados **exclusivamente no segmento 1** (`seg01`, biblioteca CRT matemática do Borland C++ 1994).
2. **Todos os 6 sites de `WIN87EM!1`** estão em `1:3230`–`1:3274` (rotinas de inicialização/encerramento do emulador FPU do Borland CRT):
   - `1:3234`: `BX=1` (`__fpMath` consulta/controle)
   - `1:323D`: `BX=2` (encerramento FPU)
   - `1:3257`/`1:325D`: grava ponteiro far de `WIN87EM!1` em `DS:20A4`/`DS:20A6`
   - `1:3262`: `BX=0` (inicialização FPU)
   - `1:3270`: `BX=3`, `DX:AX = 1:321F` (registra handler de exceção FPU)
3. **Uso real de x87**: das 199 instruções com mnemônico `f*`/`wait` no disassembly linear, **187 estão em `seg01`** (rotinas CRT como `_ftol` e aritmética de ponto flutuante). As 12 ocorrências isoladas fora de `seg01` (em `seg06`, `seg09`, `seg14`, `seg19`, `seg25`, `seg29`, `seg40`) são bytes de tabelas de salto/dados interpretados linearmente.
4. **Implicação para o recomp**: não há necessidade de emular `WIN87EM.DLL` nem fixups de emulador 80287. No runtime x64, `WIN87EM!1` pode ser um stub no-op bem-comportado (`AX=0`), e as rotinas matemáticas de `seg01` podem ser recompiladas com `double` IEEE-754 nativo (ou interceptadas diretamente pelas funções C padrão equivalentes).

# Pipeline gráfica GDI, recursos NE e formatos PIC

Análise estática dos pontos de chamada GDI/USER, da tabela de recursos NE de `BIGJOBNG.EXE` e de todas as 4.744 entradas de `BIGJOBCD.PIC` e `BIGJOBHD.PIC` na ISO brasileira (`D:\emulacao pc antigo\BIGJOBCD.ISO`), gerada por `python tools/map_gdi_pic.py` e gravada em `analysis/gdi-pic-contracts.json`.

## Superfície GDI de apresentação e texto

| API Win16 | Sites | Contrato observado nos chamadores |
|---|---:|---|
| `GDI!439` (`StretchDIBits`) | 3 | Todos usam `ROP = 0x00CC0020` (`SRCCOPY`). **Tela (`37:229A`, `37:2324`)**: `iUsage = 1` (`DIB_PAL_COLORS`, 8-bit indexado referenciando a paleta lógica ativa). **Impressão (`34:25D9`)**: `iUsage = 0` (`DIB_RGB_COLORS`). |
| `GDI!360` (`CreatePalette`) | 2 | `31:1167` (paleta principal da aplicação, guardada em `objeto+3D2`) e `31:3347` (paleta de 256 entradas). Layout `LOGPALETTE` Win16 de 256 entradas confirmado em [37-WIN16-ESTRUTURAS.md](37-WIN16-ESTRUTURAS.md). |
| `USER!282` (`SelectPalette`) | 6 | `31:1180`, `31:11BF`, `31:334F`, `31:3388`, `38:3227`, `38:3242` (incluindo resposta a `WM_QUERYNEWPALETTE` / `WM_PALETTECHANGED` em `seg38`). |
| `USER!283` (`RealizePalette`) | 3 | `31:118D`, `38:3232`, `38:324A`. |
| `GDI!57` (`CreateFontIndirect`) | 2 | `37:0EA7` (selecionada com `GDI!45` `SelectObject`) e `38:3AC1`. Usa `LOGFONT16` de 50 bytes. |
| `USER!85` (`DrawText`) | 2 | `31:1FBC` e `31:2188`, precedidos por `SetBkMode`/`SetTextColor`/`GetTextExtent`. |
| `USER!221` (`ScrollDC`) | 4 | Rolagem de regiões da superfície de janela. |
| `GDI!53/68/38/377/378` | 9 | Pipeline de impressão CT (`CreateDC`, `DeleteDC`, `Escape(GETPHYSPAGESIZE=3)`, `StartDoc`, `EndDoc`), detalhado em [23-IMPRESSAO.md](23-IMPRESSAO.md). |

**Contrato para o runtime do recomp**: como `BIGJOBNG.EXE` não usa WinG, toda a renderização em tela converge para apenas **2 sites de `StretchDIBits` (`37:229A` e `37:2324`)** com `DIB_PAL_COLORS` + `SRCCOPY`, mais `DrawText` e `ScrollDC`. Um framebuffer 640×480 RGBA32 no host com tabela de paleta lógica de 256 entradas (atualizada por `CreatePalette`/`SelectPalette`/`RealizePalette`) atende integralmente a saída visual do jogo.

## Recursos NE embutidos em `BIGJOBNG.EXE`

Total de 29 entradas na tabela de recursos NE:
- **11 cursores** (`RT_CURSOR` + `RT_GROUP_CURSOR`)
- **1 ícone** (`RT_ICON` + `RT_GROUP_ICON`)
- **1 tabela de aceleradores** (`RT_ACCELERATOR`, ID `1`)
- **4 blocos de strings** (`RT_STRING`, 20 strings brasileiras em CP1252)

### Aceleradores (`RT_ACCELERATOR` ID 1)

Explica o mapeamento das teclas físicas de seta e `F1` no laço de mensagens (`TranslateAccelerator` em `38:3096`):

| Tecla Win16 (`FVIRTKEY`) | Código virtual | `cmd_id` (`WM_COMMAND`) |
|---|---:|---:|
| `VK_RIGHT` | `0x0027` | `101` |
| `VK_LEFT` | `0x0025` | `102` |
| `VK_UP` | `0x0026` | `103` |
| `VK_DOWN` | `0x0028` | `104` |
| `VK_F1` | `0x0070` | `105` |

### Strings NE brasileiras (CP1252)

- **Identidade e bancos**: `1 = "Big Job"`, `700 = "BIGJOBCD"`, `701 = "BIGJOB.MMS"`, `702 = "BIGJOBCD.PIC,BIGJOBHD.PIC"`, `703 = "BIGJOB.SND,BIGVOX.SND"`, `704 = "BIGJOB.MOV"`.
- **Mensagens de erro (`2002`–`2015`)**: `"Sem Erro."`, `"Memória Insuficiente."`, `"Não foi possível criar janela."`, `"Bad palette."`, `"Diretórios não encontrados."`, `"Insira o CD."`, `"Arquivo MMS não encontrado."`, `"Arquivo PIC não encontrado."`, `"Arquivo SND não encontrado."`, `"Arquivo MOV não encontrado."`, `"Não foi possível iniciar o áudio."`, `"Não foi possível iniciar o QuickTime."`, `"Bad or missing hot key resource."`, `"Mysterious error."`.

## Taxonomia completa das 4.744 entradas PIC

A varredura direta na ISO identificou a tabela de tipo/flags do índice no offset `108 + 8 * count` (`BE16[count]`), cobrindo **100%** das 4.744 entradas sem exceção:

| Tipo no índice (`108+8*N`) | `BIGJOBCD.PIC` (4.119) | `BIGJOBHD.PIC` (625) | Total | Natureza observada |
|---:|---:|---:|---:|---|
| `0x0056` | 3.330 | 384 | 3.714 | Sprites/imagens compactadas com transparência/linhas (cabeçalho `W, H, 0x0056, ...`) |
| `0x0054` | 717 | 116 | 833 | Imagens com cabeçalho `W, H, 0x0054, ...` |
| `0x0006` | 0 | 113 | 113 | Sprites de direção `DS*` em HD (`0064 0064 0006 0001 ...`) |
| `0x0055` | 41 | 9 | 50 | Fundos/telas de 640×480 com cabeçalho `W, H, 0x0055, ...` |
| `0x0057` | 21 | 2 | 23 | Variante `W, H, 0x0057, ...` |
| `0x0009` | 10 | 0 | 10 | Exatamente as 10 telas `*splash` (`3318`–`3327`: `CGsplash`..`WCsplash`), comprimidas (bit 3 = LZSS/bloco sem cabeçalho `W,H` explícito no payload) |
| `0x0004` | 0 | 1 | 1 | Entrada única no HD |

**Impacto no recomp**: na recompilação estática de `BIGJOBNG.EXE`, o próprio decodificador nativo de `seg34`/`seg31` é recompilado e trata todos os 7 tipos acima automaticamente, entregando um DIB 8-bit pronto para `StretchDIBits`. Só seria necessário reimplementar os 7 decodificadores caso se optasse por substituir o subsistema PIC em vez de recompilá-lo.

# Clássicos de CD-ROM do Brasil no Windows 10/11

Coleção de **Jogos clássicos de CD-ROM lançados no Brasil**, adaptados e recompilados para rodar de forma nativa e simples no **Windows 10 e Windows 11 (64-bit)** — sem precisar instalar emuladores, máquinas virtuais, Windows 95/98 ou drivers antigos.

> **Como funciona (para qualquer pessoa):**
> 1. Clique em **Releases** (na barra lateral direita do GitHub) e baixe o `.zip` do jogo desejado.
> 2. Extraia o `.zip` em qualquer pasta do seu computador.
> 3. Abra o executável (`.exe`), **selecione a imagem do seu CD original** (`.iso` ou `.cue`/`.bin`) na primeira vez e **jogue!**
>
> Nas próximas vezes, o jogo abre direto — sem precisar escolher a ISO de novo.
>
> **Projeto em andamento:** novos títulos clássicos de CD-ROM lançados no Brasil serão adicionados ao repositório com o passar do tempo.

---

## Jogos Disponíveis

| Jogo | Formato aceito na 1ª vez | Atalhos úteis dentro do jogo |
|---|---|---|
| **101 Dálmatas: Livro Animado Interativo** | `.iso` (lê direto da ISO, sem extrair) | `F5` abre o menu de configurações (gráficos, shaders, tela cheia, áudio) |
| **O Corcunda de Notre Dame: Livro Animado Interativo** | `.iso` (lê direto da ISO, sem extrair) | `F5` abre o menu de configurações (gráficos, shaders, tela cheia, áudio) |
| **A Casa de Franklin** | `.iso` (lê direto da ISO, sem extrair) | `F5` abre o menu de configurações (gráficos, shaders, tela cheia, áudio) |
| **Oficina de Invenções** | `.cue` + `.bin` *(recomendado para música do CD)* ou `.iso` | `Ctrl+I` abre o menu de tela cheia, escala e filtros gráficos |
| **Orly: Histórias para Desenhar** | `.iso`, `.cue` ou `.bin` | `Alt+Enter` tela cheia · `F1`–`F4` filtros gráficos |
| **Big Job** | `.iso` (lê direto da ISO, sem extrair) | Recompilação estática nativa Win16 → Windows x64 |

> **Precisou mover a ISO de pasta ou selecionar outra imagem?**
> - **101 Dálmatas, Corcunda e Franklin:** aperte `F5` no jogo → **Trocar ISO...** (se a ISO for movida, o jogo também pede automaticamente ao abrir).
> - **Oficina de Invenções e Orly:** segure `Shift` ao abrir o `.exe` caso queira reextrair de outra imagem.
> - **Big Job:** apague o arquivo `saves\recomp.ini` para escolher outra `.iso`.

---

## Importante: Arquivos Originais dos Jogos

**Nenhum arquivo original dos jogos está incluído neste repositório ou nos downloads.**  
Você precisa possuir sua própria cópia/imagem (`.iso` ou `.cue`/`.bin`) da edição brasileira de cada CD-ROM. Na primeira execução, o programa valida automaticamente se a imagem corresponde à versão brasileira suportada.

### Edições Brasileiras Suportadas e Hashes de Verificação (SHA-256)

Use a tabela abaixo para conferir se a imagem do seu CD original corresponde exatamente à edição brasileira suportada (no Windows PowerShell: `Get-FileHash "arquivo.iso" -Algorithm SHA256`):

| Jogo (Edição PT-BR) | Volume ID do CD | Arquivo / Imagem verificada | SHA-256 |
|---|---|---|---|
| **Disney's Livro Animado Interativo: 101 Dálmatas** | `LAI101` | ISO completa (410.056.704 bytes) | `78B1F3FB1BB6722DCF09CEECFC55537DB18EB5578E409A19E7E72D3A0F56024D` |
| **Disney's Livro Animado Interativo: O Corcunda de Notre Dame** | `NOTRE_DAME` | ISO completa (575.694.848 bytes) | `B420C6384DBC82A3142EE2676E945FCEAD9F06C77A368FC243998A48D0E936A6` |
| **A Casa de Franklin** | `FRANKLIN` | ISO completa (129.843.200 bytes) | `7E490DD7369769B1307CE43A9A427471FAC0E315B9174B853D66D339E592E9AB` |
| **Oficina de Invenções** *(Discovery Channel Multimídia)* | CD misto (Dados + Faixa de Áudio) | `BRINV3R.EXE` dentro do CD (237.056 bytes) | `B7E651C590EF3FED96225DECA792A36D07BA4252A2F153B359C8164AC9FB311F` |
| **Orly: Histórias para Desenhar** *(Brøderbund / Brasoft)* | CD-ROM PT-BR (`ARCHIVE\ARCHIVE.Z`) | `Orly32.exe` dentro do `ARCHIVE.Z` (655.360 bytes) | `562834EC09CC674C7D2CBA2E2B1AE1A002AD0F14BD88C92550E357E5B0C44666` |
| **Big Job** *(Discovery Channel Multimídia)* | `BIGJOBCD` | ISO completa (518.621.184 bytes) · `BIGJOBNG.EXE` (667.568 bytes) | ISO: `33E51963B14770209300CF6C682415EB2BFACF62840368B55C2A52A467AAD4E8`<br>EXE: `E483AC5986FAEA03F683168B6B05EAE1E7CA9A5EED8449B0E880849362D65A1A` |

> **Dica para Oficina de Invenções:** se a sua cópia do CD estiver em formato `.cue` + `.bin` (CD misto com faixa de áudio), selecione o arquivo **`.cue`** para que a música de fundo original do CD também seja extraída automaticamente.

---

## Para Desenvolvedores (Código-Fonte e Compilação)

* **`games/oficina-de-invencoes/`** — Código-fonte da correção de fontes (`txtdiag.dll`), patch automático de 28 bytes do `BRINV3R.EXE` e launcher nativo ISO/CUE/WAV (`build.py`).
* **`games/orly-br/`** — Código-fonte `orlyfix` adaptado para o executável brasileiro (`-DORLY_BR`) + launcher nativo com extrator ISO 9660 e InstallShield 3.0 em C puro (`build.py`).
* **`games/bigjob-br/`** — Lifter estático Win16 → C, runtime nativo, VFS de ISO, player QuickTime e ferramentas de análise feitos do zero (`tools/build_recomp.py`).
* **`games/scummvm-standalone/`** — Perfis JSON e gerador automatizado dos executáveis independentes (código-fonte completo GPLv3+ do motor customizado baseado no ScummVM disponível em [guilhermehanti/custom-scummvm-cdrom-engine](https://github.com/guilhermehanti/custom-scummvm-cdrom-engine)).
* **`tools/cdlauncher/`** — Biblioteca C compartilhada (leitor ISO 9660/Joliet, parser CUE/BIN, extrator de CD-Audio WAV, descompactador InstallShield 3.0 / PKWARE DCL, SHA-256 e interface Win32).

---

## Créditos e Licenças

Veja [CREDITS.md](CREDITS.md) para a lista completa de autores, repositórios originais e licenças (`GPLv3+`, `MIT`, `CC0 1.0`).

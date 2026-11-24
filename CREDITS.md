# Créditos e Licenças

Este repositório reúne **Jogos clássicos de CD-ROM localizados para o Português do Brasil (PT-BR)** adaptados para rodar nativamente no Windows 10/11 (x64), sem emuladores manuais, máquinas virtuais ou configurações complexas.

> **Aviso legal:** Nenhum arquivo original dos jogos (executáveis originais, gráficos, áudios, vídeos ou imagens `.iso`/`.bin`/`.cue`) é distribuído neste repositório ou nos Releases. O usuário deve fornecer sua própria imagem de CD original. Todos os direitos sobre os jogos originais pertencem aos seus respectivos detentores.

---

## Tabela de Créditos e Licenças

| Jogo | Tipo de Projeto | Projeto Base / Upstream | Licença | Minhas Contribuições / Adaptações PT-BR |
|---|---|---|---|---|
| **101 Dálmatas: Livro Animado Interativo** | Standalone (motor `mediastation`) | [ScummVM](https://github.com/scummvm/scummvm) — ScummVM Team | **GPLv3+** | Fork customizado `ScummVM-BR` com detecção PT-BR, leitura direta de ISO 9660/Joliet sem extração, modo standalone sem launcher/branding, menu in-game no `F5`, download de shaders com CA nativa do Windows e build estático |
| **O Corcunda de Notre Dame: Livro Animado Interativo** | Standalone (motor `mediastation`) | [ScummVM](https://github.com/scummvm/scummvm) — ScummVM Team | **GPLv3+** | Mesmo sistema standalone `ScummVM-BR` + detecção da edição brasileira |
| **A Casa de Franklin** | Standalone (motor `director`) | [ScummVM](https://github.com/scummvm/scummvm) — ScummVM Team | **GPLv3+** | Mesmo sistema standalone `ScummVM-BR` + correção específica de paletas de cores (`engines/director/castmember/bitmap.cpp`) |
| **Oficina de Invenções** | Wrapper + Patch + Launcher ISO/CUE | [`leo-dragons/invention-studio-rescue`](https://github.com/leo-dragons/invention-studio-rescue) (`winmm.dll` — CD Audio + escala HD `Ctrl+I`) | **MIT** | Adaptação para a versão brasileira (`BRINV3R.EXE`), correção de fontes sem antialiasing (`txtdiag.dll` / `font-fix.c`), patch de 28 bytes (profundidade de cor 24-bit + bypass CD + import `txtdiag.dll`), `system\invent.ini` portátil e launcher nativo 1-clique com extrator ISO 9660 / CUE+BIN / CD-Audio WAV |
| **Orly: Histórias para Desenhar** | Wrapper + Launcher ISO/IS3 | [`matthewbloch/orlys-draw-a-story-fixes`](https://github.com/matthewbloch/orlys-draw-a-story-fixes) — Matthew Bloch (`orlyfix` / `winspool.dll`) | **CC0 1.0** (Domínio Público) | Adaptação completa de offsets e hooks para o executável brasileiro (`ORLY_BR` em `orly32.h` e `orlyfix.c`) + launcher nativo 1-clique em C com leitor ISO 9660/Joliet e descompactador InstallShield 3.0 / PKWARE DCL próprio (sem dependência de Python, 7-Zip ou PowerShell) |
| **Big Job (Caminhões, Tratores e Máquinas)** | Recompilação estática Win16 → C/x64 | **100% autoral (feito do zero)** | **MIT** | Engenharia reversa completa do executável Win16 `BIGJOBNG.EXE`, lifter estático x86-16/386 → C (40 segmentos), runtime Win16 nativo, VFS de ISO, player QuickTime e filtros de vídeo |

---

## Bibliotecas e Referências Técnicas

- **ScummVM Team & Contribuidores:** Motor `mediastation`, motor `director`, GUI e sistema de shaders (GPLv3+). Código-fonte completo da nossa versão customizada disponível em [guilhermehanti/cdrom-motor](https://github.com/guilhermehanti/cdrom-motor).
- **leo-dragons (`invention-studio-rescue`):** Proxy `winmm.dll` para emulação de CD Audio via CUE/WAV e escalonamento HD (`Ctrl+I`) em *Invention Studio* / *Taller de Inventos* (Licença MIT — veja [MIT-invention-studio-rescue.txt](LICENSES/MIT-invention-studio-rescue.txt)).
- **Matthew Bloch (`orlys-draw-a-story-fixes`):** Correções originais de compatibilidade para *Orly's Draw-A-Story* (Licença CC0 1.0 — veja [CC0-1.0.txt](LICENSES/CC0-1.0.txt)), com referências prévias a `dxwnd`, `DDrawCompat`, `Sharp-Bilinear-Shaders` e `Ultimate ASI Loader`.
- **Especificação PKWARE DCL / InstallShield 3.0:** O extrator nativo em `tools/cdlauncher/is3_extract.c` implementa a descompressão do formato público PKWARE Data Compression Library (Implode/Explode) e cabeçalhos InstallShield 3.0 em C puro.

## Textos Completos das Licenças

- [LICENSES/GPL-3.0.txt](LICENSES/GPL-3.0.txt)
- [LICENSES/MIT-invention-studio-rescue.txt](LICENSES/MIT-invention-studio-rescue.txt)
- [LICENSES/CC0-1.0.txt](LICENSES/CC0-1.0.txt)

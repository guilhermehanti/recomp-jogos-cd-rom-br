# Standalones ScummVM-BR (101 Dálmatas, O Corcunda de Notre Dame e A Casa de Franklin)

Esta pasta contém os perfis JSON, ícones e o gerador automatizado (`build_standalone.py`) utilizados sobre o nosso fork
customizado **`ScummVM-BR`** (Licença **GPLv3+**) para produzir executáveis 100% independentes no Windows 10/11 x64.

## Jogos gerados

- **101 Dálmatas: Livro Animado Interativo** (`games/101dalmatas.json`, motor `mediastation`)
- **O Corcunda de Notre Dame: Livro Animado Interativo** (`games/corcunda.json`, motor `mediastation`)
- **A Casa de Franklin** (`games/franklin.json`, motor `director` + correção de paleta)

## Recursos integrados no ScummVM-BR

- Leitura direta da `.iso` (ISO 9660 / Joliet) sem extração nem drive virtual.
- Seletor gráfico nativo do Windows na primeira execução + validação por MD5.
- Sem launcher, sem splash screen e sem branding externo; menu de configurações completo no **`F5`** (incluindo OpenGL, filtros e download de shaders via CA nativa do Windows).
- Executável estático único (sem DLLs externas).

## Código-fonte completo (GPLv3+)

O código-fonte completo do motor customizado (incluindo `common/formats/iso9660.*`, `common/fs-iso.*`, `base/standalone.*`,
detecções PT-BR e correção de paleta de *A Casa de Franklin*) está publicado em [guilhermehanti/custom-scummvm-cdrom-engine](https://github.com/guilhermehanti/custom-scummvm-cdrom-engine)
em conformidade com a licença GPLv3+.

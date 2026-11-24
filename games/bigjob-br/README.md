# Big Job (PT-BR) — Recompilação Estática Win16 → Windows x64

Recompilação estática **100% autoral** do executável Win16 `BIGJOBNG.EXE` (40 segmentos de código)
para Windows 10/11 64-bit nativo, com leitura direta da ISO (`BIGJOBCD.ISO`), sem emuladores ou máquinas virtuais.

## Estrutura

- `runtime/` — Runtime Win16 nativo em C (`cpu.h`, `cpu_ops.h`, `win16_mem.c`, `win16_api.c`, `iso_vfs.c`, `qt_movie.c`, `video_filter.c`).
- `src/` — Ponto de entrada (`recomp_main.c`) com seletor gráfico de ISO e suporte a saves portáteis.
- `tools/` — Lifter estático x86-16/386 → C (`recomp_lifter.py`), compilador paralelo (`build_recomp.py`) e ferramentas de análise.
- `tests/` — Testes de contratos do host e leitura de saves.
- `analysis/` e `01..06*.md` — Documentação técnica de engenharia reversa.

## Como compilar a partir da sua ISO original

Requer Visual Studio 2022 Build Tools + LLVM (`clang-cl`, `lld`) + Python 3:

1. Extraia o `BIGJOBNG.EXE` da sua ISO brasileira (`BIGJOBCD.ISO`) para `build/BIGJOBNG.EXE`.
2. Gere os segmentos em C localmente com `tools/recomp_lifter.py`.
3. Compile o executável nativo:

```powershell
python tools/build_recomp.py
```

> **Nota de direitos autorais:** Nenhum executável original (`BIGJOBNG.EXE`), código gerado (`seg*.c`),
> disassembly completo ou recurso da ISO está incluído neste repositório.

# Orly: Histórias para Desenhar (PT-BR)

Adaptação da versão brasileira de *Orly's Draw-A-Story* (**Orly: Histórias para Desenhar**) para Windows 10/11 x64 com launcher gráfico 1-clique.

## O que este projeto faz

1. **Launcher nativo 1-clique (`Orly - Historias para Desenhar.exe`):**
   - Na primeira execução, pede para o usuário escolher a imagem do CD (`.iso`, `.cue` ou `.bin`).
   - Lê diretamente o sistema de arquivos ISO 9660/Joliet e descompacta `ARCHIVE\ARCHIVE.Z` (InstallShield 3.0 / PKWARE DCL) em **C puro**, sem precisar de Python, 7-Zip ou PowerShell.
   - Valida o SHA-256 do `Orly32.exe` brasileiro (`562834EC09CC674C7D2CBA2E2B1AE1A002AD0F14BD88C92550E357E5B0C44666`).
   - Extrai automaticamente `ORLY\*.MHK` sobrescrevendo os stubs e instala `winspool.dll` + `orlyfix.ini` em `orly-data\`.
2. **Camada de compatibilidade PT-BR (`src/` → `winspool.dll` com `-DORLY_BR`):**
   - Baseada em [`matthewbloch/orlys-draw-a-story-fixes`](https://github.com/matthewbloch/orlys-draw-a-story-fixes) por Matthew Bloch (CC0 1.0), adaptada para os endereços e verificações do executável brasileiro.
   - Atalhos no jogo: **`Alt+Enter`** (tela cheia) e **`F1`–`F4`** (filtros gráficos).

## Como compilar

Requer Visual Studio 2022 Build Tools + LLVM (`clang-cl`, `lld`) + Python 3:

```powershell
python build.py
```

O pacote pronto para distribuição será gerado em `dist\Orly - Historias para Desenhar\`.

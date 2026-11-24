# Oficina de Invenções (PT-BR)

Adaptação completa da versão brasileira de *Invention Studio* (**Oficina de Invenções**) para Windows 10/11 x64 com launcher gráfico 1-clique.

## O que este projeto faz

1. **Launcher nativo 1-clique (`Oficina de Invencoes.exe`):**
   - Na primeira execução, pede para o usuário escolher a imagem do CD (`.cue` + `.bin` para incluir a música do CD, ou `.iso`).
   - Valida o SHA-256 do `BRINV3R.EXE` brasileiro (`B7E651C590EF3FED96225DECA792A36D07BA4252A2F153B359C8164AC9FB311F`).
   - Extrai automaticamente os arquivos do jogo e a faixa de CD-Audio (`music\track02.wav` + `music\music.cue`) para `oficina-data\`.
   - Aplica em memória/disco o patch binário de 28 bytes (profundidade de cor 24-bit, bypass da checagem legada de CD e import de `txtdiag.dll`).
   - Gera `system\invent.ini` portátil e copia `txtdiag.dll`, `winmm.dll` e `cdaudio.ini`.
2. **Correção de fontes (`src/font-fix.c` → `txtdiag.dll`):**
   - Intercepta `CreateFontA` / `CreateFontIndirectA` e força `NONANTIALIASED_QUALITY` para corrigir artefatos de texto no Windows moderno.
3. **Música de CD e Escala HD (`winmm.dll` + `cdaudio.ini`):**
   - Baseado em [`leo-dragons/invention-studio-rescue`](https://github.com/leo-dragons/invention-studio-rescue) (Licença MIT).
   - Pressione **`Ctrl+I`** dentro do jogo para configurar tela cheia, escala e filtros gráficos.

## Como compilar

Requer Visual Studio 2022 Build Tools + LLVM (`clang-cl`, `lld`) + Python 3:

```powershell
python build.py
```

O pacote pronto para distribuição será gerado em `dist\Oficina de Invencoes\`.
*(Para incluir `winmm.dll`, compile-o a partir de `leo-dragons/invention-studio-rescue` e coloque-o em `prebuilt\winmm.dll`.)*

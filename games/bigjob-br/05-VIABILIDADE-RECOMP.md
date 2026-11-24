# Viabilidade

| Estratégia | Trabalho e viabilidade | Obstáculos e riscos | Edição brasileira e distribuição |
|---|---|---|---|
| A · Recompilação estática | Alto. PCRecomp é aplicável para análise e candidato a lift, sem teste de build ainda. | 40 segmentos de código, callbacks far, 98 fixups OS, despacho indireto, x87 e QuickTime. Preservar comportamento do motor requer fechar as APIs do host. | Reutiliza os arquivos BR. Pode gerar executável nativo independente do emulador, com runtime para semântica segmentada e APIs antigas. |
| B · Reimplementação | Alto a muito alto enquanto o script e o DAT estiverem sem especificação. | Reproduzir comandos, fluxo, tempo, paletas, áudio, interação e salvamento. Risco de regressão de fidelidade. | Pode gerar aplicativo independente com runtime próprio, sem CPU original, mas exige testes extensos de equivalência. |
| C · Patch de compatibilidade | Baixo para alterações cirúrgicas depois de uma execução-base. Insuficiente sozinho para Windows x64. | O PE32 da abertura não muda a arquitetura do jogo. Patch a um arquivo NE mantém a necessidade de um ambiente Win16. | Preserva o conteúdo original. Pode corrigir caminhos, multimídia ou paletas sob uma camada de compatibilidade. |
| D · WineVDM | Baixo para um teste inicial, esforço de correção variável. | Dependências de vídeo, inicialização de QuickTime, requisitos de drivers e caminhos. Os arquivos de instalação não foram testados. | Usa os binários e dados brasileiros. Depende de emulação de CPU e tradução de APIs em execução, embora dispense VM completa. |

## Escolha

Começar pelo levantamento NE e por um lift pequeno de BIGJOBNG.EXE. A opção NG reduz dependências de renderização estáticas sem trocar a mídia. Reimplementar seletivamente o leitor de recursos, o backend de áudio e o caminho de QuickTime pode ser apropriado mesmo se a lógica principal vier de lift. Reavaliar uma reimplementação ampla somente quando o fluxo script → dispatch → recursos estiver mapeado. A diferença entre controlar os recursos e preservar as atividades jogáveis é o critério de escolha.


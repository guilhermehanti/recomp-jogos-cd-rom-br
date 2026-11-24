# Plano de implementação

## Atualização de escopo: fechamento antes da demo integrada

A instrução atual é concluir os módulos restantes antes de retomar prototipagem. Não entregar uma atividade isolada como demo completa. Os relatórios25–30 documentam os fechamentos de persistência, recompensas, APIs e subsistemas de mídia; o README distingue provas estáticas de comportamento ainda não validado em execução.

A aceitação da próxima demo exige um único fluxo: abertura e perfil -> escolha de tema -> oficina -> todas as atividades acessíveis -> regras e resultados -> áudio/vídeo originais -> retorno à oficina; salvar, fechar e recuperar o progresso. Incluir também painel, jukebox e controles comuns. A cortina deve obedecer ao evento original de entrada na cabine. Elementos inacessíveis, recursos substituídos por aproximações e retornos por teclas de diagnóstico impedem classificar a entrega como completa.

Antes de implementar os adapters, usar as correções de28: GDI!53=CreateDC, GDI!439=StretchDIBits e KERNEL!113=constante __AHSHIFT. Para áudio, distinguir taxa declarada, saída real e backend silencioso de29; para vídeo, usar o ciclo de vida e os17 protocolos observados em30. Preservar os dados brasileiros e criar saves de trabalho fora dos originais.

Não há novo pedido de autorização para operações já autorizadas. A pausa atual da prototipagem decorre da prioridade expressa de rastreamento. O plano abaixo é o registro da fase inicial e não redefine o escopo atual.

## Plano inicial preservado

A fase atual termina na análise estática. O workspace contém analysis/ e tools/. Criar src/, runtime/, resources/, tests/ e build/ quando houver código correspondente. Manter a ISO como entrada somente leitura e deixar arquivos originais fora dos artefatos distribuíveis.

1. **Prova executada, inspeção repetível.** A ferramenta própria abre a ISO sem montagem, percorre o diretório e faz parsing de NE, quatro bancos MMFW e todos os clipes. Critério: 52 arquivos, 49 segmentos em cada variante, IDs de importação reproduzíveis, endpoints dos quatro bancos iguais ao EOF e 12.093 referências MOV dentro do mdat correspondente. Não executar o jogo para provar esses fatos.
2. **Próxima prova de recompilação, após autorização.** Escolher uma pequena rotina de busca num índice PIC após rastrear um callsite a partir de MMFW Pictures. Validar estaticamente a fronteira da função, os fixups e seus ponteiros far. Gerar C para essa rotina com PCRecomp e comparar o índice e o intervalo retornado sobre uma cópia da imagem com o inspetor independente. Cobrir primeiro, último e fora da faixa. Se a função depender de toda a inicialização, isolar a leitura BE16/BE32 para o primeiro lift e deixar o callsite completo como próximo marco.
3. **Bring-up NE.** Resolver nomes e assinaturas Win16, validar chains e callbacks, classificar 98 fixups OS, mapear chamadas indiretas e entradas registradas em USER. Integrar memória, I/O, diretório de idioma e salvamento local. Só marcar o bootstrap como aceito ao criar a janela, responder a eventos e ler recursos corretos.
4. **Multimídia.** Mapear a família MMMovieQuickTime e waveOut. Validar uma imagem 0x0055, uma 0x0056, voz e bancos de som. Para vídeo, validar a primeira faixa do banco e o rebase de stco antes de integrar Cinepak e áudio raw. Usar leitores modernos compatíveis quando possível. Não executar instaladores antigos como estratégia padrão.
5. **Fidelidade das atividades.** Mapear primeiro dispatch do MMS, recursos referidos e interação. Comparar cenas, textos, vozes, vídeos, paletas, eventos e saves com uma instalação brasileira autorizada de referência. Instrumentar a renderização antes/depois da paleta quando necessário, dado o tipo de perda de texto já observado em outros títulos Win16.
6. **Entrega no Windows.** Empacotar o host com launcher local e importação a partir da ISO do usuário, com smoke tests e verificação de hashes. Validar início, todas as atividades, música e vídeo, salvamento, reinício e execução sem a imagem montada.

Para reproduzir a prova estática, no diretório do projeto use PowerShell:

~~~powershell
python .\tools\inspect_iso.py 'D:\emulacao pc antigo\BIGJOBCD.ISO' .\analysis\inventory.json
~~~

A primeira rotina traduzida, qualquer build de componentes e qualquer execução do jogo ficam condicionados à autorização solicitada na entrega.


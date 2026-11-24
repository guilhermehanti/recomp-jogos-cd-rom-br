# Vídeo: superfície realmente referenciada

O inventário de127 stubs QuickTime não corresponde a127 funções demonstradamente usadas pelo jogo. As relocações expandidas e as chamadas near do segmento1 referenciam17 stubs, em18 sites far e2 sites near. Os outros110 não têm referência direta nesse inventário; isso não prova ausência de chamada indireta. Nenhum nome de API Apple foi inventado a partir do número BX.

`host-media-contracts.json` preserva as janelas de instruções de cada chamada. Os tamanhos abaixo vêm dos argumentos preparados pelos chamadores. Um `add SP` pode limpar argumentos acumulados de mais de uma chamada e não deve ser usado sozinho para deduzir o protótipo.

| Stub | Dispatcher/BX | Bytes de argumento observados | Operação demonstrada no chamador |
|---|---|---:|---|
|1:068A|QTIM/07|4|33:3299 libera movie+24, depois zera a referência|
|1:06B2|QTIM/0B|0|33:2B87 inicialização global, retorno zero é aceito|
|1:06BC|QTIM/0C|0|33:2BB2 encerramento global|
|1:075C|QTIM/1C|4|33:31F8 consulta valor do movie para estrutura de parada/tempo|
|1:07DE|QTIM/29|12|33:2A48 fornece out-movie far*, handle de arquivo WORD, offset moov DWORD e flag WORD0|
|1:081A|QTIM/2F|12|33:30A3 fornece movie e DWORDs0/00010000 durante início|
|1:082E|QTIM/31|6|33:308A fornece movie e WORD1;33:3280 fornece movie e WORD0|
|1:0856|QTIM/35|4|33:3794 atualiza imagem do movie durante WM_PAINT|
|1:086A|QTIM/37|4|33:2EE3 libera controller+28|
|1:0874|QTIM/38|14|33:2E8C cria controller com DWORD 0, ponteiro far para o RECT, DWORD 9 e HWND16. ADD SP,0Eh em 33:2E91|
|1:087E|QTIM/39|0|1:0206 inicialização no primeiro uso de QTIM|
|1:0888|QTIM/3A|0|1:0490 encerramento no último uso de QTIM|
|1:0A7A|CMgr/01|10|33:315C ação8/DWORD00010000;33:31A5 ação8/0;33:321D ação12/ponteiro de estrutura|
|1:0AB2|CMgr/05|8|33:36BB consulta flags do controller; bit40 participa do teste de término|
|1:0B06|CMgr/0B|14|33:374F encaminha mensagem, WORDs e DWORD recebidos pela janela ao controller|
|1:0B76|CMgr/13|14|33:306E associa controller, movie, janela WORD e DWORD0|
|1:0B84|CMgr/14|6|33:2EB1 fornece controller recém-criado e WORD0|

As operações descritas são o efeito observável no chamador, sem atribuir identidade SDK aos seletores. O estado owner+24 é movie e+28 controller; a janela fica em+4 e o comando de término em+20. As geometrias vêm do wrapper4:1710 e não identificam o filme. O banco, os58 moov e a seleção por atividade estão em14/27.

## Término, interrupção e restauração

O comando disparado no término natural vem de **+20**. Em 33:36D5, o teste +1E exige reprodução ativa. A chamada 36EA consulta 3695, e o ramo 36F2 encerra a rotina se AX for zero. Com AX verdadeiro, 36F9 restaura o áudio por 3350, 3702 zera +1E e 3715 executa o comando opcional em +20 via 32:009A. Limpar +1E antes de despachar impede uma segunda conclusão pelo mesmo caminho. O registrador 33:2F11 grava o ponteiro em +20 na instrução 2F54, confirmando o mesmo campo pelo lado do produtor.

A parada explícita por 33:3172 também limpa +1E e restaura o dispositivo por 3350, mas esse corpo não despacha o comando em +20. A restauração passa por 39:008E em 33:337D. Assim, a camada de compatibilidade precisa distinguir fim detectado pelo controlador de parada solicitada pelo próprio jogo.

33:3695 consulta o controller por1:0AB2. Sem controller retorna verdadeiro; com controller e bit40 ausente também retorna verdadeiro. Com bit40 presente retorna falso. O consumidor33:3726 encaminha os eventos por1:0B06 e chama essa consulta; não se deve substituir esse estado por um tempo fixo estimado.

33:3172 envia ação8 com0 quando movie/controller existem e flag+1E está ativa; limpa+1E e chama33:3350.33:324B usa esse caminho antes de desativar e liberar movie, limpar global11B0 e as referências. A destruição33:2EBE também chama324B antes de liberar controller e DestroyWindow. A ordem evita callback em objetos já destruídos.

33:31C1 prepara uma estrutura e envia ação12 após consultar o movie. Os campos exatos dessa estrutura permanecem preservados na listagem; não chamá-la arbitrariamente de TimeRecord moderno sem confirmar o layout usado nesta versão. O callback registrado em33:2F11 liga o término ao comando proprietário: cabine em12/27; recompensas em14; jukebox em21. A volta à oficina continua sendo uma ação de cena, não uma consequência automática de fechar o decoder.

## Fronteira para implementação nativa

Os módulos originais estão dentro de `QTW/QTINSTAL.EXE` na ISO. A análise em memória do formato SZDD confirmou QTIMCMGR em 0x048400 (14.544 bytes), QTIM em 0x06DB00 (429.424 bytes) e QTNOTIFY em 0x0FA600 (4.176 bytes), todos NE. QTIM e QTIMCMGR exportam `_ENTRYPOINT` pelo ordinal 2. QTIM exporta `_CLEANUPTASK` pelo 39, e QTIMCMGR exporta `_CMGRTERMINATE` pelo 3 e `_CMGRINITIALIZE` pelo 4. `python tools/inspect_qt_payload.py` reproduz a verificação e imprime os hashes, com leitura da ISO e descompressão em memória.

Em 33:31C2–321D, a ação CMgr 12 recebe um ponteiro far para um bloco de 16 bytes em SS:BP−0x10. Os DWORDs +0 e +4 são zerados em 31E8 e 31EC. O DWORD +8 recebe DX:AX da consulta QTIM/1C em 3200–3203. O DWORD +12 é zerado em 3206. Esta é a disposição gravada pelo chamador. A interpretação desses valores deve ser conferida no corpo do seletor da DLL.

Pode-se substituir o conjunto provedor/player do segmento33 por uma implementação moderna que leia o banco original por ordinal, decodifique cvid/raw e reproduza os contratos de iniciar, parar, pausar, término e restauração do áudio. Essa é uma reimplementação de subsistema. Preservar o corpo original traduzido exigiria reproduzir os17 protocolos acima, suas estruturas e retornos. Nenhuma dessas alternativas está implementada neste documento.

O conjunto de stubs diretamente referenciados está enumerado e suas entradas estão preservadas. A equivalência do decoder, timestamps, estruturas QuickTime e eventos de término exige implementação e testes; a contagem de17 não significa que a reprodução já esteja pronta.

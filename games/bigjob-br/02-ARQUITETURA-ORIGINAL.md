# Arquitetura original

A cadeia de inicialização tem um autorun PE32 e instaladores separados. A lógica do jogo está nos dois executáveis NE para Windows (indicador de sistema 2). Ambos iniciam em 1:0000, com DGROUP e SS no segmento 49 e SP codificado como zero no cabeçalho. Cada um tem 49 segmentos, 40 de código e nove de dados. A edição NG é a primeira candidata por dispensar o import estático de WinG. A igualdade de comportamento entre as variantes permanece por testar. O executável regular contém a mensagem de que usa WinG.

| Binário | Fixups internos | Por ordinal | Por nome | Tipo OS |
|---|---:|---:|---:|---:|
| BIGJOB.EXE | 12.990 | 408 | 0 | 98 |
| BIGJOBNG.EXE | 12.987 | 402 | 0 | 98 |

São registros de fixup, não o número final de endereços corrigidos. Um registro pode encadear vários locais dentro do segmento. O tipo OS requer interpretação específica. A concentração de segmentos e a existência de referências internas não permitem supor um modelo de memória plana. Mapear ponteiros far/near, DS/SS, relocação de seletores, callbacks e chamadas indiretas antes da geração integral. Os três pontos exportáveis do NG são 33:3722, 38:3178 e 38:33BC em hexadecimal no offset, com índices de segmento em decimal. A tabela NE inclui 11 cursores, um ícone, quatro blocos de strings, um acelerador, 11 grupos de cursores e um grupo de ícones. O JSON registra flags, offsets brutos, alocação, pontos de entrada e tipos de fixup.

## Superfície Win16

- KERNEL inclui gerenciamento de memória global, módulos e procedimentos. Ordinais 15–21 incluem alocação e travamento de memória, 50 resolve procedimentos e 51/52 tratam instâncias de procedure. A tabela comprova que existem imports capazes de carregar/resolver dependências dinamicamente. Ainda falta reconstruir os alvos em cada callsite.
- USER inclui CreateWindow (41), ShowWindow (42), RegisterClass (57), entre outros. GDI atende renderização, objetos, DIB e paletas. Mapear todos os ordinais com os arquivos .spec correspondentes, além dos tamanhos de argumentos e dos callbacks. O inventário contém os ordinais exatos para ambos os executáveis.
- MMSYSTEM 404–408 e 411 são waveOutOpen, Close, PrepareHeader, UnprepareHeader, Write e Reset. 607 é timeGetTime. Esses imports são diferentes de MCI: nenhum mciSendCommand/mciSendString foi identificado entre os imports estáticos do jogo. [Código MMSYSTEM do WineVDM](https://github.com/otya128/winevdm/blob/master/mmsystem/mmsystem.c)
- WIN87EM ordinal 1 faz parte da fronteira x87. Não presumir que os 98 fixups OS são todos dessa família sem cruzar instruções e os tipos de record. WING importa os ordinais 1001, 1003, 1006 e 1010 apenas na variante regular.
- QuickTime é uma dependência de comportamento comprovada pela classe MMMovieQuickTime, pela mensagem localizada de inicialização, pelo leia-me e pelo instalador QTW na ISO. Não consta na tabela de módulos. As DLLs exatas e as chamadas efetivas a localizar no callgraph ficam pendentes. O pacote também contém drivers e componentes WinG de arquiteturas mistas. Presença na mídia não comprova uso obrigatório. DVA.386 contém cabeçalho LE, não é um executável Win32 comum.

## Motor e caminhos

Há código de motor ligado ao próprio NE, com strings de leitura de MMFW Pictures/Sounds e rotinas de vídeo. O executável contém a lista BIGJOB.MMS, BIGJOBCD.PIC/BIGJOBHD.PIC, BIGJOB.SND/BIGVOX.SND e BIGJOB.MOV. BIGJOB.DAT aparece em uma rotina distinta e começa com MM, evidência insuficiente para classificá-lo como um contêiner MMFW padrão. O MMS é binário e contém identificadores e nomes de mídia legíveis. O material comprova uma arquitetura híbrida. Não permite quantificar o comportamento no código versus no script, nem afirmar semântica de bytecode. A próxima análise deve localizar quem abre o MMS e seguir a leitura até o primeiro dispatch de comandos.

Há referências ao rótulo BIGJOBCD, a BIGJOB.INI/SAV e ao diretório Windows em mensagens de erro. O d:\\setup do leia-me é exemplo de instalação. Não foi estabelecida uma letra fixa para o motor. Em um port, reproduzir o lookup de recursos a partir de uma raiz local escolhida, mantendo armazenamento do usuário separado e sem escrever na ISO.



# Persistencia: cartoes, BIGJOB.SAV e BIGJOB.INI

Fechamento por analise estatica de `build/BIGJOBNG.EXE`, listagens anotadas
`seg06.asm`, `seg25.asm`, `seg39.asm` e cadeias expandidas em
`analysis/disassembly/references.json`. Enderecos: segmento decimal, offset
hexadecimal. Nenhum executavel original foi iniciado; nenhuma compilacao ou
dependencia do original foi usada. Complementa `22-TC-PERFIL.md`.

## Formato completo de SAV

Nao e dump de objeto C++. Nao inclui ponteiros, indice do cartao selecionado,
flag dirty, cena corrente ou posicao de veiculo. O escritor `6:0DB7` reconstroi
uma lista temporaria e reescreve o arquivo inteiro. Tamanho canonico: `8+77*N`.

| Offset arquivo | Tamanho | Conteudo |
|---|---:|---|
| 0 | 2 | marcador de ordem `II` (nativo); leitor tambem troca WORDs se nao for II |
| 2 | 2 | WORD 0xF0AD |
| 4 | 2 | WORD versao 10 |
| 6 | 2 | contagem assinada N, exige N >= 0 |
| 8+77*i | 2 | ferramenta/avatar do cartao, inteiro assinado 0..12 |
| 10+77*i | 12 | nome, bytes sem conversao de code page |
| 22+77*i | 36 | flags de progresso/desbloqueio |
| 58+77*i | 18 | segundo vetor de progresso |
| 76+77*i | 9 | contadores saturados |

Os offsets de registro relativos sao `0,2,0E,32,44`; soma `4D`/77.
`6:0F01-0FB0` le os cinco campos para a lista temporaria; `6:128C-136A`
escreve os mesmos campos. `39:10EE` e `39:152C` leem/escrevem WORDs com
troca de bytes opcional; os quatro blocos de bytes usam os slots virtuais de
transferencia sem troca. `39:10B0` consome o marcador e define swap=0 somente
para 4949; qualquer outro marcador define swap=1. O leitor original portanto
nao valida literalmente `MM`, apenas a assinatura subsequente. `39:14F6`
escreve 4949 quando swap=0, 4D4D quando swap!=0.

O leitor independente `tools/read_save.py` aceita somente II/MM, valida
assinatura, versao, contagem, ferramenta e tamanho exato; preserva todos os
bytes dos vetores e do nome. Sua politica de seguranca limita N a 50 (capacidade
da alocacao temporaria nativa: 0F0A=3850=50*77). A interface TC impede criar
quando contagem >= 31h/49. Estes sao limites diferentes: o leitor nativo nao
aplica limite superior a contagem do arquivo, embora aloque somente 50 registros.
O leitor independente nao reproduz esse estouro. Tambem rejeita bytes extras,
que o original nao verifica. Nao escreve arquivos nem modifica saves existentes.

## Objeto de jogador e defaults

Gerenciador `6:0060`, global far DS:0432, ocupa 53h/83 bytes. Nao e uma lista
residente: guarda somente o jogador corrente e consulta disco para enumerar.

| Offset objeto | Tamanho | Uso comprovado |
|---|---:|---|
| +00 | 2 | contagem de cartoes |
| +02 | 2 | indice corrente; igual a contagem representa novo cartao |
| +04 | 2 | ferramenta/avatar |
| +06 | 12 | nome |
| +12 | 36 | primeiro vetor |
| +36 | 18 | segundo vetor |
| +48 | 9 | contadores |
| +51 | 2 | dirty; nao serializado |

Construtor zera contagem, ferramenta, nome e vetores; primeiro vetor recebe
1 em offsets de objeto 17,20,28,2C,2F,34 (indices 5,14,22,26,29,34).
Depois chama `094E` para enumerar ferramentas, coloca indice=contagem e dirty=0.
Se argumento de construcao !=0, tenta carregar cartao zero por `0B59`.
`6:0039` cria esse gerente sob demanda com argumento1; TC `25:007D-00CD`
tambem cria quando a global esta nula.

`0191` define indice e dirty=1; `01AC` define ferramenta e dirty=1; `01D9`
copia ate12 bytes do nome, zera explicitamente +11 (ultimo byte), dirty=1.
Logo nomes editados pelo original tem no maximo11 bytes uteis. Nome iniciado
por `!` ativa DS:008A; nao e um campo separado do save. `0258` retorna +06.
`0303`/`0322` colocam1/0 no segundo vetor por indice e marcam dirty;
`0341` consulta esse vetor; `03CC` consulta o primeiro.
`026C` atualiza flag do primeiro vetor e encadeia comandos de recompensa;
o prototipo deve preservar o fluxo de premio em vez de interpretar todos os
36 bytes como booleanos genericos de fase terminada.

Contadores `03E2`/`0417` usam tradutor `05CB`; IDs de atividade sao
4,6,12,15,18,19,23,31,32, mapeados respectivamente a indices
8,0,5,2,7,4,6,3,1 (tabela de saltos 0627/0639). Incremento para em200
(comparacao unsigned C8h em0402); ID desconhecido nao incrementa e consulta0.
Esse incremento nao marca dirty explicitamente; depende das demais alteracoes.
`0357` testa tres trios do primeiro vetor: seletor0 offsets21/32/18,
seletor1 1E/25/31, seletor2 16/24/29; `0444` conta quantos trios completos.
`0496` escreve os trios com ordem seletor0 ->16/24/29, 1 ->1E/25/31,
2 ->21/32/18. A inversao 0/2 entre teste e escrita e real no codigo.

## Todas as operacoes de disco do gerente

| Rotina | Operacao e resultado |
|---|---|
| 6:0762 | enumera nomes: valida header, para cada cartao descarta ferramenta e vetores, copia nome12 para destino+12*i; retorna contagem ou0 |
| 6:094E | enumera ferramentas: valida header e ferramenta0..12, copia WORD para destino+2*i, descarta nome/vetores; retorna contagem ou0 |
| 6:0B59 | carrega indice solicitado, zera indice corrente antes; le sucessivamente registros ate indice inclusive nos mesmos campos do objeto; no sucesso guarda indice e retorna1; arquivo vazio retorna1 sem trocar defaults |
| 6:0DB7(arg<0) | carrega lista antiga, substitui registro no indice corrente pelos campos do objeto; se indice==contagem acrescenta novo e incrementa contagem escrita |
| 6:0DB7(arg>=0) | exclui indice, desloca registros seguintes, decrementa contagem e regrava lista; nao substitui registro por objeto corrente |
| 6:141C | wrapper de exclusao, passa indice a0DB7 |
| 6:13FC | retorna1; nao e rotina de escrita |

`0B59` nao faz seek direto: reler todos os registros anteriores evita calcular
offset em disco, mas sobrescreve o objeto a cada iteracao. Leitura parcial pode
deixar estado parcialmente alterado antes do handler. Enumeradores so validam
campos que consomem semanticamente:0762 nao valida ferramenta;094E valida.
`0DB7` valida ferramenta de cada registro temporario antes da escrita. Antes
de gravar nome, se o segundo byte do nome (registro+3) e `?`, troca por espaco
(`6:129F-12B4`); preservar nomes do leitor nao equivale a emular essa mutacao.
No sucesso da regravacao dirty=0 em13BD. Nao ha checksum, compressao, criptografia,
backup, arquivo temporario, rename atomico ou persistencia de indice corrente.

## Triggers de carregar e salvar

| Origem | Acao |
|---|---|
| construtor6:011E | enumera ferramentas para obter contagem |
| construtor6:014A | carrega primeiro cartao se solicitado |
| TC montagem25:02FC/0321 | enumera ferramentas e nomes para exibir lista |
| TC comando1,25:0D92/0DB1 | define indice, carrega cartao e abre editor |
| TC comando3,25:1143 | muda ferramenta para novo cartao; indice corrente preserva estado de novo cadastro |
| editor25:305B | define nome; comando6 fecha editor e permite continuar |
| TC comando8 ->6:141C | exclusao imediata e regravacao integral |
| TC25:14D0/14F5 | enumera novamente ferramentas/nomes apos exclusao |
| TC comando7,25:153E | define indice corrente; saida do editor/retomada, conforme22-TC-PERFIL |
| 5:0383 | comando de teclado/menu do handler5:0305 chama0DB7(-1) se gerente existe |
| 2:0F4C-0FA4 | encerramento: se gerente existe e dirty!=0 chama0DB7(-1), destroi gerente, zera global |

Nao ha chamada de0DB7 no seletor de confirmacao de nome: a confirmacao marca
dirty e o commit depende do trigger de salvamento/encerramento. Cadeias de
referencia externas de0DB7: somente2:0F79 e5:0384; chamada interna adicional
6:142A e wrapper de exclusao. Assim nao se deve inventar autosave por quadro,
por som ou por cada entrada do editor. Referencias de0B59: construtor e TC.

## Caminho, APIs e falhas

`2:013A` chama KERNEL ordinal134 com buffer90h para diretorio Windows e copia
o resultado para app+3DA. `6:064B` recupera esse diretorio, `0690` constroi
nome `BIGJOB.SAV`, `06EE` nome `BIGJOB.INI`; `29:2F58` combina diretorio e nome
por formatacao para abrir SAV. INI usa nome relativo na API de perfil Windows,
como detalhado abaixo; nao usa a concatenacao do stream SAV. Inventario da ISO e entrada somente
leitura; ausencia de SAV na distribuicao nao significa ausencia de persistencia.

Stream `39:1972/1A94` instala vtable2456, mantem handle+10, aberto+12,
modo leitura+14 e erro+16. Modo0 abre por KERNEL!85 com acesso0; modo1 cria
por KERNEL!83; modo2 tenta acesso2 e cria se handle=-1. Slot+4 ->1D2B consulta
erro; +8 ->1D3D le usando KERNEL!349 e exige bytes transferidos==solicitados;
+14 ->1DAB escreve usando KERNEL!350 e exige a mesma igualdade. Seek1C53 usa
KERNEL!84; close1BEC usa KERNEL!81. Erros ficam sticky em+16=FFFF.
Ausencia/falha de abertura do SAV nos enumeradores/leitor retorna0 silenciosamente,
deixando defaults; regravacao pode criar o arquivo ausente. Erro de transferencia
ou close encaminha ao handler. Diretorio sem permissao pode impedir criar.

`6:15BC` recebe codigo0 leitura,1 escrita,2 versao antiga,3 corrupcao.
Mostra MessageBox (USER!1), titulo `Big Job Critical Error`. Mensagens literais
em1435,147A,14BF,1505 dizem perda de informacoes; codigo2 desativa exclusao e
saida critica nesse handler. Demais codigos tentam apagar SAV por29:2CAB; se
falha, mostram instrucao para apagar manualmente do diretorio Windows (1565).
Se estado de app+3D8 e1,2 ou3, chama slot virtual de encerramento+8.
Retornos de operacao continuam definidos pelos chamadores; o texto "exiting"
nao prova terminacao incondicional em qualquer estado. Arquivo truncado e
detectado por contagem exata de bytes no stream. Contagem negativa, assinatura
errada e ferramenta fora0..12 sao corrupcao; versao!=10 tem codigo2 proprio.

## INI completo deste escopo

Objeto de preferencias `39:06A2` armazena caminho completo256 bytes e nome13.
Global DS:1326 e ponteiro relocacionado `39:0696` = `Preferences`: a secao e
`[Preferences]`, nao o nome do arquivo. Leitura `39:08E0` usa KERNEL!128 com
default vazio, nome em objeto+100 (`BIGJOB.INI`), secao Preferences e chave recebida.
O diretorio guardado em objeto+0 nao e passado a essa API; nao confundir a
semantica de resolucao de nome da API de perfil com abertura explicita do SAV.
Retorna
falso se ausente/vazia ou se tamanho retornado igual ao limite. `39:07B3`
converte decimal opcionalmente negativo em WORD, sem espacos e sem outros
caracteres; ausente/invalido conserva default fornecido. Escrita `39:0872`
formata `%d`, passa a0934 ->KERNEL!129; esse wrapper retorna1 sem propagar
o resultado da API. Nao ha prova de sucesso no retorno de6:1805/188B.

| Gerente | Chave | Default | API |
|---|---|---:|---|
| 6:16E7 | DisablePrint | 0 | le inteiro |
| 6:1776 | DisableSound | 0 | le inteiro |
| 6:1805 | DisableSound | argumento | escreve decimal |
| 6:188B | DisablePrint | argumento | escreve decimal |

Print e consultado no escopo de impressao; Sound em2:054D e22:06F9.
22:0639/064A escreve ambas as preferencias. Nenhum nome de jogador ou vetor
de progresso passa pelas APIs INI. Outros consumidores genericos de39:06A2
podem existir; esta tabela fecha as quatro funcoes do gerente6 que usam INI.

## Verificacao e limites

`python -m unittest discover -s tests -p test_save.py` cobre fixtures sinteticas
II/MM, todos os campos e limites, vazio, cada truncamento, assinatura, versao,
contagem negativa/excessiva, ferramenta invalida e bytes extras. Nao existe
SAV real conhecido para teste de compatibilidade; conclusoes sao estaticas.
Semantica editorial de cada um dos36/18 indices deve acompanhar as atividades,
nao ser inventada por este modulo. O formato, transferencias, gatilhos e falhas
acima sao suficientes para importacao segura e implementacao de persistencia
propria, preservando bytes ainda sem rotulo.

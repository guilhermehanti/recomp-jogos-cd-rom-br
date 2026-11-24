# TC, cartoes de jogador e telas LG/JY

Analise exclusivamente estatica; enderecos NE em segmento decimal e offset hexadecimal. Nenhum original foi executado e nenhuma compilacao foi realizada.

## Identidade de TC e fluxo de entrada

A cena 10 nao e um minijogo de ferramentas. O construtor 25:0000 chama a base de cena 5:0059, instala a vtable 25:3340 e inicializa dois recursos de som: 439D em +130 e 439C em +13A (25:003F-0061), nomes `TCcabinetDoorOpen` e `TCcabinetDoorClose` em BIGVOX.SND. A string nativa em 25:0B9E e `TimeClockScene::ExecuteCommand`; em 0BC6 aparece `User clicked on time card`, em 0BE4 `Name`, e em 0C0B a mensagem sobre o numero maximo de timecards. Isso liga TC a selecao/criacao de cartoes de jogador, alem da familia visual de ferramentas.

O construtor cria o gerenciador 6:0060 se a global far 49:0432 estiver nula (25:007D-00CD). O conteudo 25:02DD monta fundo, ferramentas, cartoes e animacoes. A rotina 25:0C46 e um despachante de comandos 0..12; os valores sao seletores internos, nao numeros de cena.

| Comando | Destino da tabela 25:15DF | Efeito comprovado |
|---:|---|---|
| 0 | 15DA | Sem acao |
| 1 | 0C61 | Seleciona cartao por indice relativo +14A + argumento; chama 6:0191 e 6:0B59; abre editor 25:25CD |
| 2 | 0F22 | Exige +152=0 e +144=0; toca abertura; escolhe label 7; mostra painel/cabinet e define +150=1 |
| 3 | 102A | Exige +152=0, +144=0 e contagem +186<49; fecha cabinet, escolhe label 9 e abre editor de novo cartao |
| 4 | 1287 | Decrementa inicio da lista +14A por 8, limitado a zero; redesenha |
| 5 | 12BF | Avanca lista por 8 se +14A+10 ainda for menor que +186 |
| 6 | 145E | Marca +144=1, escolhe label 13, redesenha e limpa ponteiro de editor +12C |
| 7 | 1515 | Seleciona indice corrente da gerencia; escolhe label 1; se nenhum cartao existe, encadeia comando 2 |
| 8 | 149A | Chama 6:141C para indice +188+14A; recarrega tabelas/contagem e redesenha |
| 9 | 12EF | Se +152=0, +144!=0 e +150=0, instala callback comando 11 no MMS +124 e inicia a animacao; caso sem cartao, escolhe label 17 |
| 10 | 1586 | Se +144=0, escolhe label 7 ou 2 segundo cabinet +150 e existencia de cartao |
| 11 | 1408 | Cria e executa transicao para cena 3, contexto 9, auxiliar FFFF |
| 12 | 15CE | Define +446=1, liberando comando condicionado do editor |

O limite literal para criacao e `+186 < 0x31` (49), nao foi deduzido pelo tamanho de vetor. A lista paginada mantem indice inicial +14A e selecao +188. A remocao e identificada por 6:141C no ramo 8, mas a persistencia em disco deve acompanhar a implementacao dessa gerencia.

## Teclado e editor de nome

O +18 da cena e 25:0294: se +12C nao nulo, encaminha o codigo recebido ao slot +18 do editor; senao delega a 5:0305. Assim o teclado pertence ao editor ativo, e nao a um controle de jogo baseado nas ferramentas.

O editor e construido em 25:25CD; seus comandos 25:2503 guardam owner +26 e seletor +2A. O executor 25:2559 chama 25:3023, cujo despachante 0..3 esta em 30C6. O seletor 2 chega a 3040, extrai o texto por +72 ->+4 ->+14 e chama 6:01D9 com o gerenciador global; depois encaminha comando 6 ao owner. Os seletores 1 e 3 encaminham comandos 8 e 7. Isso demonstra caminho de confirmacao do nome e de saidas do editor, sem inventar quais teclas fisicas confirmam/cancelam. 25:3222 executa seu comando filho somente se owner+446!=0; o callback MMS que define essa flag deve ser preservado.

## Audio de TC: owner, labels e callbacks concretos

25:08E0 carrega MMS 0119 e guarda em +120; 25:1656 usa exatamente esse ponteiro para construir salto ao label recebido por 30:0FBD e executa por 32:009A. Logo os labels abaixo pertencem ao roteiro de dialogo de TC, nao ao outro MMS 0120.

| Origem | Label de 0119 |
|---|---:|
| Montagem 25:09D7-0A09, gerencia sem cartao / com cartao | 7 / 2 |
| Comando 1, 25:0C7F-0C87 | 1 |
| Comando 2, 25:0F67-0F6F | 7 |
| Comando 3, 25:1060-1068 | 9 |
| Comando 6, 25:1468-1470 | 13 |
| Comando 7, 25:1546-154E | 1 |
| Comando 9 rejeitado por falta de cartao, 25:13F7-13FF | 17 |
| Comando 10, 25:1586-15C6 | 7 ou 2 |

O script 0119, bloco 0xADAD3, possui os sons 42CA `CdMemberPickCard`, 42CC `RmFirstTimeClickJoin`, 42CE `RmMakeCardPickTool`, 42C8 `CdGoodHeresCard`, 42CD `RmIfWantTypeName`, 42C9 `CdHaveFun`, 42D2 `RmSeeYouLater`, 42C7 `CdForgotTimeCard`, 42D1 `RmPickCardComeIn`, 42CB `RmClickHereWhen`, 42CF `RmOrClickHereTryAgain`. Os offsets completos de cada disparo estao no catalogo JSON. Slots MMS 0 e 1 sao registrados em 25:0976 e 09CF: fabricas 25:2439 com seletores 9 e 12. Portanto 0xB381F chama comando TC9, e 0xB5187 chama TC12; nao sao enderecos nativos codificados no script.

MMS 0120 e outro objeto, em +124 (25:086E-0887). TC9 instala comando TC11 como callback final por 25:1362 ->30:11E5 e inicia por 25:1375 ->30:0F50; TC11 leva a selecao de cenarios/cena3. MMS 0136 e carregado em 25:0A3E; seus disparos 0x56F4F e 0x573F3 usam 439F `TCtelephone`. MMS 0137 e carregado em 25:0B0C e nao contem opcode de som. O som direto em 25:0B91 e low-word 42AA (`TCloop01d`), acompanhado do parametro alto 4E20; nao e indice de 32 bits. O catalogo mantem tambem os sites nativos e seus argumentos.

## LG e JY: introducao e passagem ao cadastro

| Cena | Evidencia de conteudo | Saidas comprovadas |
|---:|---|---|
| 9 / LG | 20:0AEE carrega HD 020D `LGbackground`; MMS 0118 em 0B23 e 0139 em 0BC1 | Callback final de 0139 instalado em 0C78: cena11/contexto9; area de tela inteira criada em 0D7E: cena11/contexto0 |
| 11 / JY | 20:11CE carrega HD 020B `JYbackground` em 1210 | Area de tela inteira criada em 1336: cena10/contexto9; +28/20:1359 constroi a mesma transicao em 137B-138F |

Os slots +40 de ambas as classes 20:10BA e 20:13AB resolvem para 5:1374 (relocacoes 20:10FA e 13EB), o auxiliar comum de roteiro por seletor. Na cena11 ele e chamado explicitamente em 20:123F com seletor 11; 5:13BD-13CA consulta a tabela WORD DS:03D4[seletor] e carrega o MMS correspondente. Nao confundir esse slot com 20:0DA1: esta outra construcao de conteudo pertence a vtable 2:1862, slot +0C em 2:186E. Ela carrega MMS 01CA em 20:0EDA, registra callback final em 0F91 para cena9/contexto0 e instala area de tela inteira em 1097 para cena11/contexto0.

O script raiz 0118/bloco0xA8AE3 contem JYdoorThud, JYtheme e dialogos de apresentacao; seu opcode de comando em 0xAD0E3 usa slot0. O carregamento em 20:0B23 associa esse roteiro a cena9. A atribuicao editorial de cada fala a um quadro depende do label e das condicoes do script, nao apenas do nome. A sequencia nativa demonstrada e LG9 -> JY11 -> TC10 -> selecao de cenarios3. Ela nao concede premio de atividade nem usa 5:14FF.

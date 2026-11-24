# Grades 8010, 8012 e 8016: caminhos nativos

Análise estática de `build/BIGJOBNG.EXE`, desmontagem LLVM em modo 16 bits e fixups NE de `mapa-jogo-v3.json`. Nenhum executável original foi iniciado, e nenhum binário foi compilado. Endereços usam segmento decimal e offset hexadecimal. As associações de cenas abaixo estão demonstradas por tabelas virtuais e chamadas, não apenas pelo segmento que contém a grade. Os nomes de recursos são os do índice brasileiro.

## Entradas, classes e carregadores

| Cena | Classe / entrada de construção | Entrada virtual de montagem | Caminho até a grade | Identidade de imagem demonstrável |
|---|---|---|---|---|
| 1 | 23:0000 instala 23:37E1 | slot +0C = 23:030F | 030F chama 271F para copiar a configuração global 02A0 a C8E; em 09EF testa C90 e em 09FC chama 3053. Ao mudar a configuração, 275F marca C90=1. | 23:0363 passa PIC 4C12 a 37:011E: CD[0C12] `SBbackground`. |
| 7 | 6:1AC2 instala 6:3786 | slot +0C = 6:1C7E | em 1CF7 chama diretamente 6:306F, antes de construir os desenhos e controladores do tabuleiro. | 6:1CBA passa PIC 411F a 37:011E: CD[011F] `CGbackground`. |
| 21 | 17:0000 instala 17:3E04 | slot +0C = 17:0127 | 0127 cria por 17BE o tabuleiro de 0x2EA bytes, guarda em +98 e anexa em 01DF. O construtor 17BE chama 197D com argumento 0 em 189C. | 17:0162 passa PIC 4A33 a 37:011E: CD[0A33] `FMbackground`. |

As entradas +4 de todas essas tabelas são 35:3BD3, que consulta +0C. Isso fecha a cadeia desde a seleção da cena pelo segmento 2 até o consumidor das tabelas MMS.

## Cena 1: edição do tabuleiro SB

23:3053 usa C8E para escolher **0 → 00DB, 1 → 00DC, 2 → 00DA**, e não a ordem numérica dos IDs. 30B5 chama 1:313B e usa o resto da divisão pelo número de entradas. Depois lê dez linhas DD de dez caracteres; as linhas auxiliares DE não são lidas por esse caminho. Portanto as vinte strings alternadas de cada tabela não são vinte linhas jogáveis.

O estado de 100 células está em `+2A6 + linha*20 + coluna*2`. O desenho de cada célula é escolhido por 1BFD e guardado em +3AC; outro valor derivado por 16DD fica em +474. Os objetos visuais correspondentes são ponteiros em `+116 + linha*40 + coluna*4`. O conversor 31E1 retorna A-U → 1-21 e V-Y → 23-26, deixando o código 22 sem letra nessa sequência.

O comando local construído em 23:382E armazena seletor/argumentos, e o executor encaminha para 11EF. Na montagem, 03F4-0404 constrói seletores 4-9 para as seis peças da primeira paleta; 11EF transforma 4-9 em índice 0-5 e consulta tabelas dependentes de C8E para definir `+64`, a peça selecionada. Seletores 10-18 usam outra paleta de nove entradas. Esses dois caminhos chegam a 1944 com objeto e argumentos da peça.

Eventos 0100-01AA codificam a célula: linha `(evento & F0)>>4`, coluna `evento & 0F`. Em 12CF-12DD o código exige que +64 seja maior que zero e menor que 28; 12F9 grava a seleção em +2A6. Atualiza o visual por 1BFD, recalcula +3AC e +474 e chama 2511. Esta última função atualiza os quatro vizinhos ortogonais, com limites 0 e 9 (251A, 25A3, 262C, 26A3). Não é um teste de vitória: as chamadas observadas são de recálculo de desenho.

Outros seletores concretos:

| Seletor | Efeito nativo |
|---|---|
| 25 (0x19) | 13F2 → 1D63. Interrompe o áudio em +396, solicita 00014013, chama 24:1D2B com +53C e prepara a sequência seguinte. O começo recusa repetir a operação enquanto o objeto +3A8 está nos estados 1, 2 ou 3. |
| 26 (0x1A) | 1403 → 344B. Caminho de impressão, confirmado também pela string `Cannot print. BJPrint is busy.`; não é conclusão do jogo. |
| 27 (0x1B) | 1418 → 3053: seleciona outra grade; em seguida varre todas as 100 células chamando 2511. |
| 19-24 (0x13-0x18) | 1548 marca +6E do objeto +53C; toca um áudio escolhido em DGROUP:0DDC; consulta tabela dependente de C8E em 0DB8 e chama 24:1C1C. |
| 2 / 3 | Operações sobre célula codificada no argumento: 2 grava tipo 2 e recompõe imagem; 3 recalcula o valor em +474. |

A entrada de teclado da cena é o slot +18 em 23:37F9, resolvido para 1110. A tabela 11AE contém 0024, 0025, 0031-0036, 0040: `$` limpa o global 008A; `%` chama 1822; `1` a `6` enviam seletores 19-24; `@` envia 25. A interpretação exata de maiúsculas/modificadores depende da conversão anterior de teclado da aplicação. Em 1822, apenas quando global 02A0=0, são procuradas até trinta células dos tipos 13/14 com +474 positivo; uma é escolhida pelo resto aleatório e passada a 16DD com flag 1. Esse é um auxílio condicionado ao nível, não uma regra universal.

**Cobertura estática:** a seção de fechamento abaixo demonstra controlador, decisões autônomas e callbacks. Os comportamentos não foram testados em execução.

## Cena 7: CG, tabuleiro de portões e controlador de animais

6:306F escolhe a próxima das oito grades por contador DGROUP:0446: usa o valor anterior, incrementa e zera quando chega à contagem. Aqui a escolha não é aleatória. Cada linha F0 tem treze caracteres. Linhas pares usam caracteres 1,3,5,7,9,11 e alimentam a matriz +11E, sete linhas de seis entradas; linhas ímpares usam caracteres 0,2,4,6,8,10,12 e alimentam +CA, seis linhas de sete entradas. As células de seis por seis usam ainda +82. A composição preserva as duas orientações das fronteiras da grade.

O código 9 (E/e) na matriz +CA grava suas coordenadas em +172/+174, em 31E4-31FA. Conversor 2C7C: 1/2/3/4 → 1/3/5/7; E/e →9, X/x →10; hífen e barra vertical →11. Não converter esses números diretamente em cores sem seguir as tabelas visuais de 2D22 e 2D41.

A entrada de teclado é 6:296A, slot +18 da tabela 3786. Tabela 2ACB: espaço chama o seletor 9 de 2B81; 1-4 encaminham os objetos +36E/+372/+376/+37A a 2:34EE; `+` reconstrói a cena pela entrada +10 e +0C após destruir +5C/+60; `L/l` e `W/w` zeram global 008A e chamam slot +4C com 0 ou 1. São entradas existentes no executável; sua exposição na interface deve ser separada das ações regulares de clique.

O executor 6:2B81 despacha dez seletores:

| Seletores | Destino |
|---|---|
| 1-4 | 7:1046 com valor 1-4, guardado em +8E do controlador +17A. |
| 5-8 | 2:35E2 nos quatro objetos +36E, +372, +376, +37A. |
| 9 | 7:0F3A com controlador +17A. |
| 10 | Chama slot +4C da própria cena com argumento 1; a tabela resolve para 6:2AF3. |

6:2AF3, quando o argumento é não nulo, chama 7:0B3C para limpar o controlador, zera o valor do indicador +382 por 5:2D43, carrega a próxima grade por 306F e redesenha por 2EA1. Para os quatro objetos de seleção, chama 2:35E2 quando o campo +4 é 1. Assim o final prepara uma nova rodada, em vez de solicitar diretamente a oficina.

Há um predicado de término verificável em **7:105C**: primeiro exige `+56 >= +54`; depois examina dez ponteiros em +64 e só retorna 1 quando todos estão nulos. Em 7:0C69-0C87, se +58 ainda é zero e o predicado passa, envia seletor 10 à cena via 6:2B81. Portanto término envolve contador e ausência dos dez objetos ativos, não apenas uma comparação de tempo. +54 é o total previsto e +56 é o total já gerado; o predicado não consulta a pontuação.

Áudio: a montagem toca 00004304 em 6:28A6 (BIGVOX[772] `CGloop`, palavra alta 7FFF no empacotamento de chamada). O seletor 9 passa a 7:0F3A, que interrompe +90 e pede 00014303 (`CGhorn`, índice 771), chama 7:04BD para cada um dos dez objetos não nulos e marca +5A=1. Em 6:2C25 há escolha entre quatro entradas de DGROUP:043E por resto aleatório, e incremento do indicador em +382. 6:36FE pede 000141A6 = BIGVOX[422] `TuCowsInFlowers`; o chamador da entrada nas flores é 7:0469 →6:36B9.

**Cobertura estática:** a seção de fechamento abaixo demonstra controlador, decisões autônomas e callbacks. Os comportamentos não foram testados em execução.

## Cena 21: FM, movimento e resgate

O tabuleiro alocado por 17:17BE possui 14 colunas por 10 linhas. O vetor de ponteiros está em `+7E + coluna*40 + linha*4`. A função 117B rejeita coluna <0 ou >=14 e linha <0 ou >=10. As dimensões coincidem com as dez strings MMS de catorze caracteres; a ordem dos índices em memória deve ser preservada.

Em 197D, o campo +6C da cena escolhe uma das metades da lista de 22 grades: não zero usa intervalo 0-10; zero usa 11-21. O argumento zero seleciona por 2:0012 dentro da metade; argumento não zero incrementa DGROUP:0BAA e volta ao início da metade ao exceder o fim. Um byte 0BAC inicializa o contador na primeira entrada. A montagem inicial 189C usa argumento 0; 17:0545, alcançável pelo slot +50 da cena, limpa, reinicializa e carrega a próxima grade com argumento 1 em 0593.

| Símbolo | Fábrica | Resultado observado |
|---|---|---|
| w | 1C37 | Constrói classe por 07FE com argumento tipo 1 e aplica imagem 4A3F. |
| o | 1D13 | Constrói por 26E7; escolhe uma de seis imagens com aleatório 2:0012. |
| f | 1E1B | Constrói por 35D3. |
| e | 1F9C | Constrói por 37B1 e guarda o objeto também em +74. |
| a | 206D | Constrói por 3841. |
| barra vertical | 1ED9, argumento 1 | Objeto com orientação explícita. |
| hífen / sublinhado | 1ED9, argumento 0 | Outra orientação da mesma fábrica. |
| ponto / desconhecido | Nenhuma fábrica | Mantém espaço vazio. |

17:0127 cria comandos 17:0D50 para quatro direções e para parada (0,1,3,2,4), ligando o ponteiro do tabuleiro. O executor 0DA6 lê +2A e chama 11EC. A função 11EC atualiza a direção +2B6; se o ator não está ocupado (consulta virtual +1C) chama a movimentação 122E. A entrada de teclado da cena em 00C1 aceita códigos hexadecimais 0067,0068,0066,0065, que enviam direções 0,1,2,3 e depois 4. O slot +18 de 3E04 aponta a 0653: teclas ASCII `8`,`2`,`4`,`6` fazem o mesmo. Por 11A5, 0 diminui linha, 1 aumenta linha, 2 diminui coluna, 3 aumenta coluna. Direção 4 interrompe a tentativa em 123A-1242.

A movimentação 122E exige ator livre, calcula célula vizinha, verifica limites por 117B, obtém tipo por 34C3 e flags de interação por 350B. Tipo 6 pode chegar ao despacho mesmo sem a segunda flag. A tabela em 1755 tem destinos de tipos 1-9: 1355,156C,139A,16FB,16FB,1527,16FB,14AD,1439. Isso permite distinguir obstáculos comuns, objetos empurráveis e ferramentas sem inferir só pela letra MMS.

No ramo de empurrar 156C, calcula mais uma célula na mesma direção: se sair da grade ou a célula contiver tipo diferente de zero, toca 00014390 = BIGVOX[912] `TuOoomph` e não move. Se livre, move o objeto por seu slot +20, limpa a referência antiga com 3574 e grava na célula adiante. O movimento do ator só segue pela continuação 16FB. O ramo tipo 9 em 1439 só atua quando flag +32 está zero: pede 00014266 = BIGVOX[614] `TuPersonOutSafely`, cria a animação 01B5 por 2199 e marca +26 do ator por 2D48. O tipo 3 em 139A, quando +32 é zero, pede 00014267 = BIGVOX[615] `TuRescuePerson`; quando +32 é não zero cria animação direcional por 22C4. O ramo tipo 8 em 14AD exige +34 zero e marca +28 do ator por 2D5C; o ramo tipo 6 em 1527 exige +34 não zero e chama 2D03 para interação direcional. A associação de +32/+34 com carregamento de pessoa/ferramenta é hipótese forte, e é demonstrada pela identidade entre o ator embutido+0C e os campos do tabuleiro na seção de fechamento.

A animação 2199 carrega MMS 000101B5 e registra no índice 0 um comando 17:0ED5, cujo executor 0F24 chama 2126. Este remove o objeto da posição +78/+7A, chama seu slot +8 e limpa a célula. O final da animação portanto possui callback concreto de atualização do tabuleiro. A animação direcional de 22C4 escolhe 011F/011E/011C/011D para direções 0/1/3/2, instala comando de objeto no índice 0 e comando 2:22FB no índice 1, e dispara a entrada inicial por 30:0F50 e 32:009A. Não é evidência de filme MOV: são animações MMS.

Outros efeitos exatos: 2C16 pede 0001438C = BIGVOX[908] `FMpush01`; 2C63 pede 0001438F = BIGVOX[911] `FMturonWalk`; 2F5B pede 0001400E = BIGVOX[14] `ALcrash01`. Os eventos estão em métodos do ator/objetos, e o áudio não deve ser reproduzido por simples presença da letra na grade.

**Conclusão:** a seção de fechamento abaixo demonstra pessoa carregada, entrega na porta e chamada de sucesso compartilhada, com latch de uma conclusão por rodada.

## Retorno, vídeo e próximos fechamentos

As cenas 7 e 21 derivam de 5:14FF e compartilham slots de controle de atividade. O retorno da oficina deve seguir o comando criado pela classe base; não apareceu instância direta de 2:1AC3 nos três segmentos analisados. O caminho de final de rodada CG é comprovadamente reset interno por 6:2AF3. As chamadas ao segmento 33 em 6/17/23 incluem classes de renderização genérica e destruidores; a presença do segmento 33 não demonstra reprodução de filme. Não foi ligado nenhum descritor 8008 nem objeto 33:2BC7 a um evento dessas três cenas.

Os fechamentos adicionais abaixo completam as decisões locais de controlador e callbacks. A cobertura é estática: nenhuma simulação, reprodução original ou compilação foi utilizada.

## Fechamento adicional: identidade dos campos e callbacks FM/SB

A equivalência dos flags FM é estrutural, não hipotética: o ator está embutido no tabuleiro em **+0C** (17:22CD-22D8 e montagem). Logo tabuleiro `+32` é ator `+26`, e tabuleiro `+34` é ator `+28`. 17:2D48 e 2D5C gravam 1 exatamente nos campos consultados pelos ramos 1439/14AD/1527. 17:2B80 zera ambos ao reinicializar o ator. Assim os nomes operacionais são **pessoa carregada** e **machado adquirido**; não existe sincronização atrasada de duas cópias desses flags.

O resgate é uma sequência em duas etapas. Colidir com tipo 9 sem pessoa marca pessoa carregada e cria 01B5; seu callback remove a pessoa da grade por 2126. Colidir depois com a porta tipo 3 tendo pessoa chama 22C4. A animação direcional instala no índice 0 o comando 17:0E1A sobre a porta: executor 0E69 chama **36E9**, que zera `porta+0E` e troca seu visual pelo retângulo vazio de 34:0000. No índice 1 instala **2:22FB(cena,1)**; executor **2:2351** chama o slot **cena+4C** com argumento 1. A tabela 17:3E04 resolve esse slot para **17:0095**. Esse método exige `cena+A4 != 0`, consome o latch zerando-o e encaminha a **5:1A52(cena,1)**. Este zera cena+6A e dispara a entrada 1 da animação de sucesso em cena+6E por 30:0F50 →32:009A. Portanto há evento terminal de sucesso comprovado: entregar a pessoa à porta, após a animação, aciona a conclusão compartilhada. Não depende de contar todas as células vazias. O latch é rearmado em **17:05AA** pela preparação da próxima grade (slot +50); evita repetir a conclusão na mesma rodada.

Machado: **17:2D03** guarda o objeto-alvo em ator+62, direção em +10, arma +22 e zera movimento +0E e fase +12. A atualização **2D9C** tem prioridade para +22. Nas direções 0..3 usa animações direcionais; na continuação **2F10** incrementa fase, configura intervalo 500 em 11:2295 e, na fase 2, toca `0001400E`, chama **3A01(alvo)**, limpa +62 e configura intervalo 200. Na fase >=3 limpa +22. Isso é interação temporizada, e não derrota causada por tocar um obstáculo. +28 permanece adquirido até o reset 2B80. 2CE0 só declara ator livre quando +0E e +22 são zero; 2CA7 consulta temporizador se há movimento/interação/animação auxiliar. A animação idle com machado em **3158** usa ciclo de quatro fases; o ramo sem machado em **32E0** também usa quatro. Não criar derrota por tempo a partir desses temporizadores de animação.

Em SB, a chamada **24:1D2B** do botão 25 foi fechada: percorre os dez ponteiros de agentes em controlador+46, remove cada um do container +42 por 1138, destrói por 1047(arg3) e zera o ponteiro. Ela é **limpeza**, não teste de sucesso. **24:1C1C** busca uma célula aceita por 23:1EF0 e não ocupada segundo 24:1D9E; recusa quando não há célula ou quando os dez slots estão ocupados; caso contrário constrói um agente por 0E06 e o anexa por 10F0. 1D9E trata coordenadas fora de 0..9 como ocupadas e compara as coordenadas +6/+8 de cada agente ativo. Depois da limpeza, 23:1DF9-1E7B remove cinco desenhos em +36E, zera controlador+6E e inicia o objeto 24:1E0A por 37:1678. O callback **24:1EAB** chama 23:276A para estado inicial (0,0), varre duas orientações por cinco etapas com espera 50 via 37:1971, e ao terminar envia (FFFF,0) a 276A. **23:276A** escolhe imagens 4C47/4C48 e percorre os desenhos da grade: é uma transição visual, sem predicado de vitória por preenchimento.

CG complementa a cadeia anterior: códigos de portão 1..8 alternam fechado/aberto por par; **6:35F5** aceita 0,2,4,6,8,9,10 como passagem. **6:32D5** abre a cor selecionada e fecha as outras cores abertas. A seleção 1..4 fica enfileirada no +8E do controlador e é aplicada em **7:0C3A** a cada dez ticks. **7:0D0D** fixa +54 como total da rodada (4 ou 8), enquanto +56 conta animais já gerados; os intervalos de geração são 80..199 ou 60..119 ticks conforme a configuração. Logo **7:105C** significa todos os animais previstos já gerados e nenhum dos dez slots ainda ativo. Sair pela coluna >=6 chama **6:2C25**, incrementando o indicador; a entrada nas flores passa por **7:0469 →6:36B9**, cuja rotina de voz em 36FF solicita 000141A6. O teste terminal não exige pontuação igual ao total: o fim e o reset descritos acima ocorrem também quando houve saída pelas flores.

Estas cadeias substituem as hipóteses anteriores sobre flags FM e os limites anteriores de conclusão FM. O movimento autônomo SB é fechado na seção seguinte. Não foi encontrado caminho de derrota FM no despacho de movimento e callbacks rastreados; não se deve inventar contagem regressiva, vidas ou vitória por ocupar a grade.

O laço dos agentes SB também está ligado ao controlador: **24:1B6D** chama 23:2B4A, e, se controlador+6E está armado, percorre os dez agentes; chama **24:1180** em cada um e remove/destrói apenas os que retornam zero. Repete a cada 50 por 37:1971. Em **1180**, flag agente+38 desvia à continuação; caso contrário o contador +0E precisa atingir dez para recalcular. Coordenadas negativas chamam 1375; coordenada ainda negativa ou célula recusada por 23:1EF0 retorna zero (11EB), resultando na remoção do agente, não em conclusão da cena. Para célula válida escolhe imagem pela tabela DGROUP:0DFC indexada por tipo+4 e direção+0C, reposiciona o desenho por 23:2477→24:0C68 e chama 17BB e 155F para atualização. Logo desaparecimento de um agente é recuperação local do controlador; o laço não contém chamada de vitória/derrota de atividade.

## Regras autônomas SB e seleção CG: fechamento das decisões

Para SB, `livre(r,c)` significa limites 0..9, **tile em 23..26** (23:1EF0) e ausência de agente em controlador+46 (24:1D9E). Os cinco deslocamentos de DGROUP:0DE8 são, na ordem: `0=(-1,0), 1=(0,1), 2=(1,0), 3=(0,-1), 4=(0,0)`. 24:1527 retorna a direção oposta `0→2,1→3,2→0,3→1,outra→4`.

| Método | Decisão completa |
|---|---|
| 24:1375 | Varre as 100 células; recolhe as livres e escolhe `random % quantidade`; grava agente+6/+8 e chama145A. Se nenhuma, não altera coordenadas;1180 poderá remover o agente. |
| 24:145A | Testa os cinco deslocamentos livres; escolhe ao acaso entre os aceitos e grava+0C. Nenhum aceito deixa direção0. O deslocamento parado normalmente é recusado pela própria ocupação do agente. |
| 24:155F | Cria lista A de todos os cinco destinos livres e lista B excluindo direção oposta à atual e direção4. Se B não vazia escolhe nela; senão escolhe A; sem A grava direção4 e retorna0. Escolha existente retorna1. |
| 24:1722 | Tenta exclusivamente direção oposta; só a grava se o destino for livre. Não aparece como chamada do laço1180 neste caminho. |
| 24:1684 | Interpola a distância do desenho à célula de destino em dez pares de deltas: `delta[i]=trunc((i+1)*D/11)-trunc(i*D/11)` para i0..9. |
| 24:1180 | Se+38 não zero retorna1 sem andar. Se fase+0E>=10, zera fase, reposiciona se coordenadas negativas por1375, remove-se se posição inválida ou tile não23..26, atualiza visual/posição, chama17BB e155F, soma deslocamento às coordenadas e prepara1684. Caso fase<10, atualiza visual quando fase0, aplica delta[fase] e incrementa fase. |

**Interações (24:17BB).** Só tipos de agente5,7,9 procuram alvo. Tipo5 aceita tiles9/8; tipo7 aceita21/17; tipo9 aceita15/13/14. Percorre cinco deslocamentos em ordem e, dentro deles, a lista de tiles na ordem indicada, rejeitando coordenadas fora0..9. Para cada alvo chama23:1F70 até uma chamada devolver sucesso. Se havia qualquer alvo,17BB devolve1 mesmo que todos tenham recusado; o chamador1180 ignora esse retorno. Os alvos são edifícios/objetos interativos, portanto não se deve aplicar a regra de caminhada23..26 à busca de interação.

**Porta de aceitação da interação (23:1F70).** Exceto agente9 contra tile13/14, exige que o agente esteja exatamente uma linha abaixo do alvo e na mesma coluna. Agente9 exige ainda `valor derivado +474 do alvo ==0`. Exige um dos cinco slots de animação+36E livre; do contrário retorna0. A seleção de MMS por2320 é:

| Agente / tile | MMS | Flag visual devolvida |
|---|---|---|
|5/8|01C8|1|
|5/9|01C9|0|
|7/21|01C6|1|
|7/17|01C7|1|
|9/15|00D8|1|
|9/13 ou14, alvo à direita|0134|0|
|9/13 ou14, alvo à esquerda|0135|1|
|9/13 ou14, alvo abaixo|0133|1|
|9/13 ou14, alvo acima|00D9|1|

A tabela2320 possui fallthroughs, mas17BB só envia os pares indicados. Inicia MMS com30:04DA, anexa à cena e registra callback índice0 **seletor1(slot,agente)**. MMS01C6 acrescenta callback índice1 **seletor2(célula)**, que muda o tile para2; MMS00D9/0133/0134/0135 acrescentam callback índice1 **seletor3(célula)**, que recalcula o derivado+474. Para posicionar0134 subtrai uma linha e coluna do alvo;0135 subtrai linha e soma coluna;0133 subtrai linha. Os outros mantêm célula-alvo. Após23:2434,01C9 desloca x em-16 e usa entrada8; outros deslocam x em-32 e usam entrada1; despacha essa entrada via30:0FBD→32:009A. O sucesso da preparação congela o agente por24:199B (+38=1), trocando visual para4056 quando a flag da tabela é1.

O callback seletor1 em **23:144F-1524** remove e destrói a animação, zera seu slot, chama **24:19E8(agente)** para descongelar+38 e restaurar imagem conforme tipo/direção, recalcula16DD da célula salva+382/+384 e grava+474. Assim o término de uma animação local retoma a circulação do agente e atualiza a célula. Não há conclusão de atividade nesses callbacks. As funções1375/145A/155F/17BB/199B/19E8 não solicitam áudio; os sons dessas interações devem vir das instruções MMS e da voz explícita de lançamento19..24, não de um som inventado a cada passo.

**CG, clique versus alteração silenciosa.** A fábrica2:30A9 cria desenho, comando2:2D91 e hitbox33:1903 sobre o desenho. O hitbox executa2:2DE0→2:34EE, o mesmo caminho das teclas1..4. Esse caminho retorna sem efeito quando flag+0A!=0. Caso habilitado, alterna estado+4: 0→1 interrompe o som anterior, solicita áudio com palavra alta1 e índice armazenado+1E, executa comando+18; 1→0 toca o mesmo som e executa comando+14. Ambos invalidam por2:2CE6. A fábrica inicializa+0A=0, escolhe fase visual0 ou4 conforme estado inicial e configura áudio por2:3454 com DWORD00034346 (índice4346, valor auxiliar3), criando o objeto de áudio por36:279B. Isso prova som do botão e não voz de troca de portão.

Na montagem CG6:2163/21A6, os comandos off/on são construídos com seletores `i+5` e `i+1`;2:30A9 recebe off em+14 e on em+18, estado inicial0 e identificador de hitbox03E9. Selecionar liga um comando1..4 que enfileira cor no controlador; desligar executa5..8. **2:35E2** apenas alterna+4 e invalida: não testa+0A, não toca áudio e não executa comandos. Por isso os seletores5..8 e o reset de rodada conseguem corrigir a apresentação sem reentrar no comando da cor.2:3614 anima a fase visual+8 em direção4 quando ligado e0 quando desligado. A alteração visual do botão e a aplicação de portões em7:0C3A são estados separados com o atraso de dez ticks já documentado.

Correção terminológica: global02A0/C8E escolhe o tema CS de cartazes/paletas. Não chamá-lo de dificuldade; a configuração externa E/H possui seu próprio campo recebido pelo construtor. Todos os usos descritos acima são comparações nativas de campos e tabelas, independentes dessa interpretação anterior.



## Vínculo de todas as grades e predicados terminais

| Família carregada | Consumidor e regras de todas as entradas |
|---|---|
|SB00DB/00DC/00DA|Todas as configurações/entradas usadas por23:3053 compartilham edição10x10, passagem tile23..26, dez agentes e cinco animações locais; tema apenas escolhe tabela/paletas. Linhas DE não entram no tabuleiro. Não existe condição de vitória por preencher100células; os callbacks retornam agentes ao laço.|
|CG, oito grades F0|Todas passam por6:306F e pela mesma conversão/portões de32D5/35F5 e controlador7. Não existe limiar de pontuação no término. O teste é +58==0, +56>=+54 e dez ponteiros+64 nulos; envia seletor10,6:2B81 chama slot4C(1),6:2AF3 reinicia controlador, zera indicador, avança grade e corrige botões. Animais que saem pelas flores também deixam o conjunto ativo; um resultado parcial não impede a próxima grade.|
|FM,22grades|Todas passam por17:197D e o mesmo tabuleiro14x10/dispatch1..9. E/H escolhe a metade11+11. A saída carregando pessoa gera a cadeia de sucesso17:0095→5:1A52(1), consumindo+A4; não existe contagem exigida de casas vazias ou de múltiplas pessoas neste caminho.|

Precisão de temporização CG: o método que configura E/H e total é **7:0CBC**, com ramo não zero80..199/4animais e zero60..119/8animais. **7:0D0D** é o método seguinte. No laço7:0B8E-0CB5, seleção pendente é aplicada quando contador+8C==9, depois zerada; contador volta a0 ao atingir10. A espera37:1971 vale100 normalmente e10 com+5A (buzina) ativo; após o ciclo de dez o flag de buzina é limpo. Portanto “dez ticks” significa dez iterações do controlador, e não um atraso constante de dez milissegundos.

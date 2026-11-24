# Atividades A: VM, DD, SG e GW — regras nativas

Análise estática do BIGJOBNG.EXE BR. Segmentos em decimal, offsets em hexadecimal. Foram lidos os segmentos 18, 20, 21, 25, 26 e 27 anotados com relocações NE e o grafo MMS corrigido. Não foi executado nem compilado o original. Os nomes descritivos abaixo derivam das operações; não são títulos oficiais.

## Resultados que corrigem a identificação somente por recursos

| Cena | Regra demonstrada | Resultado nativo |
|---|---|---|
| 5 VM | Memória: revelar dois itens, comparar tipo, retirar pares iguais; 12 itens / seis pares | +13A chega a 12 e seleciona a sequência de parabéns; erro de par somente permite nova tentativa |
| 13 DD | Quatro entidades divididas em equipe do jogador e adversários; movimento horizontal, colisões e dano | Nenhuma entidade da equipe do jogador viva: 0; nenhum adversário vivo: 1 |
| 15 SG | Selecionar arestas de uma malha; fechar grupos de quatro arestas e disputar pontos com adversário | Quando não restam arestas disponíveis, compara os dois placares; maior placar +4: 1; menor: 0 |
| 16 GW | Posicionar par de blocos de grãos em grade 5×8, deslocar/girar/soltar e formar sequências verticais correspondentes aos três pedidos | Terceiro pedido/caminhão concluído: 1; bloqueio na entrada do próximo par: 0 |

VM não deriva de 5:14FF; portanto não se deve aplicar automaticamente o mesmo registro de progresso e overlay das outras três cenas. SG possui ganchos e vigas, porém a regra computacional é fechamento de quatro lados com alternância de jogador; GW não é apenas um mecanismo livre de trigo.

## Entrada e resultado compartilhados

Mouse global: o hit-test escolhe filho (+14), e o evento executa seu comando (+4); o teclado da cena (+18) é caminho separado. Estados 1/2/3 no +26 de controladores MMS significam execução/controle do interpretador e não são, isoladamente, vitória ou derrota.

DD, SG e GW herdam a base 5:14FF. Seus IDs de progresso +64 são 12, 32 e 19, respectivamente; seus seletores de cena +66 são 13, 15 e 16. A conclusão passa pelo virtual +4C da cena para a base. O sucesso arma MMS 020112 e comando 2:2671 que chega ao gerenciador 6:026C; a falha arma 020116 e registra tentativa por 6:03E2. Não há transição para cenas 12/32/19 por esses IDs. O campo +7C não é o comando de saída: 5:1724 o entrega a 30:101E para animação de introdução/retomada. A saída comum é 5:0EF1 → 3:35A4 → 3:327F, retomando os grupos pais registrados: DD volta ao grupo 4 e depois 0; SG/GW voltam ao grupo 2 e depois 0. Os IDs +64 são progresso e não esses grupos.

A base condiciona cheats w/l à global 008A. SG possui outros comandos de diagnóstico sob a mesma global. Não transformar esses atalhos em controles normais do port.

## 5 / VM — jogo de memória

### Objetos e preparação

27:0000 aloca 0x16E bytes e instala vtable 1BC4. Aloca 0x60 bytes para 12 registros de oito bytes em +142. O inicializador 00B7–00FD grava em cada registro: imagem/identificador +0=-1, tipo +2=-1, estado +4=0, categoria +6=6. O campo de cena +13A inicia em zero; +148/+14A guardam seleção atual/anterior; +146 distingue primeira e segunda revelação; +150 é contador de frames; +158 registra comparação bem-sucedida. A global 2130 é um intertravamento de entrada.

A construção +0C, 047E, carrega VMbackground (4E92), chama 0916/09A9/0A06/17C3/0B0F/1B10 e anexa o controlador +13E. O MMS 01A4 é carregado no construtor em 016F–017E. O controlador adicional 3:24DE recebe uma transição para oficina e a identificação 5 (0326–0359), como já rastreado no relatório 11.

0B0F constrói 12 objetos selecionáveis. Para cada índice cria comando 27:1C10, ligado ao executor 1C66; este chama 0DC3 com o índice. O objeto visual é 27:2070 e recebe duas imagens adjacentes e a posição calculada por 0C63; o tipo/categoria é usado para escolher a imagem por 0D8E. O embaralhamento percorre seis valores sem repetição em 0D49–0D87. A expansão dos registros em 17C3–1B0F fornece os pares e posições; os pares não devem ser reconstruídos pela mera ordenação do catálogo PIC.

### Clique, revelação e comparação

O seletor 27:1EBC recusa enquanto +0A ou +12 do próprio objeto estão ativos. Exige hit-test positivo e global 2130 habilitada antes de armar o pressionamento (+10=1 em 1F8B–1FD8). Na soltura testa flag de evento 4 e hit-test; dispara o comando por virtual +14 em 1F2E–1F49. Logo arrastar para fora não equivale automaticamente a selecionar.

0DC3 zera 2130, toca a categoria pelo comando MMS escolhido em 0E12, grava o índice em +148 e prepara a representação. O callback comando 27:1CDA, seletor 0, chega a 15A9. Na primeira revelação (+146=1), copia a imagem do registro selecionado para +152, zera +150 e muda o sprite via 27:24A7; a animação 0EEC recebe seletor 0, percorre cinco imagens usando tabelas 0FA2/0FAC, e ao terminar grava +146=0 e +14A=+148 (0FD4–0FEF), liberando nova entrada.

Na segunda revelação, 15F6 volta +146 a 1 e compara os tipos (+2) dos registros +148 e +14A. O teste também exige estado +4=0 no registro atual. Tipos iguais produzem +158=1 e gravam as duas imagens em +154/+156 (164F–16CB); tipos diferentes deixam +158=0 (16D4).

A animação 0EEC seletor 1 revela o segundo sprite durante cinco frames. Depois:

- +158=0: 116E chama 1512 (fala de erro, sem repetir a última opção) e 08B5 para restaurar os objetos; 1512 libera 2130. Não decrementa uma reserva de vidas nesse caminho.
- +158=1: 0E7A remove os dois objetos do conjunto selecionável (27:22EA), incrementa +13A duas vezes em 06F1, e anima os dois sprites até a posição de destino. O deslocamento por frame é diferença/10 (1139–1164), com aproximação e ajuste final em 118F–145F.

Ao chegar ao destino, 118F chama 06F1 com as duas imagens, restaura a apresentação, e testa +13A==12 em 11E4. Antes de 12 libera a entrada; em 12 chama 16E3. Não há predicado de derrota por um par errado demonstrado neste circuito.

### Áudio, término e saída

As falas pertencem ao MMS 01A4: os registros de opcode 16 incluem CdItsaMatch01/02, CdGoodChoice, CdRightDiteDing02, CdYoureGoodAtThis; os erros CdNotThatOne, CdThatsNotaMatch01/02 e CdTryAgain01; finais CdCongratsYouWon/CdGoodForYouYouWon e alternativas CdNiceTryItsaBigJob/CdWhyNotTryLater. 1481 escolhe entre cinco entradas de uma tabela para acerto e evita repetir +14C; 1512 escolhe entre quatro entradas e evita +14E. 16E3 seleciona uma entrada da tabela 10CC e a executa no controlador +13E por 30:0FBD→32:009A. A presença das falas de desistência no MMS não estabelece falha automática de jogo.

O MMS 01A4 tem callbacks slot0 nos índices 118,163,208,253,298,343 e slot1 no índice1323; isso deve ser lido após permutar as palavras runtime. O controlador compartilhado 0113 usa CdYes e RmNo e callbacks 0/1 nos índices51/90. O relatório11 demonstra o comando de retorno registrado no índice1. O método 0641 executa comando slot1, anexa os 12 objetos de seleção e marca +104=1; os métodos 06C7/0781 removem/reanexam o controlador, pausando e retomando a apresentação.

## 13 / DD — entidades, equipes e dano

### Construção e controles

25:338C chama a base e guarda o parâmetro de variante em +98. 33E2 carrega DDbackground, cria botões esquerdo/direito e constrói controlador 26:0B31 em +94. A configuração 36A8–36D2 passa (8,3,4,2) quando +98=0 e (8,2,2,1) quando +98!=0; 26:1C12 armazena os quatro valores em +F6/+F8/+FA/+FC, que são limiares de decisões aleatórias da IA.

Teclado 25:3799: caractere '4' grava 1 na global0F16; '6' grava 1 na global0F14. O evento alternativo 3755 faz o mesmo para códigos 0x66/0x65. Em 26:0343, 0F14 tem prioridade, chama 07BE e zera a global; se ausente, 0F16 chama 078B e a zera. Os comandos são consumidos uma vez por atualização do jogador; não são coordenadas de mouse nem resultados.

26:0B31 cria quatro entidades (campo +44=4), vetor de ponteiros +5C. A primeira recebe bit de equipe +0=1; as restantes ficam com +0=0. Quando a variante é habilitada também atribui 40 à primeira entidade (0C58–0C69) e cria um objeto adicional em +B8; sem variante, +B8=0. +B4 é outro objeto/cenário selecionado entre PIC4938 e4939. O reinício 0A86 distribui as entidades em (100,100), incrementando x em140, quebrando para x80/y+100 depois de560; limpa fase +42, término +BC e resultado +BE.

### Atualização, IA e colisão

26:17EA processa ticks de60 via37:1971 e chama181F somente quando +FE=0. 25:3805→26:0A5C grava +FE=1 (pausa); 3821→0A71 grava0; o reset383D opcionalmente chama0A86 e sempre retoma.

Fases do controlador: +42=0 toca som436A e muda para1; fase1 atualiza entidades (18D8) e quando +BC!=0 passa para2; fase2 chama owner virtual +4C com +BE e muda para3 (18A3–18CD). Cada entidade morta (+1A=3) é excluída das decisões. As da equipe1 recebem entrada0343; as da equipe0 executam deslocamento em02EE e decisões periódicas, com limiares +F6..+FC contra sorteios 0..100/200 em19EB–1A8E. Estados de ação +1C=0,1,2,3 selecionam idle/1E7E/1FEB/1E19; não são resultados do minijogo.

2034 testa interseção estrita de retângulos entre entidades e objetos do cenário. O ramo20A5–20EB exige sobreposição nos dois eixos; toca dano se o objeto +B8 permite colisão, resolve a colisão por2E8E/2E33, e aplica dano100 à entidade (2185–2196). Há também colisões entre entidades e alteração de direção no ramo21B9–2225. O dano é centralizado em04D4: subtrai do HP +16, toca436D, e quando HP<=0 satura0, define +1A=3 e +8=0, atualiza o visual e toca a animação de morte (0532–0579). Esses testes são computacionais, independentes do nome DDexplod.

### Fim nativo

1B87 limpa +BC e percorre as quatro entidades, ignorando +1A=3. Conta separadamente sobreviventes +0!=0 e +0==0. Se não há equipe1: +BC=1/+BE=0. Caso contrário, se não há equipe0: +BC=1/+BE=1. Se ambas foram eliminadas no mesmo estado observado, o primeiro ramo dá derrota. A fase2 entrega o booleano ao owner. O atalho K só chama1B15 quando008A habilitada;1B15 provoca dano1000 em entidades adversárias sobreviventes após a primeira encontrada, e deve ser tratado como diagnóstico.

## 15 / SG — fechar quatro lados e vencer no placar

### Representação e seleção

21:0564 monta os auxiliares 10CF,0784,0A82,0C8A,11F9,1B40. 1210 monta a malha com dimensões +8A/+8C e limites espaciais +82..+88. Os grupos guardados na coleção +8E têm contador de lados em +0, retângulo em +2 e quatro ponteiros de arestas em +0A,+0E,+12,+16 (construtor0000). 0087 insere uma aresta no primeiro dos quatro slots vazios;00C9 recupera o slot por índice. Os loops17CA–189B associam cada aresta vertical a seus grupos vizinhos; bordas externas têm só um vizinho.

As arestas são objetos20:160A, imagens SG*H/V, com wrapper/comando20:14BF. A rotina1C13 recebe a aresta selecionada, consulta sua imagem (+14) para escolher controlador/apresentação e executa o comando que leva a1C91. Não basta tocar uma viga: o estado precisa ser registrado nos grupos.

### Regra de turno e pontuação

1C91 marca a aresta pela função virtual +20. Executa2396 e2454 para procurar primeiro e segundo grupo que a contenham. Para cada encontrado,0069 incrementa contador e só devolve1 quando o total se torna exatamente4. Grupo recém-fechado chama2526 para desenhar o preenchimento e contabiliza um ponto; uma aresta compartilhada pode fechar dois grupos.

O campo de cena +80 é o turno, e +C8→+52 espelha qual jogador está ativo. Se nenhum grupo fecha,1DBB–1DF5 alterna0↔1. Se fecha ao menos um, mantém o turno e soma o número de grupos fechados ao placar: +CC→+4 para turno0, +CC→+6 para turno1, via5:2D43/2D72. Portanto fechar uma caixa concede nova jogada e pode render dois pontos. A criação inicial usa parâmetro especial não zero em1C91, seleciona estilo3 e evita som/alternância/pontuação de uma jogada normal.

A rotina1E62 conduz o oponente/terminação. Se +80=1, escolhe aresta por1F7B e aciona1C13. Se é o turno0, percorre os quatro lados de todos os grupos, conta lados com estado +0E=0, e só chama2341 quando não existe nenhum disponível. A busca1F7B classifica candidatos por grau de grupos, prioriza fechamento e evita oportunidades ao oponente; os ramos2241–22C7 usam probabilidades de50% e70% na variante. As consultas2396 e2454 localizam grupo(s) da aresta;2454 só tem êxito na segunda ocorrência, o que explica a contagem de dois vizinhos.

### Resultado, áudio, reset

2341 compara +CC→+4 e +6. Se +4>+6 chama virtual+4C com1; se +4<+6 chama com0; igualdade retorna sem callback nesse método. Não inventar desempate nem contar empate como vitória. O reset1AC4, quando argumento!=0, zera ambos os placares, zera turno do controlador, remove as coleções/visuais por18D7, reconstrói0784/1210/1B40 e volta +80=0.

A jogada normal cancela áudio +D8 e seleciona a voz em +10A ou+114 conforme turno (1CED–1D34). As cenas carregam MMS0175/0176 em10CF para apresentação. O teclado E/F/H/L/W e minúsculas é inteiro condicionado à global008A nos ramos0343/03AF/041B/042D/04AD: pode forçar busca, marcar diagnóstico0C8C e completar arestas, mas não é o comando normal de jogo. O controlador AI28D0 possui tick0x800; sua animação2964 usa som4031 e passo/tempo diferentes no diagnóstico.

## 16 / GW — blocos de grãos e três pedidos

### Grade, controles e variantes

18:06E9 inicia a base, variante +A8 e constrói o fundo GWbackground, controles horário/anti-horário, esquerda/direita e plunger, sacos/pedidos e caminhões. A grade é um vetor5×8 de objetos em +104 (linha stride0x20, coluna stride4); tipo/conteúdo do bloco fica em +8. Há um par ativo em +8E com segundo elemento em +1E. Coordenadas discretas ficam em +0A/+0C. Os três pedidos têm tipos consecutivos em +BC/+BE/+C0 (registro stride8, flag +C2), estados em +AA/+AC/+AE e contador concluído +9C.

Teclado14FE: '4'/'6' criam comandos3A5C/3B2A; a/A e d/D criam38C0/398E; espaço/'2' aciona temporizador11:2295 para o par. Os executores resolvem pelos virtuais da cena:38C0→+54,398E→+58,3A5C→+5C,3B2A→+60. As rotinas1BCB/1E4F fazem rotação com checagem de posições livres e limites0..7/0..4;20B2 e seguintes deslocam o par. Variante+A8 bloqueia os giros nesses métodos. A tecla alternativa 'h' em13B8 dispara queda acelerada apenas quando o controlador+1A4→+42 é -1, gravando a posição em+42; não é resultado.

31AF centra o par na área+92, gera tipos com312D e posiciona a segunda peça em relação à primeira.312D sorteia entre tipos4A9F/4AA1/4AA3/4AA5 e especial4A92; o especial marca+1B4=1 e só aparece quando limite do sorteio permite. A variante modifica o limite (20/23) e suprime certas operações. O método318F rearma o controlador e marca o par+28=1, pronto para cair.

### Queda, bloqueio e reconhecimento de pedido

22A4 atualiza o par, exige temporizador disparado e flag+28. Consulta as células abaixo dos dois blocos, compara suas posições com os destinos discretos e trata queda conjunta/separada; só fixa quando há suporte ou limite. A escrita na grade usa0323, oculta os sprites ativos por02BA e mantém as posições dos sprites de célula. O ramo2951 distingue término normal de bloqueio. Normalmente chama3538 para reconhecer sequências e, se +28 continua ativo, gera o próximo par. Se a flag local -48 está ativa,2980–29E1 cria comando18:0000 com resultado0 e executa-o.

Comando18:0000 armazena owner+26 e booleano+2A; seu executor0056 chama199B(owner,booleano).199B cancela áudio/overlays, interrompe os controladores+1A4,+1A8,+1B0 quando ativos e chama o virtual+4C com o booleano recebido em1B48–1B55. Assim há caminho concreto bloqueio→0→base, não dedução pelo som.

3334 reconhece um pedido: para uma coluna válida0..7, procura uma janela vertical de comprimento+9A (2 ou3), percorrendo inícios0..5-comprimento. Compara tipos+8 de cada célula com a sequência do pedido (primeiro+BC, demais+BE/+C0), excluindo pedidos já sinalizados +C2 ou estado3. Só janela inteira correspondente marca+ C2=1.3538 testa as duas colunas em que o par acabou de pousar, evita repetir quando ambas são iguais, e, se houve correspondência e o par não está em estado especial+16, arma animações dos pedidos por2DBD e34A2.

### Vitória, mídia e reinício

3637 atualiza os três caminhões/pedidos: estado1 anda +5 por tick; estado2 retorna -20; estado3 está concluído. Quando o caminhão no estado1 cruza+ A2, cancela áudio+8A, limpa a flag de animação, grava estado3 e incrementa+9C (3743–376A). Quando+9C>=3,3777–37BF cria o mesmo comando18:0000, agora booleano1. O executor199B termina na base com sucesso. O critério é três partidas concluídas, não apenas três células iguais em qualquer orientação.

As falas dos tutoriais MMS0184/0185 confirmam a função das operações: TuArrangeGrainBlock, TuWhenBlocksMatch, TuDontLetBlocks, TuClickToMoveRight, TuClickToMoveLeft, TuClickToTurn (somente0184) e TuClickToGoTruck. Esses tutoriais têm callbacks0/1 nos índices10/206 para0184 e9/152 para0185;18:38A2 seleciona184/185 conforme+A8. O som4393 é retomado em1848 e197B;4394 acompanha tentativa de giro;4392 acompanha deslocamento;400F acompanha o plunger/limpeza308F. São IDs low word com parâmetros altos, não índices de32bits.

O reset18D9, quando solicitado, zera as40 células, gera novo par por318F, reativa controlador, inicializa+1B4=1/+9C=0 e repõe os três pedidos por2E5F. Os métodos172F/17BC pausam/retomam controladores MMS e áudio; não representam resultados por si.

## Fechamento das bifurcações estáticas

### VM: sorteio e tabelas completos

2:0012 implementa `lo + rand() % (hi-lo+1)` (001D-002A): ambos os extremos são inclusivos. 27:0D28 inicializa 23 marcas com -1 e obtém seis índices distintos em 0..22 por rejeição; não é permutação de apenas seis figuras. 17C3 inicializa 12 marcas de posição e seis marcas de categoria. Para cada um dos seis índices, sorteia posição A em 0..11 até estar livre, marca-a, sorteia B da mesma maneira, e marca-a. A seguir sorteia categoria A em 0..5. Se +13C=1, rejeita categorias já usadas, marca a escolhida e copia-a para B. Se +13C!=1, sorteia B independentemente em 0..5: as categorias podem repetir inclusive entre pares. 0CE0 atribui aos registros A/B as duas imagens do índice sorteado; 0C63 recupera tipo e estado de cada imagem por busca na tabela, primeira coluna produz estado1, segunda estado0. A categoria escolhe o sprite de apresentação em 0D8E e não substitui a comparação de tipo +2.

Tabela de imagem por índice, comum a DS:0FEA e DS:1046; cada linha é um tipo e suas duas imagens:

| Tipo | Imagem A | Imagem B |
|---|---|---|
| 0 | 4E72 | 4E77 |
| 1 | 4E7C | 4E81 |
| 2 | 4E86 | 4E8B |
| 3 | 4E97 | 4E9C |
| 4 | 4EAD | 4EB2 |
| 5 | 4EB7 | 4EBC |
| 6 | 4EC1 | 4EC6 |
| 7 | 4ECB | 4ED0 |
| 8 | 4ED5 | 4EDA |
| 9 | 4EDF | 4EE4 |
| 10 | 4EE9 | 4EEE |
| 11 | 4EF3 | 4EF8 |
| 12 | 4EFD | 4F02 |
| 13 | 4F07 | 4F0C |
| 14 | 4F11 | 4F16 |
| 15 | 4F1B | 4F20 |
| 16 | 4F3D | 4F42 |
| 17 | 4F51 | 4F56 |
| 18 | 4F5B | 4F60 |
| 19 | 4F65 | 4F6A |
| 20 | 4F47 | 4F4C |
| 21 | 4F6F | 4F74 |
| 22 | 4F83 | 4F88 |

As categorias 0..5 em DS:10A2 são, na ordem, 4F33,4F37,4F39,4F3B,4F35,4F31. 197F refaz o sorteio, reconstrói as duas imagens adjacentes de cada seletor, limpa +15A/+150/+13A e libera 2130; não preserva progresso de pares no reinício.

### SG: prioridade, duplicatas e continuação de empate

1F7B constrói três vetores com capacidade100, inicialmente nulos. Percorre grupos na ordem da coleção e lados na ordem0..3. O vetor geral recebe cada ocorrência de aresta livre, sem deduplicação: uma aresta compartilhada pode ter peso duplo. O vetor seguro recebe arestas de grupo com contador<2 somente se ambos os grupos vizinhos existentes também têm contador<2. O vetor de fechamento recebe o lado livre de cada grupo com contador3. A ordem não é uma ordenação por distância.

| Condição | Seleção |
|---|---|
| Variante0 e fechamento disponível | `rand()%n` no vetor de fechamento |
| Variante0, sem fechamento, seguro disponível | `rand()%n` no vetor seguro |
| Variante0, nenhum dos anteriores | `rand()%n` no geral, ou nulo se vazio |
| Variante!=0, fechamento disponível e `rand()%100<50` | Fechamento |
| Caso contrário, seguro disponível e novo `rand()%100<70` | Seguro |
| Caso contrário | Geral se não vazio; senão nulo |

Não há tentativa de escolher um seguro depois de falhar o sorteio70: cai diretamente no geral. Quando o geral está vazio, 2233 chama2341 antes de continuar pela seleção/liberação dos vetores; o retorno é nulo. Em1E62, turno1 somente apresenta aresta se o ponteiro retornado não é nulo; turno0 conta ocorrências livres e chama2341 quando são zero. Em empate2341 retorna por2391 sem +4C. Os dois chamadores então apenas liberam o iterador e retornam: nenhum arma filme, desempate, nova partida nem sucesso/falha. Após a apresentação,2964 limpa os alvos +46/+4E em2F0B/2F14, chama1E62 em2F24 e testa novamente esses ponteiros em2F3A; em empate eles permanecem nulos e a corrotina retorna em2F4F. Logo o caminho estático completo termina sem relatório de resultado e conserva o tabuleiro completo até reset/saída externa.

A geometria é construída antes dessa busca por1210-18D6: grupos em ordem de linhas/colunas, lados associados por ponteiros, bordas com um vizinho e interiores com dois. A fonte exata das multiplicações, limites e retângulos não é uma aproximação visual: está preservada integralmente no anexo de instruções abaixo, inclusive a ordem das inserções e arredondamentos de16bits.

### DD: ordem de atualização e colisões

18D8 processa entidades em ordem crescente. Primeiro atualiza/remove o visual transitório +74 se existente; morto(+1A=3) salta diretamente para03AE. Equipe1 consome0343. Equipe0 com +6!=0 executa02EE e não toma decisão naquele tick. Com +6=0 só decide em `global0FA0%2==0`. 07F1 verdadeiro faz ação2 voltar a0 e deixa outras ações intactas. Com 07F1 falso: ação1 chama1E7E; ação0 faz três sorteios independentes e sequenciais (`R(0,100)<+F6` chama0851; `R(0,200)<+FA` põe ação1; `R(0,200)<+FC` põe ação3). O terceiro pode sobrescrever o segundo no mesmo tick. Ação2 executa1FEB e depois `R(0,100)<+F8` retorna a0. Ação3 executa1E19. Ações restantes não chamam esses handlers. Todas as vivas passam depois por08E5 e09C5; a cada terceiro tick por0715, e por03AE. Só após as quatro entidades,2034 resolve colisões; depois incrementa0FA0. Não trocar a ordem por decisões simultâneas.

2034 tem três passes. Primeiro, para cada viva: (a) objeto+B8 existe e seu+10=0, retângulos estritamente sobrepostos,291D prepara as duas poses e2E8E confirma contato:2E33 consome o objeto e04D4 aplica100; (b) sobreposição estrita com retângulo+50..+56 limita +8 a2 somente se antes>2; (c) percorre objetos +8C, contando+46, com caixa local `(x+9,y+20,x+54,y+45)`, verifica interseção, calcula poses por291D e verifica novamente: chama099F,0700(0),0382. Essas operações não são a mesma coisa que dano100. O segundo passe percorre todos os pares `i<j`, testa interseção estrita e chama26BF. A rotina26BF decide a resposta mútua;24CF recebe origem/destino, propaga direção/velocidade, usa `max(velocidade,1)` e causa metade desse valor por04D4 no destino ainda vivo; velocidade>1 também produz efeito visual no ponto médio. Os guardas, seleção de direção, lados e saturações estão integralmente registrados no anexo, sem fundir os dois participantes em um único resultado. Tocar bordas exatas não satisfaz as interseções estritas.

26BF prepara dois retângulos por participante usando291D. A prioridade das quatro comparações é fixa: (1) retângulo modificado de B contra primeiro de A chama24CF com origem A/destino B; (2) modificado de A contra primeiro de B chama24CF com origem B/destino A; (3) primeiro de B contra primeiro de A freia ambos por099F/0700(0), volta ação a0 salvo ação3 e chama0851 somente nos participantes equipe0; (4) o último teste em28EF calcula condição mas não chama resposta, retornando2918 nos dois resultados. Não aplicar dano duplo quando testes se sobrepõem: os ramos1/2 saltam ao retorno. 2E8E faz OR de quatro sobreposições: cada um dos dois retângulos do objeto contra cada um dos dois passados. O primeiro positivo retorna1; quatro negativos retornam0.

### GW: orientação, pivô e gravidade

Para evitar inverter a grade: aqui `u=+0A` tem limite0..4 e `v=+0C` tem limite0..7; endereço `+104+32*u+4*v`. O pivô é a primeira peça A; B é+1E. Os métodos não trocam tipos A/B. Antes de girar,1BCB incrementa os v locais se a posição gráfica atual de A passou da posição discreta;1E4F faz esse ajuste quando a posição gráfica é maior ou igual. Ambos saem se variante!=0 ou tipo B=0. A tabela é aplicada em ordem:

| Relação inicial | 1BCB: destino de B | 1E4F: destino de B |
|---|---|---|
| uA<uB | (uA,vA+1) | (uA,vA-1) |
| uA>uB | (uA,vA-1) | (uA,vA+1) |
| uA=uB e vA<vB | (uA-1,vA) | (uA+1,vA) |
| Caso restante | (uA+1,vA) | (uA-1,vA) |

Cada ramo testa o limite do destino e, salvo o primeiro ramo de1E4F, testa tipo da célula destino=0 antes de alterar B. O primeiro ramo de1E4F só exige v local>0 (1F07-1F0B): não adicionar teste de ocupação ausente no original. Destino ocupado mantém coordenadas e passa por0528 com a pose existente; destino fora do limite limpa a flag local e retorna sem0528. Assim falha de limite e falha de ocupação têm efeitos de apresentação distintos.

22A4 primeiro chama04C8; temporizador inativo ou +28=0 retorna. Lê poses A/B e destinos discretos na próxima profundidade. Com controlador+42=-1 soma5 a ambas as poses; senão alinha A ao próximo destino e soma a mesma diferença a B. Se alguma célula abaixo está ocupada, alinha A ao destino. Enquanto ambas as poses estão antes do destino só move sprites e retorna. Ao atingir destino, tenta avançar ambas as coordenadas quando `<8` e as células estão livres; B vazia copia uA e vA-1 em vez de avançar como peça viva. Reconsulta suporte/limites depois de alterar coordenadas. Havendo suporte fixa; sem suporte continua. A sequência exata de testes, incluindo consulta local2380 que usa uA, é preservada no anexo: não corrigir silenciosamente para uB.

Fixação de par especial(+16!=0) oculta a peça, para controlador+1A4 e inicia+1B0 com vA; não escreve os tipos normais. Fixação normal prepara0528, toca4391, verifica ocupação das células atuais (qualquer ocupada arma flag local-48), escreve ambos os tipos por0323, oculta A/B e copia as poses para as células. Depois2951 decide: flag-48=0 reconhece pedidos3538 e só gera318F se +28 continua ativo; flag!=0 executa resultado0. O caminho especial termina no mesmo epílogo29E6. Nesse epílogo, se +1A4→+42!=-1, restaura o temporizador do par por11:2295 com esse valor e volta +42 a-1; caso contrário retorna diretamente. Isso inclui separadamente temporização, interpolação, avanço discreto, suporte, especial e bloqueio.

## Anexo: transcrição estática dos caminhos espaciais

Este anexo conserva todas as instruções das faixas indicadas, em ordem, com fixups NE. É parte do mapa estático e permite verificar cada condição e seu ramo complementar sem depender de uma implementação nova. Sintaxe AT&T: `cmp origem,destino` testa destino-origem; saltos assinados devem conservar semântica de16bits. Dados embutidos não são handlers; os limites de função e comentários do corpo principal delimitam a interpretação. Não constitui execução, compilação ou prova de equivalência quadro a quadro.

### SG: geometria completa (21:1210-18D6)

```asm
21:1210  incw	%bp
21:1211  pushw	%bp
21:1212  movw	%sp, %bp
21:1214  subw	$0x60, %sp
21:1217  pushw	%si
21:1218  pushw	%di
21:1219  movw	$0x0, -0x2(%bp)
21:121E  jmp	0x13e0 <.text+0x13e0>
21:1221  lesw	0x6(%bp), %bx
21:1224  movw	%es:0x88(%bx), %ax
21:1229  subw	%es:0x84(%bx), %ax
21:122E  imulw	-0x2(%bp)
21:1231  movw	%ax, -0x1a(%bp)
21:1234  movw	$0x0, -0x1e(%bp)
21:1239  movw	%ax, -0x1c(%bp)
21:123C  movl	-0x1e(%bp), %eax
21:1240  movl	%eax, -0x22(%bp)
21:1244  movw	0x8(%bp), %ax
21:1247  movw	0x6(%bp), %dx
21:124A  addw	$0x82, %dx
21:124E  movw	%ax, -0x24(%bp)
21:1251  movw	%dx, -0x26(%bp)
21:1254  lesw	-0x26(%bp), %bx
21:1257  movw	%es:0x6(%bx), %ax
21:125B  addw	-0x20(%bp), %ax
21:125E  movw	%ax, -0x28(%bp)
21:1261  movw	%es:0x4(%bx), %ax
21:1265  addw	-0x22(%bp), %ax
21:1268  movw	%ax, -0x2a(%bp)
21:126B  movw	%es:0x2(%bx), %ax
21:126F  addw	-0x20(%bp), %ax
21:1272  movw	%ax, -0x2c(%bp)
21:1275  movw	%es:(%bx), %ax
21:1278  addw	-0x22(%bp), %ax
21:127B  movw	%ax, -0x2e(%bp)
21:127E  movw	%ax, -0x36(%bp)
21:1281  movw	-0x2c(%bp), %ax
21:1284  movw	%ax, -0x34(%bp)
21:1287  movw	-0x2a(%bp), %ax
21:128A  movw	%ax, -0x32(%bp)
21:128D  movw	-0x28(%bp), %ax
21:1290  movw	%ax, -0x30(%bp)
21:1293  leaw	-0x18(%bp), %ax
21:1296  pushw	%ss
21:1297  pushw	%ax
21:1298  leaw	-0x36(%bp), %ax
21:129B  pushw	%ss
21:129C  pushw	%ax
21:129D  movw	$0x8, %cx
21:12A0  lcallw	$0x0, $0xffff           # imm = 0xFFFF ; NE 12A1=1:3E64
21:12A5  movw	$0x0, -0x4(%bp)
21:12AA  jmp	0x13ce <.text+0x13ce>
21:12AD  pushl	$0x0
21:12B0  movw	$0x0, %ax ; NE 12B1=49:1204
21:12B3  movw	%ax, %es
21:12B5  pushl	%es:0x1204
21:12BB  pushw	$0x1a
21:12BD  lcallw	$0x0, $0xffff           # imm = 0xFFFF ; NE 12BE=30:0113
21:12C2  addw	$0xa, %sp
21:12C5  movw	%dx, -0x3c(%bp)
21:12C8  movw	%ax, -0x3e(%bp)
21:12CB  orw	%dx, %ax
21:12CD  je	0x12eb <.text+0x12eb>
21:12CF  pushw	$0x0
21:12D1  leaw	-0x18(%bp), %ax
21:12D4  movw	%ss, %dx
21:12D6  movw	$0x8, %cx
21:12D9  lcallw	$0x0, $0xffff           # imm = 0xFFFF ; NE 12DA=1:3E86
21:12DE  pushl	-0x3e(%bp)
21:12E2  pushw	%cs
21:12E3  callw	0x0 <.text>
21:12E6  addw	$0xe, %sp
21:12E9  jmp	0x12f1 <.text+0x12f1>
21:12EB  movw	-0x3c(%bp), %dx
21:12EE  movw	-0x3e(%bp), %ax
21:12F1  movw	%dx, -0x38(%bp)
21:12F4  movw	%ax, -0x3a(%bp)
21:12F7  movl	-0x3a(%bp), %eax
21:12FB  movl	%eax, -0x42(%bp)
21:12FF  pushl	-0x42(%bp)
21:1303  movw	0x6(%bp), %ax
21:1306  addw	$0x8e, %ax
21:1309  pushw	0x8(%bp)
21:130C  pushw	%ax
21:130D  lcallw	$0x0, $0xffff           # imm = 0xFFFF ; NE 130E=29:36E8
21:1312  addw	$0x8, %sp
21:1315  movw	-0x14(%bp), %ax
21:1318  subw	-0x18(%bp), %ax
21:131B  movw	%ax, -0x44(%bp)
21:131E  movw	%ax, -0x48(%bp)
21:1321  movw	$0x0, -0x46(%bp)
21:1326  movl	-0x48(%bp), %eax
21:132A  movl	%eax, -0x4c(%bp)
21:132E  movw	-0x4c(%bp), %ax
21:1331  addw	%ax, -0x18(%bp)
21:1334  addw	%ax, -0x14(%bp)
21:1337  movw	-0x4a(%bp), %ax
21:133A  addw	%ax, -0x16(%bp)
21:133D  addw	%ax, -0x12(%bp)
21:1340  pushl	$0x0
21:1343  movw	$0x0, %ax ; NE 1344=49:1204
21:1346  movw	%ax, %es
21:1348  pushl	%es:0x1204
21:134E  pushw	$0x8
21:1350  lcallw	$0x0, $0xffff           # imm = 0xFFFF ; NE 1351=30:0113
21:1355  addw	$0xa, %sp
21:1358  movw	%dx, -0x52(%bp)
21:135B  movw	%ax, -0x54(%bp)
21:135E  orw	%dx, %ax
21:1360  je	0x1393 <.text+0x1393>
21:1362  pushw	$0x14
21:1364  lesw	-0x3a(%bp), %bx
21:1367  movw	%es:0x4(%bx), %ax
21:136B  movw	%ax, -0x56(%bp)
21:136E  movw	%es:0x2(%bx), %ax
21:1372  movw	%ax, -0x58(%bp)
21:1375  movw	%ax, -0x5c(%bp)
21:1378  movw	-0x56(%bp), %ax
21:137B  movw	%ax, -0x5a(%bp)
21:137E  pushl	-0x5c(%bp)
21:1382  pushw	$0x4cee                 # imm = 0x4CEE
21:1385  pushw	%dx
21:1386  pushw	-0x54(%bp)
21:1389  lcallw	$0x0, $0xffff           # imm = 0xFFFF ; NE 138A=37:011E
21:138E  addw	$0xc, %sp
21:1391  jmp	0x1399 <.text+0x1399>
21:1393  movw	-0x52(%bp), %dx
21:1396  movw	-0x54(%bp), %ax
21:1399  movw	%dx, -0x4e(%bp)
21:139C  movw	%ax, -0x50(%bp)
21:139F  pushw	%dx
21:13A0  pushw	%ax
21:13A1  pushl	0x6(%bp)
21:13A5  lcallw	$0x0, $0xffff           # imm = 0xFFFF ; NE 13A6=35:3B22
21:13AA  addw	$0x8, %sp
21:13AD  movl	-0x50(%bp), %eax
21:13B1  movl	%eax, -0x60(%bp)
21:13B5  pushl	-0x60(%bp)
21:13B9  movw	0x6(%bp), %ax
21:13BC  addw	$0x9c, %ax
21:13BF  pushw	0x8(%bp)
21:13C2  pushw	%ax
21:13C3  lcallw	$0x0, $0xffff           # imm = 0xFFFF ; NE 13C4=29:36E8
21:13C8  addw	$0x8, %sp
21:13CB  incw	-0x4(%bp)
21:13CE  lesw	0x6(%bp), %bx
21:13D1  movw	%es:0x8a(%bx), %ax
21:13D6  cmpw	-0x4(%bp), %ax
21:13D9  jg	0x12ad <.text+0x12ad>
21:13DD  incw	-0x2(%bp)
21:13E0  lesw	0x6(%bp), %bx
21:13E3  movw	%es:0x8c(%bx), %ax
21:13E8  cmpw	-0x2(%bp), %ax
21:13EB  jg	0x1221 <.text+0x1221>
21:13EF  movw	$0x0, -0x2(%bp)
21:13F4  jmp	0x1655 <.text+0x1655>
21:13F7  lesw	0x6(%bp), %bx
21:13FA  movw	%es:0x88(%bx), %ax
21:13FF  addw	$0x84, %bx
21:1403  movw	%bx, -0x6(%bp)
21:1406  subw	%es:(%bx), %ax
21:1409  imulw	-0x2(%bp)
21:140C  movw	%ax, -0x16(%bp)
21:140F  movw	$0x0, -0x1a(%bp)
21:1414  movw	%ax, -0x18(%bp)
21:1417  movl	-0x1a(%bp), %eax
21:141B  movl	%eax, -0x1e(%bp)
21:141F  movw	%es:(%bx), %ax
21:1422  movw	%ax, -0x20(%bp)
21:1425  movw	0x6(%bp), %bx
21:1428  movw	%es:0x82(%bx), %ax
21:142D  movw	%ax, -0x22(%bp)
21:1430  movw	%ax, -0x26(%bp)
21:1433  movw	-0x20(%bp), %ax
21:1436  movw	%ax, -0x24(%bp)
21:1439  movl	-0x26(%bp), %eax
21:143D  movl	%eax, -0x2a(%bp)
21:1441  movw	-0x28(%bp), %ax
21:1444  addw	-0x1c(%bp), %ax
21:1447  movw	%ax, -0x2c(%bp)
21:144A  movw	-0x2a(%bp), %ax
21:144D  addw	-0x1e(%bp), %ax
21:1450  movw	%ax, -0x2e(%bp)
21:1453  movw	%ax, -0x32(%bp)
21:1456  movw	-0x2c(%bp), %ax
21:1459  movw	%ax, -0x30(%bp)
21:145C  movl	-0x32(%bp), %eax
21:1460  movl	%eax, -0x14(%bp)
21:1464  subw	$0x3, -0x12(%bp)
21:1468  addw	$0x3, -0x14(%bp)
21:146C  movw	$0x0, -0x4(%bp)
21:1471  jmp	0x1643 <.text+0x1643>
21:1474  movl	-0x14(%bp), %eax
21:1478  movl	%eax, -0x36(%bp)
21:147C  subw	$0x3, -0x34(%bp)
21:1480  pushl	$0x0
21:1483  movw	$0x0, %ax ; NE 1484=49:1204
21:1486  movw	%ax, %es
21:1488  pushl	%es:0x1204
21:148E  pushw	$0x26
21:1490  lcallw	$0x0, $0xffff           # imm = 0xFFFF ; NE 1491=30:0113
21:1495  addw	$0xa, %sp
21:1498  movw	%dx, -0x3c(%bp)
21:149B  movw	%ax, -0x3e(%bp)
21:149E  orw	%dx, %ax
21:14A0  je	0x14be <.text+0x14be>
21:14A2  pushl	0x6(%bp)
21:14A6  pushw	$0x4cef                 # imm = 0x4CEF
21:14A9  pushw	%ss
21:14AA  leaw	-0x36(%bp), %ax
21:14AD  pushw	%ax
21:14AE  pushw	$0x0
21:14B0  pushw	%dx
21:14B1  pushw	-0x3e(%bp)
21:14B4  lcallw	$0x0, $0xffff           # imm = 0xFFFF ; NE 14B5=20:160A
21:14B9  addw	$0x10, %sp
21:14BC  jmp	0x14c4 <.text+0x14c4>
21:14BE  movw	-0x3c(%bp), %dx
21:14C1  movw	-0x3e(%bp), %ax
21:14C4  movw	%dx, -0x38(%bp)
21:14C7  movw	%ax, -0x3a(%bp)
21:14CA  pushw	$0x16
21:14CC  lesw	-0x3a(%bp), %bx
21:14CF  movw	%es, %ax
21:14D1  movw	%ax, -0x8(%bp)
21:14D4  movw	%es:(%bx), %dx
21:14D7  movw	%dx, -0xa(%bp)
21:14DA  pushw	%ax
21:14DB  pushw	%dx
21:14DC  lesw	-0xa(%bp), %bx
21:14DF  lesw	%es:(%bx), %bx
21:14E2  lcallw	*%es:0x34(%bx)
21:14E6  addw	$0x6, %sp
21:14E9  pushl	$0x0
21:14EC  movw	$0x0, %ax ; NE 14ED=49:1204
21:14EF  movw	%ax, %es
21:14F1  pushl	%es:0x1204
21:14F7  pushw	$0x8
21:14F9  lcallw	$0x0, $0xffff           # imm = 0xFFFF ; NE 14FA=30:0113
21:14FE  addw	$0xa, %sp
21:1501  movw	%dx, -0x44(%bp)
21:1504  movw	%ax, -0x46(%bp)
21:1507  orw	%dx, %ax
21:1509  je	0x151d <.text+0x151d>
21:150B  pushl	-0x3a(%bp)
21:150F  pushw	%dx
21:1510  pushw	-0x46(%bp)
21:1513  lcallw	$0x0, $0xffff           # imm = 0xFFFF ; NE 1514=20:14BF
21:1518  addw	$0x8, %sp
21:151B  jmp	0x1523 <.text+0x1523>
21:151D  movw	-0x44(%bp), %dx
21:1520  movw	-0x46(%bp), %ax
21:1523  movw	%dx, -0x40(%bp)
21:1526  movw	%ax, -0x42(%bp)
21:1529  pushw	%dx
21:152A  pushw	%ax
21:152B  pushl	0x6(%bp)
21:152F  lcallw	$0x0, $0xffff           # imm = 0xFFFF ; NE 1530=35:3B22
21:1534  addw	$0x8, %sp
21:1537  movl	-0x42(%bp), %eax
21:153B  movl	%eax, -0x4a(%bp)
21:153F  pushl	-0x4a(%bp)
21:1543  movw	0x6(%bp), %ax
21:1546  addw	$0xaa, %ax
21:1549  pushw	0x8(%bp)
21:154C  pushw	%ax
21:154D  lcallw	$0x0, $0xffff           # imm = 0xFFFF ; NE 154E=29:36E8
21:1552  addw	$0x8, %sp
21:1555  cmpw	$0x0, -0x2(%bp)
21:1559  jle	0x15c2 <.text+0x15c2>
21:155B  lesw	0x6(%bp), %bx
21:155E  movw	%es:0x8a(%bx), %ax
21:1563  movw	-0x2(%bp), %dx
21:1566  decw	%dx
21:1567  imulw	%dx
21:1569  addw	-0x4(%bp), %ax
21:156C  pushw	%ax
21:156D  movw	0x6(%bp), %ax
21:1570  addw	$0x8e, %ax
21:1573  pushw	0x8(%bp)
21:1576  pushw	%ax
21:1577  pushw	%ss
21:1578  leaw	-0x56(%bp), %ax
21:157B  pushw	%ax
21:157C  lcallw	$0x0, $0xffff           # imm = 0xFFFF ; NE 157D=29:3B7F
21:1581  addw	$0xa, %sp
21:1584  movw	%dx, -0x58(%bp)
21:1587  movw	%ax, -0x5a(%bp)
21:158A  pushw	%dx
21:158B  pushw	%ax
21:158C  movw	0x6(%bp), %ax
21:158F  addw	$0x8e, %ax
21:1592  pushw	0x8(%bp)
21:1595  pushw	%ax
21:1596  lcallw	$0x0, $0xffff           # imm = 0xFFFF ; NE 1597=29:3780
21:159B  addw	$0x8, %sp
21:159E  movw	%dx, -0x4c(%bp)
21:15A1  movw	%ax, -0x4e(%bp)
21:15A4  pushw	$0x2
21:15A6  pushw	%ss
21:15A7  leaw	-0x56(%bp), %ax
21:15AA  pushw	%ax
21:15AB  lcallw	$0x0, $0xffff           # imm = 0xFFFF ; NE 15AC=29:3BCC
21:15B0  addw	$0x6, %sp
21:15B3  pushl	-0x3a(%bp)
21:15B7  pushl	-0x4e(%bp)
21:15BB  pushw	%cs
21:15BC  callw	0x87 <.text+0x87>
21:15BF  addw	$0x8, %sp
21:15C2  lesw	0x6(%bp), %bx
21:15C5  movw	%es:0x8c(%bx), %ax
21:15CA  cmpw	-0x2(%bp), %ax
21:15CD  jle	0x1630 <.text+0x1630>
21:15CF  movw	%es:0x8a(%bx), %ax
21:15D4  imulw	-0x2(%bp)
21:15D7  addw	-0x4(%bp), %ax
21:15DA  pushw	%ax
21:15DB  movw	0x6(%bp), %ax
21:15DE  addw	$0x8e, %ax
21:15E1  pushw	0x8(%bp)
21:15E4  pushw	%ax
21:15E5  pushw	%ss
21:15E6  leaw	-0x56(%bp), %ax
21:15E9  pushw	%ax
21:15EA  lcallw	$0x0, $0xffff           # imm = 0xFFFF ; NE 15EB=29:3B7F
21:15EF  addw	$0xa, %sp
21:15F2  movw	%dx, -0x58(%bp)
21:15F5  movw	%ax, -0x5a(%bp)
21:15F8  pushw	%dx
21:15F9  pushw	%ax
21:15FA  movw	0x6(%bp), %ax
21:15FD  addw	$0x8e, %ax
21:1600  pushw	0x8(%bp)
21:1603  pushw	%ax
21:1604  lcallw	$0x0, $0xffff           # imm = 0xFFFF ; NE 1605=29:3780
21:1609  addw	$0x8, %sp
21:160C  movw	%dx, -0x4c(%bp)
21:160F  movw	%ax, -0x4e(%bp)
21:1612  pushw	$0x2
21:1614  pushw	%ss
21:1615  leaw	-0x56(%bp), %ax
21:1618  pushw	%ax
21:1619  lcallw	$0x0, $0xffff           # imm = 0xFFFF ; NE 161A=29:3BCC
21:161E  addw	$0x6, %sp
21:1621  pushl	-0x3a(%bp)
21:1625  pushl	-0x4e(%bp)
21:1629  pushw	%cs
21:162A  callw	0x87 <.text+0x87>
21:162D  addw	$0x8, %sp
21:1630  lesw	0x6(%bp), %bx
21:1633  movw	%es:0x86(%bx), %ax
21:1638  subw	%es:0x82(%bx), %ax
21:163D  addw	%ax, -0x14(%bp)
21:1640  incw	-0x4(%bp)
21:1643  lesw	0x6(%bp), %bx
21:1646  movw	%es:0x8a(%bx), %ax
21:164B  cmpw	-0x4(%bp), %ax
21:164E  jg	0x1474 <.text+0x1474>
21:1652  incw	-0x2(%bp)
21:1655  lesw	0x6(%bp), %bx
21:1658  movw	%es:0x8c(%bx), %ax
21:165D  cmpw	-0x2(%bp), %ax
21:1660  jge	0x13f7 <.text+0x13f7>
21:1664  movw	$0x0, -0x2(%bp)
21:1669  jmp	0x18c3 <.text+0x18c3>
21:166C  lesw	0x6(%bp), %bx
21:166F  movw	%es:0x88(%bx), %ax
21:1674  addw	$0x84, %bx
21:1678  movw	%bx, -0xc(%bp)
21:167B  subw	%es:(%bx), %ax
21:167E  imulw	-0x2(%bp)
21:1681  movw	%ax, -0x16(%bp)
21:1684  movw	$0x0, -0x1a(%bp)
21:1689  movw	%ax, -0x18(%bp)
21:168C  movl	-0x1a(%bp), %eax
21:1690  movl	%eax, -0x1e(%bp)
21:1694  movw	%es:(%bx), %ax
21:1697  movw	%ax, -0x20(%bp)
21:169A  movw	0x6(%bp), %bx
21:169D  movw	%es:0x82(%bx), %ax
21:16A2  movw	%ax, -0x22(%bp)
21:16A5  movw	%ax, -0x26(%bp)
21:16A8  movw	-0x20(%bp), %ax
21:16AB  movw	%ax, -0x24(%bp)
21:16AE  movl	-0x26(%bp), %eax
21:16B2  movl	%eax, -0x2a(%bp)
21:16B6  movw	-0x28(%bp), %ax
21:16B9  addw	-0x1c(%bp), %ax
21:16BC  movw	%ax, -0x2c(%bp)
21:16BF  movw	-0x2a(%bp), %ax
21:16C2  addw	-0x1e(%bp), %ax
21:16C5  movw	%ax, -0x2e(%bp)
21:16C8  movw	%ax, -0x32(%bp)
21:16CB  movw	-0x2c(%bp), %ax
21:16CE  movw	%ax, -0x30(%bp)
21:16D1  movl	-0x32(%bp), %eax
21:16D5  movl	%eax, -0x14(%bp)
21:16D9  subw	$0x3, -0x14(%bp)
21:16DD  addw	$0x3, -0x12(%bp)
21:16E1  movw	$0x0, -0x4(%bp)
21:16E6  jmp	0x18b1 <.text+0x18b1>
21:16E9  movl	-0x14(%bp), %eax
21:16ED  movl	%eax, -0x36(%bp)
21:16F1  subw	$0x3, -0x36(%bp)
21:16F5  pushl	$0x0
21:16F8  movw	$0x0, %ax ; NE 16F9=49:1204
21:16FB  movw	%ax, %es
21:16FD  pushl	%es:0x1204
21:1703  pushw	$0x26
21:1705  lcallw	$0x0, $0xffff           # imm = 0xFFFF ; NE 1706=30:0113
21:170A  addw	$0xa, %sp
21:170D  movw	%dx, -0x3c(%bp)
21:1710  movw	%ax, -0x3e(%bp)
21:1713  orw	%dx, %ax
21:1715  je	0x1733 <.text+0x1733>
21:1717  pushl	0x6(%bp)
21:171B  pushw	$0x4cf0                 # imm = 0x4CF0
21:171E  pushw	%ss
21:171F  leaw	-0x36(%bp), %ax
21:1722  pushw	%ax
21:1723  pushw	$0x0
21:1725  pushw	%dx
21:1726  pushw	-0x3e(%bp)
21:1729  lcallw	$0x0, $0xffff           # imm = 0xFFFF ; NE 172A=20:160A
21:172E  addw	$0x10, %sp
21:1731  jmp	0x1739 <.text+0x1739>
21:1733  movw	-0x3c(%bp), %dx
21:1736  movw	-0x3e(%bp), %ax
21:1739  movw	%dx, -0x38(%bp)
21:173C  movw	%ax, -0x3a(%bp)
21:173F  pushw	$0x16
21:1741  lesw	-0x3a(%bp), %bx
21:1744  movw	%es, %ax
21:1746  movw	%ax, -0xe(%bp)
21:1749  movw	%es:(%bx), %dx
21:174C  movw	%dx, -0x10(%bp)
21:174F  pushw	%ax
21:1750  pushw	%dx
21:1751  lesw	-0x10(%bp), %bx
21:1754  lesw	%es:(%bx), %bx
21:1757  lcallw	*%es:0x34(%bx)
21:175B  addw	$0x6, %sp
21:175E  pushl	$0x0
21:1761  movw	$0x0, %ax ; NE 1762=49:1204
21:1764  movw	%ax, %es
21:1766  pushl	%es:0x1204
21:176C  pushw	$0x8
21:176E  lcallw	$0x0, $0xffff           # imm = 0xFFFF ; NE 176F=30:0113
21:1773  addw	$0xa, %sp
21:1776  movw	%dx, -0x44(%bp)
21:1779  movw	%ax, -0x46(%bp)
21:177C  orw	%dx, %ax
21:177E  je	0x1792 <.text+0x1792>
21:1780  pushl	-0x3a(%bp)
21:1784  pushw	%dx
21:1785  pushw	-0x46(%bp)
21:1788  lcallw	$0x0, $0xffff           # imm = 0xFFFF ; NE 1789=20:14BF
21:178D  addw	$0x8, %sp
21:1790  jmp	0x1798 <.text+0x1798>
21:1792  movw	-0x44(%bp), %dx
21:1795  movw	-0x46(%bp), %ax
21:1798  movw	%dx, -0x40(%bp)
21:179B  movw	%ax, -0x42(%bp)
21:179E  pushw	%dx
21:179F  pushw	%ax
21:17A0  pushl	0x6(%bp)
21:17A4  lcallw	$0x0, $0xffff           # imm = 0xFFFF ; NE 17A5=35:3B22
21:17A9  addw	$0x8, %sp
21:17AC  movl	-0x42(%bp), %eax
21:17B0  movl	%eax, -0x4a(%bp)
21:17B4  pushl	-0x4a(%bp)
21:17B8  movw	0x6(%bp), %ax
21:17BB  addw	$0xaa, %ax
21:17BE  pushw	0x8(%bp)
21:17C1  pushw	%ax
21:17C2  lcallw	$0x0, $0xffff           # imm = 0xFFFF ; NE 17C3=29:36E8
21:17C7  addw	$0x8, %sp
21:17CA  cmpw	$0x0, -0x4(%bp)
21:17CE  jle	0x1835 <.text+0x1835>
21:17D0  lesw	0x6(%bp), %bx
21:17D3  movw	%es:0x8a(%bx), %ax
21:17D8  imulw	-0x2(%bp)
21:17DB  addw	-0x4(%bp), %ax
21:17DE  decw	%ax
21:17DF  pushw	%ax
21:17E0  movw	0x6(%bp), %ax
21:17E3  addw	$0x8e, %ax
21:17E6  pushw	0x8(%bp)
21:17E9  pushw	%ax
21:17EA  pushw	%ss
21:17EB  leaw	-0x56(%bp), %ax
21:17EE  pushw	%ax
21:17EF  lcallw	$0x0, $0xffff           # imm = 0xFFFF ; NE 17F0=29:3B7F
21:17F4  addw	$0xa, %sp
21:17F7  movw	%dx, -0x58(%bp)
21:17FA  movw	%ax, -0x5a(%bp)
21:17FD  pushw	%dx
21:17FE  pushw	%ax
21:17FF  movw	0x6(%bp), %ax
21:1802  addw	$0x8e, %ax
21:1805  pushw	0x8(%bp)
21:1808  pushw	%ax
21:1809  lcallw	$0x0, $0xffff           # imm = 0xFFFF ; NE 180A=29:3780
21:180E  addw	$0x8, %sp
21:1811  movw	%dx, -0x4c(%bp)
21:1814  movw	%ax, -0x4e(%bp)
21:1817  pushw	$0x2
21:1819  pushw	%ss
21:181A  leaw	-0x56(%bp), %ax
21:181D  pushw	%ax
21:181E  lcallw	$0x0, $0xffff           # imm = 0xFFFF ; NE 181F=29:3BCC
21:1823  addw	$0x6, %sp
21:1826  pushl	-0x3a(%bp)
21:182A  pushl	-0x4e(%bp)
21:182E  pushw	%cs
21:182F  callw	0x87 <.text+0x87>
21:1832  addw	$0x8, %sp
21:1835  lesw	0x6(%bp), %bx
21:1838  movw	%es:0x8a(%bx), %ax
21:183D  cmpw	-0x4(%bp), %ax
21:1840  jle	0x189e <.text+0x189e>
21:1842  imulw	-0x2(%bp)
21:1845  addw	-0x4(%bp), %ax
21:1848  pushw	%ax
21:1849  movw	0x6(%bp), %ax
21:184C  addw	$0x8e, %ax
21:184F  pushw	0x8(%bp)
21:1852  pushw	%ax
21:1853  pushw	%ss
21:1854  leaw	-0x56(%bp), %ax
21:1857  pushw	%ax
21:1858  lcallw	$0x0, $0xffff           # imm = 0xFFFF ; NE 1859=29:3B7F
21:185D  addw	$0xa, %sp
21:1860  movw	%dx, -0x58(%bp)
21:1863  movw	%ax, -0x5a(%bp)
21:1866  pushw	%dx
21:1867  pushw	%ax
21:1868  movw	0x6(%bp), %ax
21:186B  addw	$0x8e, %ax
21:186E  pushw	0x8(%bp)
21:1871  pushw	%ax
21:1872  lcallw	$0x0, $0xffff           # imm = 0xFFFF ; NE 1873=29:3780
21:1877  addw	$0x8, %sp
21:187A  movw	%dx, -0x4c(%bp)
21:187D  movw	%ax, -0x4e(%bp)
21:1880  pushw	$0x2
21:1882  pushw	%ss
21:1883  leaw	-0x56(%bp), %ax
21:1886  pushw	%ax
21:1887  lcallw	$0x0, $0xffff           # imm = 0xFFFF ; NE 1888=29:3BCC
21:188C  addw	$0x6, %sp
21:188F  pushl	-0x3a(%bp)
21:1893  pushl	-0x4e(%bp)
21:1897  pushw	%cs
21:1898  callw	0x87 <.text+0x87>
21:189B  addw	$0x8, %sp
21:189E  lesw	0x6(%bp), %bx
21:18A1  movw	%es:0x86(%bx), %ax
21:18A6  subw	%es:0x82(%bx), %ax
21:18AB  addw	%ax, -0x14(%bp)
21:18AE  incw	-0x4(%bp)
21:18B1  lesw	0x6(%bp), %bx
21:18B4  movw	%es:0x8a(%bx), %ax
21:18B9  cmpw	-0x4(%bp), %ax
21:18BC  jge	0x16e9 <.text+0x16e9>
21:18C0  incw	-0x2(%bp)
21:18C3  lesw	0x6(%bp), %bx
21:18C6  movw	%es:0x8c(%bx), %ax
21:18CB  cmpw	-0x2(%bp), %ax
21:18CE  jg	0x166c <.text+0x166c>
21:18D2  popw	%di
21:18D3  popw	%si
21:18D4  leave
21:18D5  decw	%bp
21:18D6  lretw
```

### SG: montagem e seleção da IA (21:1F7B-2317)

```asm
21:1F7B  incw	%bp
21:1F7C  pushw	%bp
21:1F7D  movw	%sp, %bp
21:1F7F  subw	$0x3e, %sp
21:1F82  pushw	%si
21:1F83  pushw	%di
21:1F84  pushw	$0x0
21:1F86  movw	0x6(%bp), %ax
21:1F89  addw	$0x8e, %ax
21:1F8C  pushw	0x8(%bp)
21:1F8F  pushw	%ax
21:1F90  pushw	%ss
21:1F91  leaw	-0xc(%bp), %ax
21:1F94  pushw	%ax
21:1F95  lcallw	$0x0, $0xffff           # imm = 0xFFFF ; NE 1F96=29:3B24
21:1F9A  addw	$0xa, %sp
21:1F9D  pushl	$0x0
21:1FA0  movw	$0x0, %ax ; NE 1FA1=49:1204
21:1FA3  movw	%ax, %es
21:1FA5  pushl	%es:0x1204
21:1FAB  pushw	$0x640                  # imm = 0x640
21:1FAE  lcallw	$0x0, $0xffff           # imm = 0xFFFF ; NE 1FAF=30:0163
21:1FB3  addw	$0xa, %sp
21:1FB6  movw	%dx, -0xe(%bp)
21:1FB9  movw	%ax, -0x10(%bp)
21:1FBC  pushl	$0x0
21:1FBF  movw	$0x0, %ax ; NE 1FC0=49:1204
21:1FC2  movw	%ax, %es
21:1FC4  pushl	%es:0x1204
21:1FCA  pushw	$0x640                  # imm = 0x640
21:1FCD  lcallw	$0x0, $0xffff           # imm = 0xFFFF ; NE 1FCE=30:0163
21:1FD2  addw	$0xa, %sp
21:1FD5  movw	%dx, -0x12(%bp)
21:1FD8  movw	%ax, -0x14(%bp)
21:1FDB  pushl	$0x0
21:1FDE  movw	$0x0, %ax ; NE 1FDF=49:1204
21:1FE1  movw	%ax, %es
21:1FE3  pushl	%es:0x1204
21:1FE9  pushw	$0x640                  # imm = 0x640
21:1FEC  lcallw	$0x0, $0xffff           # imm = 0xFFFF ; NE 1FED=30:0163
21:1FF1  addw	$0xa, %sp
21:1FF4  movw	%dx, -0x16(%bp)
21:1FF7  movw	%ax, -0x18(%bp)
21:1FFA  movw	$0x0, -0x1c(%bp)
21:1FFF  movw	$0x0, -0x1e(%bp)
21:2004  movw	$0x0, -0x20(%bp)
21:2009  movl	$0x0, -0x28(%bp)
21:2011  movw	$0x0, -0x1a(%bp)
21:2016  movw	-0x18(%bp), %cx
21:2019  movw	-0x14(%bp), %ax
21:201C  movw	%ax, -0x2c(%bp)
21:201F  movw	-0x10(%bp), %ax
21:2022  movw	%ax, -0x2e(%bp)
21:2025  movw	-0x16(%bp), %es
21:2028  movw	%cx, %bx
21:202A  xorw	%ax, %ax
21:202C  xorw	%dx, %dx
21:202E  movw	%ax, %es:0x2(%bx)
21:2032  movw	%dx, %es:(%bx)
21:2035  movw	-0x12(%bp), %es
21:2038  movw	-0x2c(%bp), %bx
21:203B  movw	%ax, %es:0x2(%bx)
21:203F  movw	%dx, %es:(%bx)
21:2042  movw	-0xe(%bp), %es
21:2045  movw	-0x2e(%bp), %bx
21:2048  movw	%ax, %es:0x2(%bx)
21:204C  movw	%dx, %es:(%bx)
21:204F  addw	$0x4, %cx
21:2052  addw	$0x4, -0x2c(%bp)
21:2056  addw	$0x4, -0x2e(%bp)
21:205A  incw	-0x1a(%bp)
21:205D  cmpw	$0x64, -0x1a(%bp)
21:2061  jl	0x2025 <.text+0x2025>
21:2063  pushw	%ss
21:2064  leaw	-0xc(%bp), %ax
21:2067  pushw	%ax
21:2068  lcallw	$0x0, $0xffff           # imm = 0xFFFF ; NE 2069=29:3C20
21:206D  addw	$0x4, %sp
21:2070  movw	-0x18(%bp), %ax
21:2073  movw	-0x20(%bp), %dx
21:2076  shlw	$0x2, %dx
21:2079  addw	%dx, %ax
21:207B  movw	%ax, -0x36(%bp)
21:207E  movw	-0x14(%bp), %ax
21:2081  movw	-0x1e(%bp), %dx
21:2084  shlw	$0x2, %dx
21:2087  addw	%dx, %ax
21:2089  movw	%ax, -0x38(%bp)
21:208C  movw	-0x10(%bp), %ax
21:208F  movw	-0x1c(%bp), %dx
21:2092  shlw	$0x2, %dx
21:2095  addw	%dx, %ax
21:2097  movw	%ax, -0x3a(%bp)
21:209A  jmp	0x2214 <.text+0x2214>
21:209D  pushw	%ss
21:209E  leaw	-0xc(%bp), %ax
21:20A1  pushw	%ax
21:20A2  movw	0x6(%bp), %ax
21:20A5  addw	$0x8e, %ax
21:20A8  pushw	0x8(%bp)
21:20AB  pushw	%ax
21:20AC  lcallw	$0x0, $0xffff           # imm = 0xFFFF ; NE 20AD=29:3780
21:20B1  addw	$0x8, %sp
21:20B4  movw	%dx, -0x2(%bp)
21:20B7  movw	%ax, -0x4(%bp)
21:20BA  lesw	-0x4(%bp), %bx
21:20BD  movw	%es:(%bx), %ax
21:20C0  movw	%ax, -0x2a(%bp)
21:20C3  movw	$0x0, -0x1a(%bp)
21:20C8  movw	-0x3a(%bp), %ax
21:20CB  movw	%ax, -0x30(%bp)
21:20CE  pushw	-0x1a(%bp)
21:20D1  pushl	-0x4(%bp)
21:20D5  pushw	%cs
21:20D6  callw	0xc9 <.text+0xc9>
21:20D9  addw	$0x6, %sp
21:20DC  movw	%dx, -0x22(%bp)
21:20DF  movw	%ax, -0x24(%bp)
21:20E2  lesw	-0x24(%bp), %bx
21:20E5  cmpw	$0x0, %es:0xe(%bx)
21:20EA  jne	0x210a <.text+0x210a>
21:20EC  movw	-0xe(%bp), %es
21:20EF  movw	-0x30(%bp), %bx
21:20F2  movw	-0x22(%bp), %ax
21:20F5  movw	-0x24(%bp), %dx
21:20F8  movw	%ax, %es:0x2(%bx)
21:20FC  movw	%dx, %es:(%bx)
21:20FF  addw	$0x4, -0x30(%bp)
21:2103  addw	$0x4, -0x3a(%bp)
21:2107  incw	-0x1c(%bp)
21:210A  incw	-0x1a(%bp)
21:210D  cmpw	$0x4, -0x1a(%bp)
21:2111  jl	0x20ce <.text+0x20ce>
21:2113  cmpw	$0x2, -0x2a(%bp)
21:2117  jge	0x21b1 <.text+0x21b1>
21:211B  movw	$0x0, -0x1a(%bp)
21:2120  movw	-0x38(%bp), %ax
21:2123  movw	%ax, -0x32(%bp)
21:2126  pushw	-0x1a(%bp)
21:2129  pushl	-0x4(%bp)
21:212D  pushw	%cs
21:212E  callw	0xc9 <.text+0xc9>
21:2131  addw	$0x6, %sp
21:2134  movw	%dx, -0x22(%bp)
21:2137  movw	%ax, -0x24(%bp)
21:213A  lesw	-0x24(%bp), %bx
21:213D  cmpw	$0x0, %es:0xe(%bx)
21:2142  jne	0x21a6 <.text+0x21a6>
21:2144  pushw	%ss
21:2145  leaw	-0x3e(%bp), %ax
21:2148  pushw	%ax
21:2149  pushl	-0x24(%bp)
21:214D  pushl	0x6(%bp)
21:2151  nop
21:2152  pushw	%cs
21:2153  callw	0x2396 <.text+0x2396>
21:2156  addw	$0xc, %sp
21:2159  orw	%ax, %ax
21:215B  je	0x2166 <.text+0x2166>
21:215D  lesw	-0x3e(%bp), %bx
21:2160  cmpw	$0x2, %es:(%bx)
21:2164  jge	0x21a6 <.text+0x21a6>
21:2166  pushw	%ss
21:2167  leaw	-0x3e(%bp), %ax
21:216A  pushw	%ax
21:216B  pushl	-0x24(%bp)
21:216F  pushl	0x6(%bp)
21:2173  nop
21:2174  pushw	%cs
21:2175  callw	0x2454 <.text+0x2454>
21:2178  addw	$0xc, %sp
21:217B  orw	%ax, %ax
21:217D  je	0x2188 <.text+0x2188>
21:217F  lesw	-0x3e(%bp), %bx
21:2182  cmpw	$0x2, %es:(%bx)
21:2186  jge	0x21a6 <.text+0x21a6>
21:2188  movw	-0x12(%bp), %es
21:218B  movw	-0x32(%bp), %bx
21:218E  movw	-0x22(%bp), %ax
21:2191  movw	-0x24(%bp), %dx
21:2194  movw	%ax, %es:0x2(%bx)
21:2198  movw	%dx, %es:(%bx)
21:219B  addw	$0x4, -0x32(%bp)
21:219F  addw	$0x4, -0x38(%bp)
21:21A3  incw	-0x1e(%bp)
21:21A6  incw	-0x1a(%bp)
21:21A9  cmpw	$0x4, -0x1a(%bp)
21:21AD  jl	0x2126 <.text+0x2126>
21:21B1  cmpw	$0x3, -0x2a(%bp)
21:21B5  jne	0x2207 <.text+0x2207>
21:21B7  movw	$0x0, -0x1a(%bp)
21:21BC  movw	-0x36(%bp), %ax
21:21BF  movw	%ax, -0x34(%bp)
21:21C2  pushw	-0x1a(%bp)
21:21C5  pushl	-0x4(%bp)
21:21C9  pushw	%cs
21:21CA  callw	0xc9 <.text+0xc9>
21:21CD  addw	$0x6, %sp
21:21D0  movw	%dx, -0x22(%bp)
21:21D3  movw	%ax, -0x24(%bp)
21:21D6  lesw	-0x24(%bp), %bx
21:21D9  cmpw	$0x0, %es:0xe(%bx)
21:21DE  jne	0x21fe <.text+0x21fe>
21:21E0  movw	-0x16(%bp), %es
21:21E3  movw	-0x34(%bp), %bx
21:21E6  movw	-0x22(%bp), %ax
21:21E9  movw	-0x24(%bp), %dx
21:21EC  movw	%ax, %es:0x2(%bx)
21:21F0  movw	%dx, %es:(%bx)
21:21F3  addw	$0x4, -0x34(%bp)
21:21F7  addw	$0x4, -0x36(%bp)
21:21FB  incw	-0x20(%bp)
21:21FE  incw	-0x1a(%bp)
21:2201  cmpw	$0x4, -0x1a(%bp)
21:2205  jl	0x21c2 <.text+0x21c2>
21:2207  pushw	%ss
21:2208  leaw	-0xc(%bp), %ax
21:220B  pushw	%ax
21:220C  lcallw	$0x0, $0xffff           # imm = 0xFFFF ; NE 220D=29:3D37
21:2211  addw	$0x4, %sp
21:2214  pushw	%ss
21:2215  leaw	-0xc(%bp), %ax
21:2218  pushw	%ax
21:2219  lcallw	$0x0, $0xffff           # imm = 0xFFFF ; NE 221A=29:3DC9
21:221E  addw	$0x4, %sp
21:2221  orw	%ax, %ax
21:2223  jne	0x209d <.text+0x209d>
21:2227  cmpw	$0x0, -0x1c(%bp)
21:222B  jne	0x2241 <.text+0x2241>
21:222D  pushl	0x6(%bp)
21:2231  nop
21:2232  pushw	%cs
21:2233  callw	0x2341 <.text+0x2341>
21:2236  addw	$0x4, %sp
21:2239  movl	$0x0, -0x28(%bp)
21:2241  cmpw	$0x0, 0xa(%bp)
21:2245  je	0x227b <.text+0x227b>
21:2247  cmpw	$0x0, -0x20(%bp)
21:224B  je	0x225d <.text+0x225d>
21:224D  lcallw	$0x0, $0xffff           # imm = 0xFFFF ; NE 224E=1:313B
21:2252  movw	$0x64, %bx
21:2255  cltd
21:2256  idivw	%bx
21:2258  cmpw	$0x32, %dx
21:225B  jl	0x2281 <.text+0x2281>
21:225D  cmpw	$0x0, -0x1e(%bp)
21:2261  je	0x2273 <.text+0x2273>
21:2263  lcallw	$0x0, $0xffff           # imm = 0xFFFF ; NE 2264=1:313B
21:2268  movw	$0x64, %bx
21:226B  cltd
21:226C  idivw	%bx
21:226E  cmpw	$0x46, %dx
21:2271  jl	0x229b <.text+0x229b>
21:2273  cmpw	$0x0, -0x1c(%bp)
21:2277  je	0x22d6 <.text+0x22d6>
21:2279  jmp	0x22b5 <.text+0x22b5>
21:227B  cmpw	$0x0, -0x20(%bp)
21:227F  je	0x2295 <.text+0x2295>
21:2281  lcallw	$0x0, $0xffff           # imm = 0xFFFF ; NE 2282=1:313B
21:2286  cltd
21:2287  idivw	-0x20(%bp)
21:228A  shlw	$0x2, %dx
21:228D  movw	-0x16(%bp), %es
21:2290  addw	-0x18(%bp), %dx
21:2293  jmp	0x22c7 <.text+0x22c7>
21:2295  cmpw	$0x0, -0x1e(%bp)
21:2299  je	0x22af <.text+0x22af>
21:229B  lcallw	$0x0, $0xffff           # imm = 0xFFFF ; NE 229C=1:313B
21:22A0  cltd
21:22A1  idivw	-0x1e(%bp)
21:22A4  shlw	$0x2, %dx
21:22A7  movw	-0x12(%bp), %es
21:22AA  addw	-0x14(%bp), %dx
21:22AD  jmp	0x22c7 <.text+0x22c7>
21:22AF  cmpw	$0x0, -0x1c(%bp)
21:22B3  je	0x22d6 <.text+0x22d6>
21:22B5  lcallw	$0x0, $0xffff           # imm = 0xFFFF ; NE 22B6=1:313B
21:22BA  cltd
21:22BB  idivw	-0x1c(%bp)
21:22BE  shlw	$0x2, %dx
21:22C1  movw	-0xe(%bp), %es
21:22C4  addw	-0x10(%bp), %dx
21:22C7  movw	%dx, %bx
21:22C9  movw	%es:0x2(%bx), %ax
21:22CD  movw	%es:(%bx), %dx
21:22D0  movw	%ax, -0x26(%bp)
21:22D3  movw	%dx, -0x28(%bp)
21:22D6  pushl	-0x10(%bp)
21:22DA  lcallw	$0x0, $0xffff           # imm = 0xFFFF ; NE 22DB=30:025B
21:22DF  addw	$0x4, %sp
21:22E2  pushl	-0x14(%bp)
21:22E6  lcallw	$0x0, $0xffff           # imm = 0xFFFF ; NE 22E7=30:025B
21:22EB  addw	$0x4, %sp
21:22EE  pushl	-0x18(%bp)
21:22F2  lcallw	$0x0, $0xffff           # imm = 0xFFFF ; NE 22F3=30:025B
21:22F7  addw	$0x4, %sp
21:22FA  movw	-0x26(%bp), %dx
21:22FD  movw	-0x28(%bp), %ax
21:2300  pushw	%dx
21:2301  pushw	%ax
21:2302  pushw	$0x2
21:2304  pushw	%ss
21:2305  leaw	-0xc(%bp), %ax
21:2308  pushw	%ax
21:2309  lcallw	$0x0, $0xffff           # imm = 0xFFFF ; NE 230A=29:3BCC
21:230E  addw	$0x6, %sp
21:2311  popw	%ax
21:2312  popw	%dx
21:2313  popw	%di
21:2314  popw	%si
21:2315  leave
21:2316  decw	%bp
21:2317  lretw
```

### SG: apresentação e continuação (21:2964-2F53)

```asm
21:2964  incw	%bp
21:2965  pushw	%bp
21:2966  movw	%sp, %bp
21:2968  subw	$0x158, %sp             # imm = 0x158
21:296C  pushw	%si
21:296D  pushw	%di
21:296E  cmpw	$0x0, 0xc8c
21:2973  je	0x297a <.text+0x297a>
21:2975  movw	$0x2, %ax
21:2978  jmp	0x297d <.text+0x297d>
21:297A  movw	$0x14, %ax
21:297D  movw	%ax, -0xa(%bp)
21:2980  cmpw	$0x0, 0xc8c
21:2985  je	0x298c <.text+0x298c>
21:2987  movw	$0x2, %ax
21:298A  jmp	0x298f <.text+0x298f>
21:298C  movw	$0xa, %ax
21:298F  movw	%ax, -0xc(%bp)
21:2992  movw	$0x2bc, -0x14(%bp)      # imm = 0x2BC
21:2997  movw	$0x0, -0x12(%bp)
21:299C  movl	-0x14(%bp), %eax
21:29A0  movl	%eax, -0x10(%bp)
21:29A4  jmp	0x2f3a <.text+0x2f3a>
21:29A7  lesw	0x6(%bp), %bx
21:29AA  cmpw	$0x0, %es:0x52(%bx)
21:29AF  je	0x29c1 <.text+0x29c1>
21:29B1  movw	$0x0, -0x1c(%bp)
21:29B6  movw	$0x64, -0x1a(%bp)
21:29BB  movl	-0x1c(%bp), %eax
21:29BF  jmp	0x29cf <.text+0x29cf>
21:29C1  movw	$0x280, -0x20(%bp)      # imm = 0x280
21:29C6  movw	$0x64, -0x1e(%bp)
21:29CB  movl	-0x20(%bp), %eax
21:29CF  movl	%eax, -0x18(%bp)
21:29D3  pushw	$0x1
21:29D5  pushw	%ss
21:29D6  leaw	-0x158(%bp), %ax
21:29DA  pushw	%ax
21:29DB  lcallw	$0x0, $0xffff           # imm = 0xFFFF ; NE 29DC=36:279B
21:29E0  addw	$0x6, %sp
21:29E3  pushl	$0x0
21:29E6  pushl	$0x14031                # imm = 0x14031
21:29EC  pushw	%ss
21:29ED  leaw	-0x158(%bp), %ax
21:29F1  pushw	%ax
21:29F2  lcallw	$0x0, $0xffff           # imm = 0xFFFF ; NE 29F3=36:28F7
21:29F7  addw	$0xc, %sp
21:29FA  lesw	0x6(%bp), %bx
21:29FD  movw	%es:0x48(%bx), %ax
21:2A01  movw	%ax, -0x22(%bp)
21:2A04  movw	%es:0x46(%bx), %dx
21:2A08  movw	%dx, -0x24(%bp)
21:2A0B  pushw	%ax
21:2A0C  pushw	%dx
21:2A0D  pushw	%ss
21:2A0E  leaw	-0x50(%bp), %ax
21:2A11  pushw	%ax
21:2A12  lesw	-0x24(%bp), %bx
21:2A15  lesw	%es:(%bx), %bx
21:2A18  lcallw	*%es:0x8(%bx)
21:2A1C  addw	$0x8, %sp
21:2A1F  movw	-0x4a(%bp), %ax
21:2A22  subw	-0x4e(%bp), %ax
21:2A25  movw	%ax, -0x52(%bp)
21:2A28  movw	-0x4c(%bp), %ax
21:2A2B  subw	-0x50(%bp), %ax
21:2A2E  movw	%ax, -0x54(%bp)
21:2A31  movw	%ax, -0x58(%bp)
21:2A34  movw	-0x52(%bp), %ax
21:2A37  movw	%ax, -0x56(%bp)
21:2A3A  movl	-0x58(%bp), %eax
21:2A3E  movl	%eax, -0x5c(%bp)
21:2A42  movw	-0x16(%bp), %ax
21:2A45  subw	-0x5a(%bp), %ax
21:2A48  movw	%ax, -0x5e(%bp)
21:2A4B  movw	-0x18(%bp), %ax
21:2A4E  subw	-0x5c(%bp), %ax
21:2A51  movw	%ax, -0x60(%bp)
21:2A54  movw	%ax, -0x64(%bp)
21:2A57  movw	-0x5e(%bp), %ax
21:2A5A  movw	%ax, -0x62(%bp)
21:2A5D  movl	-0x64(%bp), %eax
21:2A61  movl	%eax, -0x4(%bp)
21:2A65  movw	$0x3, -0x68(%bp)
21:2A6A  movw	$0x3, -0x66(%bp)
21:2A6F  movl	-0x68(%bp), %eax
21:2A73  movl	%eax, -0x6c(%bp)
21:2A77  lesw	0x6(%bp), %bx
21:2A7A  movw	%es:0x48(%bx), %ax
21:2A7E  movw	%ax, -0x26(%bp)
21:2A81  movw	%es:0x46(%bx), %dx
21:2A85  movw	%dx, -0x28(%bp)
21:2A88  pushw	%ax
21:2A89  pushw	%dx
21:2A8A  pushw	%ss
21:2A8B  leaw	-0x74(%bp), %ax
21:2A8E  pushw	%ax
21:2A8F  lesw	-0x28(%bp), %bx
21:2A92  lesw	%es:(%bx), %bx
21:2A95  lcallw	*%es:0x8(%bx)
21:2A99  addw	$0x8, %sp
21:2A9C  movw	-0x6e(%bp), %ax
21:2A9F  subw	-0x72(%bp), %ax
21:2AA2  movw	%ax, -0x76(%bp)
21:2AA5  movw	-0x70(%bp), %ax
21:2AA8  subw	-0x74(%bp), %ax
21:2AAB  movw	%ax, -0x78(%bp)
21:2AAE  movw	%ax, -0x7c(%bp)
21:2AB1  movw	-0x76(%bp), %ax
21:2AB4  movw	%ax, -0x7a(%bp)
21:2AB7  movl	-0x7c(%bp), %eax
21:2ABB  movl	%eax, -0x80(%bp)
21:2ABF  lesw	0x6(%bp), %bx
21:2AC2  movw	%es:0x50(%bx), %ax
21:2AC6  movw	%ax, -0x2a(%bp)
21:2AC9  movw	%es:0x4e(%bx), %dx
21:2ACD  movw	%dx, -0x2c(%bp)
21:2AD0  pushw	%ax
21:2AD1  pushw	%dx
21:2AD2  pushw	%ss
21:2AD3  leaw	-0x88(%bp), %ax
21:2AD7  pushw	%ax
21:2AD8  lesw	-0x2c(%bp), %bx
21:2ADB  lesw	%es:0x2(%bx), %bx
21:2ADF  lcallw	*%es:0x8(%bx)
21:2AE3  addw	$0x8, %sp
21:2AE6  movw	-0x82(%bp), %ax
21:2AEA  movw	%ax, -0x8a(%bp)
21:2AEE  movw	-0x84(%bp), %ax
21:2AF2  movw	%ax, -0x8c(%bp)
21:2AF6  movw	%ax, -0x90(%bp)
21:2AFA  movw	-0x8a(%bp), %ax
21:2AFE  movw	%ax, -0x8e(%bp)
21:2B02  movl	-0x90(%bp), %eax
21:2B07  movl	%eax, -0x94(%bp)
21:2B0C  movw	-0x92(%bp), %ax
21:2B10  subw	-0x7e(%bp), %ax
21:2B13  movw	%ax, -0x96(%bp)
21:2B17  movw	-0x94(%bp), %ax
21:2B1B  subw	-0x80(%bp), %ax
21:2B1E  movw	%ax, -0x98(%bp)
21:2B22  movw	%ax, -0x9c(%bp)
21:2B26  movw	-0x96(%bp), %ax
21:2B2A  movw	%ax, -0x9a(%bp)
21:2B2E  movl	-0x9c(%bp), %eax
21:2B33  movl	%eax, -0xa0(%bp)
21:2B38  movw	-0x9e(%bp), %ax
21:2B3C  subw	-0x6a(%bp), %ax
21:2B3F  movw	%ax, -0xa2(%bp)
21:2B43  movw	-0xa0(%bp), %ax
21:2B47  subw	-0x6c(%bp), %ax
21:2B4A  movw	%ax, -0xa4(%bp)
21:2B4E  movw	%ax, -0xa8(%bp)
21:2B52  movw	-0xa2(%bp), %ax
21:2B56  movw	%ax, -0xa6(%bp)
21:2B5A  movl	-0xa8(%bp), %eax
21:2B5F  movl	%eax, -0x8(%bp)
21:2B63  movw	-0xa(%bp), %ax
21:2B66  movw	%ax, -0xb2(%bp)
21:2B6A  movw	-0x8(%bp), %ax
21:2B6D  subw	-0x4(%bp), %ax
21:2B70  movw	%ax, -0xb4(%bp)
21:2B74  movw	$0x0, -0xae(%bp) ; NE 2B78=21:2F54
21:2B7A  movw	$0x2f54, -0xb0(%bp)     # imm = 0x2F54
21:2B80  movw	%ax, -0xac(%bp)
21:2B84  movw	-0xb2(%bp), %ax
21:2B88  movw	%ax, -0xaa(%bp)
21:2B8C  movw	$0x0, -0xae(%bp) ; NE 2B90=21:2F5C
21:2B92  movw	$0x2f5c, -0xb0(%bp)     # imm = 0x2F5C
21:2B98  movw	-0xa(%bp), %ax
21:2B9B  movw	%ax, -0xbe(%bp)
21:2B9F  movw	-0x6(%bp), %ax
21:2BA2  subw	-0x2(%bp), %ax
21:2BA5  movw	%ax, -0xc0(%bp)
21:2BA9  movw	$0x0, -0xba(%bp) ; NE 2BAD=21:2F54
21:2BAF  movw	$0x2f54, -0xbc(%bp)     # imm = 0x2F54
21:2BB5  movw	%ax, -0xb8(%bp)
21:2BB9  movw	-0xbe(%bp), %ax
21:2BBD  movw	%ax, -0xb6(%bp)
21:2BC1  movw	$0x0, -0xba(%bp) ; NE 2BC5=21:2F5C
21:2BC7  movw	$0x2f5c, -0xbc(%bp)     # imm = 0x2F5C
21:2BCD  movw	$0x1, -0xc2(%bp)
21:2BD3  jmp	0x2c3a <.text+0x2c3a>
21:2BD5  pushw	-0xc2(%bp)
21:2BD9  pushw	%ss
21:2BDA  leaw	-0xb0(%bp), %ax
21:2BDE  pushw	%ax
21:2BDF  pushw	%cs
21:2BE0  callw	0x2813 <.text+0x2813>
21:2BE3  addw	$0x6, %sp
21:2BE6  addw	%ax, -0x4(%bp)
21:2BE9  pushw	-0xc2(%bp)
21:2BED  pushw	%ss
21:2BEE  leaw	-0xbc(%bp), %ax
21:2BF2  pushw	%ax
21:2BF3  pushw	%cs
21:2BF4  callw	0x2813 <.text+0x2813>
21:2BF7  addw	$0x6, %sp
21:2BFA  addw	%ax, -0x2(%bp)
21:2BFD  pushl	-0x4(%bp)
21:2C01  lesw	0x6(%bp), %bx
21:2C04  movw	%es:0x48(%bx), %ax
21:2C08  movw	%ax, -0x2e(%bp)
21:2C0B  movw	%es:0x46(%bx), %dx
21:2C0F  movw	%dx, -0x30(%bp)
21:2C12  pushw	%ax
21:2C13  pushw	%dx
21:2C14  lesw	-0x30(%bp), %bx
21:2C17  lesw	%es:(%bx), %bx
21:2C1A  lcallw	*%es:0x14(%bx)
21:2C1E  addw	$0x8, %sp
21:2C21  pushl	$0x32
21:2C24  pushl	0x6(%bp)
21:2C28  lcallw	$0x0, $0xffff           # imm = 0xFFFF ; NE 2C29=37:1971
21:2C2D  addw	$0x8, %sp
21:2C30  orw	%ax, %ax
21:2C32  je	0x2eb5 <.text+0x2eb5>
21:2C36  incw	-0xc2(%bp)
21:2C3A  movw	-0xc2(%bp), %ax
21:2C3E  cmpw	-0xa(%bp), %ax
21:2C41  jle	0x2bd5 <.text+0x2bd5>
21:2C43  pushl	-0x10(%bp)
21:2C47  lesw	0x6(%bp), %bx
21:2C4A  movw	%es:0x48(%bx), %ax
21:2C4E  movw	%ax, -0x32(%bp)
21:2C51  movw	%es:0x46(%bx), %dx
21:2C55  movw	%dx, -0x34(%bp)
21:2C58  pushw	%ax
21:2C59  pushw	%dx
21:2C5A  lesw	-0x34(%bp), %bx
21:2C5D  lesw	%es:(%bx), %bx
21:2C60  lcallw	*%es:0x14(%bx)
21:2C64  addw	$0x8, %sp
21:2C67  lesw	0x6(%bp), %bx
21:2C6A  movl	%es:0x4a(%bx), %eax
21:2C6F  movl	%eax, %es:0x46(%bx)
21:2C74  movw	%es:0x48(%bx), %ax
21:2C78  movw	%ax, -0x36(%bp)
21:2C7B  movw	%es:0x46(%bx), %dx
21:2C7F  movw	%dx, -0x38(%bp)
21:2C82  pushw	%ax
21:2C83  pushw	%dx
21:2C84  pushw	%ss
21:2C85  leaw	-0xca(%bp), %ax
21:2C89  pushw	%ax
21:2C8A  lesw	-0x38(%bp), %bx
21:2C8D  lesw	%es:(%bx), %bx
21:2C90  lcallw	*%es:0x8(%bx)
21:2C94  addw	$0x8, %sp
21:2C97  movw	-0xc4(%bp), %ax
21:2C9B  subw	-0xc8(%bp), %ax
21:2C9F  movw	%ax, -0xcc(%bp)
21:2CA3  movw	-0xc6(%bp), %ax
21:2CA7  subw	-0xca(%bp), %ax
21:2CAB  movw	%ax, -0xce(%bp)
21:2CAF  movw	%ax, -0xd2(%bp)
21:2CB3  movw	-0xcc(%bp), %ax
21:2CB7  movw	%ax, -0xd0(%bp)
21:2CBB  movl	-0xd2(%bp), %eax
21:2CC0  movl	%eax, -0xd6(%bp)
21:2CC5  lesw	0x6(%bp), %bx
21:2CC8  movw	%es:0x50(%bx), %ax
21:2CCC  movw	%ax, -0x3a(%bp)
21:2CCF  movw	%es:0x4e(%bx), %dx
21:2CD3  movw	%dx, -0x3c(%bp)
21:2CD6  pushw	%ax
21:2CD7  pushw	%dx
21:2CD8  pushw	%ss
21:2CD9  leaw	-0xde(%bp), %ax
21:2CDD  pushw	%ax
21:2CDE  lesw	-0x3c(%bp), %bx
21:2CE1  lesw	%es:0x2(%bx), %bx
21:2CE5  lcallw	*%es:0x8(%bx)
21:2CE9  addw	$0x8, %sp
21:2CEC  movw	-0xd8(%bp), %ax
21:2CF0  movw	%ax, -0xe0(%bp)
21:2CF4  movw	-0xda(%bp), %ax
21:2CF8  movw	%ax, -0xe2(%bp)
21:2CFC  movw	%ax, -0xe6(%bp)
21:2D00  movw	-0xe0(%bp), %ax
21:2D04  movw	%ax, -0xe4(%bp)
21:2D08  movl	-0xe6(%bp), %eax
21:2D0D  movl	%eax, -0xea(%bp)
21:2D12  movw	-0xe8(%bp), %ax
21:2D16  subw	-0xd4(%bp), %ax
21:2D1A  movw	%ax, -0xec(%bp)
21:2D1E  movw	-0xea(%bp), %ax
21:2D22  subw	-0xd6(%bp), %ax
21:2D26  movw	%ax, -0xee(%bp)
21:2D2A  movw	%ax, -0xf2(%bp)
21:2D2E  movw	-0xec(%bp), %ax
21:2D32  movw	%ax, -0xf0(%bp)
21:2D36  movl	-0xf2(%bp), %eax
21:2D3B  movl	%eax, -0x4(%bp)
21:2D3F  lesw	0x6(%bp), %bx
21:2D42  movw	%es:0x48(%bx), %ax
21:2D46  movw	%ax, -0x3e(%bp)
21:2D49  movw	%es:0x46(%bx), %dx
21:2D4D  movw	%dx, -0x40(%bp)
21:2D50  pushw	%ax
21:2D51  pushw	%dx
21:2D52  pushw	%ss
21:2D53  leaw	-0xfa(%bp), %ax
21:2D57  pushw	%ax
21:2D58  lesw	-0x40(%bp), %bx
21:2D5B  lesw	%es:(%bx), %bx
21:2D5E  lcallw	*%es:0x8(%bx)
21:2D62  addw	$0x8, %sp
21:2D65  movw	-0xf4(%bp), %ax
21:2D69  subw	-0xf8(%bp), %ax
21:2D6D  movw	%ax, -0xfc(%bp)
21:2D71  movw	-0xf6(%bp), %ax
21:2D75  subw	-0xfa(%bp), %ax
21:2D79  movw	%ax, -0xfe(%bp)
21:2D7D  movw	%ax, -0x102(%bp)
21:2D81  movw	-0xfc(%bp), %ax
21:2D85  movw	%ax, -0x100(%bp)
21:2D89  movl	-0x102(%bp), %eax
21:2D8E  movl	%eax, -0x106(%bp)
21:2D93  movw	-0x16(%bp), %ax
21:2D96  subw	-0x104(%bp), %ax
21:2D9A  movw	%ax, -0x108(%bp)
21:2D9E  movw	-0x18(%bp), %ax
21:2DA1  subw	-0x106(%bp), %ax
21:2DA5  movw	%ax, -0x10a(%bp)
21:2DA9  movw	%ax, -0x10e(%bp)
21:2DAD  movw	-0x108(%bp), %ax
21:2DB1  movw	%ax, -0x10c(%bp)
21:2DB5  movl	-0x10e(%bp), %eax
21:2DBA  movl	%eax, -0x8(%bp)
21:2DBE  pushw	%ss
21:2DBF  leaw	-0x158(%bp), %ax
21:2DC3  pushw	%ax
21:2DC4  lcallw	$0x0, $0xffff           # imm = 0xFFFF ; NE 2DC5=36:2A8F
21:2DC9  addw	$0x4, %sp
21:2DCC  pushw	$0x0
21:2DCE  lesw	0x6(%bp), %bx
21:2DD1  pushl	%es:0x4e(%bx)
21:2DD6  pushl	%es:0x42(%bx)
21:2DDB  nop
21:2DDC  pushw	%cs
21:2DDD  callw	0x1c91 <.text+0x1c91>
21:2DE0  addw	$0xa, %sp
21:2DE3  movw	-0xc(%bp), %ax
21:2DE6  movw	%ax, -0x118(%bp)
21:2DEA  movw	-0x8(%bp), %ax
21:2DED  subw	-0x4(%bp), %ax
21:2DF0  movw	%ax, -0x11a(%bp)
21:2DF4  movw	$0x0, -0x114(%bp) ; NE 2DF8=21:2F54
21:2DFA  movw	$0x2f54, -0x116(%bp)    # imm = 0x2F54
21:2E00  movw	%ax, -0x112(%bp)
21:2E04  movw	-0x118(%bp), %ax
21:2E08  movw	%ax, -0x110(%bp)
21:2E0C  movw	$0x0, -0x114(%bp) ; NE 2E10=21:2F5C
21:2E12  movw	$0x2f5c, -0x116(%bp)    # imm = 0x2F5C
21:2E18  movw	-0xc(%bp), %ax
21:2E1B  movw	%ax, -0x124(%bp)
21:2E1F  movw	-0x6(%bp), %ax
21:2E22  subw	-0x2(%bp), %ax
21:2E25  movw	%ax, -0x126(%bp)
21:2E29  movw	$0x0, -0x120(%bp) ; NE 2E2D=21:2F54
21:2E2F  movw	$0x2f54, -0x122(%bp)    # imm = 0x2F54
21:2E35  movw	%ax, -0x11e(%bp)
21:2E39  movw	-0x124(%bp), %ax
21:2E3D  movw	%ax, -0x11c(%bp)
21:2E41  movw	$0x0, -0x120(%bp) ; NE 2E45=21:2F5C
21:2E47  movw	$0x2f5c, -0x122(%bp)    # imm = 0x2F5C
21:2E4D  movw	$0x1, -0xc2(%bp)
21:2E53  jmp	0x2ed9 <.text+0x2ed9>
21:2E56  pushw	-0xc2(%bp)
21:2E5A  pushw	%ss
21:2E5B  leaw	-0x116(%bp), %ax
21:2E5F  pushw	%ax
21:2E60  pushw	%cs
21:2E61  callw	0x2813 <.text+0x2813>
21:2E64  addw	$0x6, %sp
21:2E67  addw	%ax, -0x4(%bp)
21:2E6A  pushw	-0xc2(%bp)
21:2E6E  pushw	%ss
21:2E6F  leaw	-0x122(%bp), %ax
21:2E73  pushw	%ax
21:2E74  pushw	%cs
21:2E75  callw	0x2813 <.text+0x2813>
21:2E78  addw	$0x6, %sp
21:2E7B  addw	%ax, -0x2(%bp)
21:2E7E  pushl	-0x4(%bp)
21:2E82  lesw	0x6(%bp), %bx
21:2E85  movw	%es:0x48(%bx), %ax
21:2E89  movw	%ax, -0x42(%bp)
21:2E8C  movw	%es:0x46(%bx), %dx
21:2E90  movw	%dx, -0x44(%bp)
21:2E93  pushw	%ax
21:2E94  pushw	%dx
21:2E95  lesw	-0x44(%bp), %bx
21:2E98  lesw	%es:(%bx), %bx
21:2E9B  lcallw	*%es:0x14(%bx)
21:2E9F  addw	$0x8, %sp
21:2EA2  pushl	$0x32
21:2EA5  pushl	0x6(%bp)
21:2EA9  lcallw	$0x0, $0xffff           # imm = 0xFFFF ; NE 2EAA=37:1971
21:2EAE  addw	$0x8, %sp
21:2EB1  orw	%ax, %ax
21:2EB3  jne	0x2ed5 <.text+0x2ed5>
21:2EB5  pushw	%ss
21:2EB6  leaw	-0x158(%bp), %ax
21:2EBA  pushw	%ax
21:2EBB  lcallw	$0x0, $0xffff           # imm = 0xFFFF ; NE 2EBC=36:2A8F
21:2EC0  addw	$0x4, %sp
21:2EC3  pushw	$0x2
21:2EC5  pushw	%ss
21:2EC6  leaw	-0x158(%bp), %ax
21:2ECA  pushw	%ax
21:2ECB  lcallw	$0x0, $0xffff           # imm = 0xFFFF ; NE 2ECC=36:285B
21:2ED0  addw	$0x6, %sp
21:2ED3  jmp	0x2f4f <.text+0x2f4f>
21:2ED5  incw	-0xc2(%bp)
21:2ED9  movw	-0xc2(%bp), %ax
21:2EDD  cmpw	-0xc(%bp), %ax
21:2EE0  jle	0x2e56 <.text+0x2e56>
21:2EE4  pushl	-0x10(%bp)
21:2EE8  lesw	0x6(%bp), %bx
21:2EEB  movw	%es:0x48(%bx), %ax
21:2EEF  movw	%ax, -0x46(%bp)
21:2EF2  movw	%es:0x46(%bx), %dx
21:2EF6  movw	%dx, -0x48(%bp)
21:2EF9  pushw	%ax
21:2EFA  pushw	%dx
21:2EFB  lesw	-0x48(%bp), %bx
21:2EFE  lesw	%es:(%bx), %bx
21:2F01  lcallw	*%es:0x14(%bx)
21:2F05  addw	$0x8, %sp
21:2F08  lesw	0x6(%bp), %bx
21:2F0B  movl	$0x0, %es:0x46(%bx)
21:2F14  movl	$0x0, %es:0x4e(%bx)
21:2F1D  pushl	%es:0x42(%bx)
21:2F22  nop
21:2F23  pushw	%cs
21:2F24  callw	0x1e62 <.text+0x1e62>
21:2F27  addw	$0x4, %sp
21:2F2A  pushw	$0x2
21:2F2C  pushw	%ss
21:2F2D  leaw	-0x158(%bp), %ax
21:2F31  pushw	%ax
21:2F32  lcallw	$0x0, $0xffff           # imm = 0xFFFF ; NE 2F33=36:285B
21:2F37  addw	$0x6, %sp
21:2F3A  lesw	0x6(%bp), %bx
21:2F3D  cmpl	$0x0, %es:0x4e(%bx)
21:2F43  je	0x2f4f <.text+0x2f4f>
21:2F45  cmpl	$0x0, %es:0x46(%bx)
21:2F4B  jne	0x29a7 <.text+0x29a7>
21:2F4F  popw	%di
21:2F50  popw	%si
21:2F51  leave
21:2F52  decw	%bp
21:2F53  lretw
```

### DD: decisões espaciais auxiliares (26:1C12-2033)

```asm
26:1C12  incw	%bp
26:1C13  pushw	%bp
26:1C14  movw	%sp, %bp
26:1C16  pushw	%si
26:1C17  pushw	%di
26:1C18  lesw	0x6(%bp), %bx
26:1C1B  movw	0xa(%bp), %ax
26:1C1E  movw	%ax, %es:0xf6(%bx)
26:1C23  movw	0xc(%bp), %ax
26:1C26  movw	%ax, %es:0xf8(%bx)
26:1C2B  movw	0xe(%bp), %ax
26:1C2E  movw	%ax, %es:0xfa(%bx)
26:1C33  movw	0x10(%bp), %ax
26:1C36  movw	%ax, %es:0xfc(%bx)
26:1C3B  popw	%di
26:1C3C  popw	%si
26:1C3D  popw	%bp
26:1C3E  decw	%bp
26:1C3F  lretw
26:1C40  incw	%bp
26:1C41  pushw	%bp
26:1C42  movw	%sp, %bp
26:1C44  subw	$0xc, %sp
26:1C47  pushw	%si
26:1C48  pushw	%di
26:1C49  movw	$0xffff, -0x2(%bp)      # imm = 0xFFFF
26:1C4E  movw	$0x270f, -0x4(%bp)      # imm = 0x270F
26:1C53  movw	$0x0, -0x6(%bp)
26:1C58  movw	0x6(%bp), %ax
26:1C5B  movw	%ax, -0x8(%bp)
26:1C5E  jmp	0x1cb0 <.text+0x1cb0>
26:1C60  movw	-0x6(%bp), %ax
26:1C63  cmpw	0xa(%bp), %ax
26:1C66  je	0x1ca9 <.text+0x1ca9>
26:1C68  movw	0x8(%bp), %es
26:1C6B  movw	-0x8(%bp), %bx
26:1C6E  lesw	%es:0x5c(%bx), %bx
26:1C72  cmpw	$0x3, %es:0x1a(%bx)
26:1C77  je	0x1ca9 <.text+0x1ca9>
26:1C79  pushw	%ss
26:1C7A  leaw	-0xc(%bp), %ax
26:1C7D  pushw	%ax
26:1C7E  pushw	%ss
26:1C7F  leaw	-0xa(%bp), %ax
26:1C82  pushw	%ax
26:1C83  pushw	0xa(%bp)
26:1C86  pushw	-0x6(%bp)
26:1C89  pushl	0x6(%bp)
26:1C8D  nop
26:1C8E  pushw	%cs
26:1C8F  callw	0x1cc4 <.text+0x1cc4>
26:1C92  addw	$0x10, %sp
26:1C95  movw	-0xa(%bp), %dx
26:1C98  addw	-0xc(%bp), %dx
26:1C9B  cmpw	-0x4(%bp), %dx
26:1C9E  jge	0x1ca9 <.text+0x1ca9>
26:1CA0  movw	%dx, -0x4(%bp)
26:1CA3  movw	-0x6(%bp), %ax
26:1CA6  movw	%ax, -0x2(%bp)
26:1CA9  addw	$0x4, -0x8(%bp)
26:1CAD  incw	-0x6(%bp)
26:1CB0  lesw	0x6(%bp), %bx
26:1CB3  movw	%es:0x44(%bx), %ax
26:1CB7  cmpw	-0x6(%bp), %ax
26:1CBA  jg	0x1c60 <.text+0x1c60>
26:1CBC  movw	-0x2(%bp), %ax
26:1CBF  popw	%di
26:1CC0  popw	%si
26:1CC1  leave
26:1CC2  decw	%bp
26:1CC3  lretw
26:1CC4  incw	%bp
26:1CC5  pushw	%bp
26:1CC6  movw	%sp, %bp
26:1CC8  subw	$0x8, %sp
26:1CCB  pushw	%si
26:1CCC  pushw	%di
26:1CCD  movw	0xa(%bp), %ax
26:1CD0  shlw	$0x2, %ax
26:1CD3  lesw	0x6(%bp), %bx
26:1CD6  addw	%ax, %bx
26:1CD8  lesw	%es:0x5c(%bx), %bx
26:1CDC  movl	%es:0xc(%bx), %eax
26:1CE1  movl	%eax, -0x4(%bp)
26:1CE5  movw	0xc(%bp), %ax
26:1CE8  shlw	$0x2, %ax
26:1CEB  lesw	0x6(%bp), %bx
26:1CEE  addw	%ax, %bx
26:1CF0  lesw	%es:0x5c(%bx), %bx
26:1CF4  movl	%es:0xc(%bx), %eax
26:1CF9  movl	%eax, -0x8(%bp)
26:1CFD  movw	-0x4(%bp), %ax
26:1D00  subw	-0x8(%bp), %ax
26:1D03  lesw	0xe(%bp), %bx
26:1D06  movw	%ax, %es:(%bx)
26:1D09  movw	-0x2(%bp), %ax
26:1D0C  subw	-0x6(%bp), %ax
26:1D0F  lesw	0x12(%bp), %bx
26:1D12  movw	%ax, %es:(%bx)
26:1D15  lesw	0xe(%bp), %bx
26:1D18  cmpw	$0x0, %es:(%bx)
26:1D1C  jge	0x1d26 <.text+0x1d26>
26:1D1E  movw	%es:(%bx), %ax
26:1D21  negw	%ax
26:1D23  movw	%ax, %es:(%bx)
26:1D26  lesw	0x12(%bp), %bx
26:1D29  cmpw	$0x0, %es:(%bx)
26:1D2D  jge	0x1d37 <.text+0x1d37>
26:1D2F  movw	%es:(%bx), %ax
26:1D32  negw	%ax
26:1D34  movw	%ax, %es:(%bx)
26:1D37  popw	%di
26:1D38  popw	%si
26:1D39  leave
26:1D3A  decw	%bp
26:1D3B  lretw
26:1D3C  incw	%bp
26:1D3D  pushw	%bp
26:1D3E  movw	%sp, %bp
26:1D40  subw	$0xc, %sp
26:1D43  pushw	%si
26:1D44  pushw	%di
26:1D45  movw	0xa(%bp), %ax
26:1D48  shlw	$0x2, %ax
26:1D4B  lesw	0x6(%bp), %bx
26:1D4E  addw	%ax, %bx
26:1D50  movw	%es:0x5e(%bx), %ax
26:1D54  movw	%es:0x5c(%bx), %dx
26:1D58  movw	%ax, -0x2(%bp)
26:1D5B  movw	%dx, -0x4(%bp)
26:1D5E  lesw	-0x4(%bp), %bx
26:1D61  movl	%es:0xc(%bx), %eax
26:1D66  movl	%eax, -0x8(%bp)
26:1D6A  xorw	%cx, %cx
26:1D6C  movw	$0x0, -0xa(%bp)
26:1D71  xorw	%dx, %dx
26:1D73  xorw	%bx, %bx
26:1D75  movw	-0x8(%bp), %ax
26:1D78  subw	0xc(%bp), %ax
26:1D7B  cmpw	$0x14, %ax
26:1D7E  jle	0x1d83 <.text+0x1d83>
26:1D80  movw	$0x1, %bx
26:1D83  movw	0xc(%bp), %ax
26:1D86  subw	-0x8(%bp), %ax
26:1D89  cmpw	$0x14, %ax
26:1D8C  jle	0x1d91 <.text+0x1d91>
26:1D8E  movw	$0x1, %dx
26:1D91  movw	-0x6(%bp), %ax
26:1D94  subw	0xe(%bp), %ax
26:1D97  cmpw	$0xf, %ax
26:1D9A  jle	0x1d9f <.text+0x1d9f>
26:1D9C  movw	$0x1, %cx
26:1D9F  movw	0xe(%bp), %ax
26:1DA2  subw	-0x6(%bp), %ax
26:1DA5  cmpw	$0xf, %ax
26:1DA8  jle	0x1daf <.text+0x1daf>
26:1DAA  movw	$0x1, -0xa(%bp)
26:1DAF  cmpw	$0x0, 0x10(%bp)
26:1DB3  je	0x1dc9 <.text+0x1dc9>
26:1DB5  movw	%bx, -0xc(%bp)
26:1DB8  movw	%dx, %bx
26:1DBA  movw	-0xc(%bp), %dx
26:1DBD  movw	%cx, -0xc(%bp)
26:1DC0  movw	-0xa(%bp), %cx
26:1DC3  movw	-0xc(%bp), %ax
26:1DC6  movw	%ax, -0xa(%bp)
26:1DC9  orw	%cx, %cx
26:1DCB  je	0x1de1 <.text+0x1de1>
26:1DCD  orw	%bx, %bx
26:1DCF  je	0x1dd5 <.text+0x1dd5>
26:1DD1  pushw	$0x7
26:1DD3  jmp	0x1e09 <.text+0x1e09>
26:1DD5  orw	%dx, %dx
26:1DD7  je	0x1ddd <.text+0x1ddd>
26:1DD9  pushw	$0x1
26:1DDB  jmp	0x1e09 <.text+0x1e09>
26:1DDD  pushw	$0x0
26:1DDF  jmp	0x1e09 <.text+0x1e09>
26:1DE1  cmpw	$0x0, -0xa(%bp)
26:1DE5  je	0x1dfb <.text+0x1dfb>
26:1DE7  orw	%bx, %bx
26:1DE9  je	0x1def <.text+0x1def>
26:1DEB  pushw	$0x5
26:1DED  jmp	0x1e09 <.text+0x1e09>
26:1DEF  orw	%dx, %dx
26:1DF1  je	0x1df7 <.text+0x1df7>
26:1DF3  pushw	$0x3
26:1DF5  jmp	0x1e09 <.text+0x1e09>
26:1DF7  pushw	$0x4
26:1DF9  jmp	0x1e09 <.text+0x1e09>
26:1DFB  orw	%bx, %bx
26:1DFD  je	0x1e03 <.text+0x1e03>
26:1DFF  pushw	$0x6
26:1E01  jmp	0x1e09 <.text+0x1e09>
26:1E03  orw	%dx, %dx
26:1E05  je	0x1e14 <.text+0x1e14>
26:1E07  pushw	$0x2
26:1E09  pushl	-0x4(%bp)
26:1E0D  pushw	%cs
26:1E0E  callw	0x984 <.text+0x984>
26:1E11  addw	$0x6, %sp
26:1E14  popw	%di
26:1E15  popw	%si
26:1E16  leave
26:1E17  decw	%bp
26:1E18  lretw
26:1E19  incw	%bp
26:1E1A  pushw	%bp
26:1E1B  movw	%sp, %bp
26:1E1D  subw	$0x6, %sp
26:1E20  pushw	%si
26:1E21  pushw	%di
26:1E22  movw	$0xffff, -0x2(%bp)      # imm = 0xFFFF
26:1E27  xorw	%dx, %dx
26:1E29  movw	0x6(%bp), %cx
26:1E2C  jmp	0x1e46 <.text+0x1e46>
26:1E2E  movw	0x8(%bp), %es
26:1E31  movw	%cx, %bx
26:1E33  lesw	%es:0x5c(%bx), %bx
26:1E37  cmpw	$0x0, %es:(%bx)
26:1E3B  je	0x1e42 <.text+0x1e42>
26:1E3D  movw	%dx, -0x2(%bp)
26:1E40  jmp	0x1e4f <.text+0x1e4f>
26:1E42  addw	$0x4, %cx
26:1E45  incw	%dx
26:1E46  lesw	0x6(%bp), %bx
26:1E49  cmpw	%dx, %es:0x44(%bx)
26:1E4D  jg	0x1e2e <.text+0x1e2e>
26:1E4F  movw	-0x2(%bp), %ax
26:1E52  shlw	$0x2, %ax
26:1E55  lesw	0x6(%bp), %bx
26:1E58  addw	%ax, %bx
26:1E5A  lesw	%es:0x5c(%bx), %bx
26:1E5E  movl	%es:0xc(%bx), %eax
26:1E63  movl	%eax, -0x6(%bp)
26:1E67  pushw	$0x0
26:1E69  pushl	%eax
26:1E6B  pushw	0xa(%bp)
26:1E6E  pushl	0x6(%bp)
26:1E72  pushw	%cs
26:1E73  callw	0x1d3c <.text+0x1d3c>
26:1E76  addw	$0xc, %sp
26:1E79  popw	%di
26:1E7A  popw	%si
26:1E7B  leave
26:1E7C  decw	%bp
26:1E7D  lretw
26:1E7E  incw	%bp
26:1E7F  pushw	%bp
26:1E80  movw	%sp, %bp
26:1E82  subw	$0x4, %sp
26:1E85  pushw	%si
26:1E86  pushw	%di
26:1E87  pushw	0xa(%bp)
26:1E8A  pushl	0x6(%bp)
26:1E8E  pushw	%cs
26:1E8F  callw	0x1c40 <.text+0x1c40>
26:1E92  addw	$0x6, %sp
26:1E95  movw	%ax, %dx
26:1E97  orw	%dx, %dx
26:1E99  jle	0x1ec2 <.text+0x1ec2>
26:1E9B  shlw	$0x2, %ax
26:1E9E  lesw	0x6(%bp), %bx
26:1EA1  addw	%ax, %bx
26:1EA3  lesw	%es:0x5c(%bx), %bx
26:1EA7  movl	%es:0xc(%bx), %eax
26:1EAC  movl	%eax, -0x4(%bp)
26:1EB0  pushw	$0x0
26:1EB2  pushl	%eax
26:1EB4  pushw	0xa(%bp)
26:1EB7  pushl	0x6(%bp)
26:1EBB  pushw	%cs
26:1EBC  callw	0x1d3c <.text+0x1d3c>
26:1EBF  addw	$0xc, %sp
26:1EC2  popw	%di
26:1EC3  popw	%si
26:1EC4  leave
26:1EC5  decw	%bp
26:1EC6  lretw
26:1EC7  incw	%bp
26:1EC8  pushw	%bp
26:1EC9  movw	%sp, %bp
26:1ECB  pushw	%si
26:1ECC  pushw	%di
26:1ECD  lesw	0x6(%bp), %bx
26:1ED0  cmpw	$0x0, %es:0x2(%bx)
26:1ED5  je	0x1f0b <.text+0x1f0b>
26:1ED7  lesw	0x6(%bp), %bx
26:1EDA  cmpw	$0x7, %es:0x2(%bx)
26:1EDF  je	0x1ef6 <.text+0x1ef6>
26:1EE1  cmpw	$0x6, %es:0x2(%bx)
26:1EE6  je	0x1ef6 <.text+0x1ef6>
26:1EE8  cmpw	$0x5, %es:0x2(%bx)
26:1EED  je	0x1ef6 <.text+0x1ef6>
26:1EEF  cmpw	$0x4, %es:0x2(%bx)
26:1EF4  jne	0x1f00 <.text+0x1f00>
26:1EF6  pushl	0x6(%bp)
26:1EFA  pushw	%cs
26:1EFB  callw	0x7be <.text+0x7be>
26:1EFE  jmp	0x1f08 <.text+0x1f08>
26:1F00  pushl	0x6(%bp)
26:1F04  pushw	%cs
26:1F05  callw	0x78b <.text+0x78b>
26:1F08  addw	$0x4, %sp
26:1F0B  popw	%di
26:1F0C  popw	%si
26:1F0D  popw	%bp
26:1F0E  decw	%bp
26:1F0F  lretw
26:1F10  incw	%bp
26:1F11  pushw	%bp
26:1F12  movw	%sp, %bp
26:1F14  pushw	%si
26:1F15  pushw	%di
26:1F16  lesw	0x6(%bp), %bx
26:1F19  cmpw	$0x4, %es:0x2(%bx)
26:1F1E  je	0x1f54 <.text+0x1f54>
26:1F20  lesw	0x6(%bp), %bx
26:1F23  cmpw	$0x3, %es:0x2(%bx)
26:1F28  je	0x1f3f <.text+0x1f3f>
26:1F2A  cmpw	$0x2, %es:0x2(%bx)
26:1F2F  je	0x1f3f <.text+0x1f3f>
26:1F31  cmpw	$0x1, %es:0x2(%bx)
26:1F36  je	0x1f3f <.text+0x1f3f>
26:1F38  cmpw	$0x0, %es:0x2(%bx)
26:1F3D  jne	0x1f49 <.text+0x1f49>
26:1F3F  pushl	0x6(%bp)
26:1F43  pushw	%cs
26:1F44  callw	0x7be <.text+0x7be>
26:1F47  jmp	0x1f51 <.text+0x1f51>
26:1F49  pushl	0x6(%bp)
26:1F4D  pushw	%cs
26:1F4E  callw	0x78b <.text+0x78b>
26:1F51  addw	$0x4, %sp
26:1F54  popw	%di
26:1F55  popw	%si
26:1F56  popw	%bp
26:1F57  decw	%bp
26:1F58  lretw
26:1F59  incw	%bp
26:1F5A  pushw	%bp
26:1F5B  movw	%sp, %bp
26:1F5D  pushw	%si
26:1F5E  pushw	%di
26:1F5F  lesw	0x6(%bp), %bx
26:1F62  cmpw	$0x2, %es:0x2(%bx)
26:1F67  je	0x1f9d <.text+0x1f9d>
26:1F69  lesw	0x6(%bp), %bx
26:1F6C  cmpw	$0x6, %es:0x2(%bx)
26:1F71  je	0x1f88 <.text+0x1f88>
26:1F73  cmpw	$0x7, %es:0x2(%bx)
26:1F78  je	0x1f88 <.text+0x1f88>
26:1F7A  cmpw	$0x0, %es:0x2(%bx)
26:1F7F  je	0x1f88 <.text+0x1f88>
26:1F81  cmpw	$0x1, %es:0x2(%bx)
26:1F86  jne	0x1f92 <.text+0x1f92>
26:1F88  pushl	0x6(%bp)
26:1F8C  pushw	%cs
26:1F8D  callw	0x7be <.text+0x7be>
26:1F90  jmp	0x1f9a <.text+0x1f9a>
26:1F92  pushl	0x6(%bp)
26:1F96  pushw	%cs
26:1F97  callw	0x78b <.text+0x78b>
26:1F9A  addw	$0x4, %sp
26:1F9D  popw	%di
26:1F9E  popw	%si
26:1F9F  popw	%bp
26:1FA0  decw	%bp
26:1FA1  lretw
26:1FA2  incw	%bp
26:1FA3  pushw	%bp
26:1FA4  movw	%sp, %bp
26:1FA6  pushw	%si
26:1FA7  pushw	%di
26:1FA8  lesw	0x6(%bp), %bx
26:1FAB  cmpw	$0x6, %es:0x2(%bx)
26:1FB0  je	0x1fe6 <.text+0x1fe6>
26:1FB2  lesw	0x6(%bp), %bx
26:1FB5  cmpw	$0x2, %es:0x2(%bx)
26:1FBA  je	0x1fd1 <.text+0x1fd1>
26:1FBC  cmpw	$0x3, %es:0x2(%bx)
26:1FC1  je	0x1fd1 <.text+0x1fd1>
26:1FC3  cmpw	$0x4, %es:0x2(%bx)
26:1FC8  je	0x1fd1 <.text+0x1fd1>
26:1FCA  cmpw	$0x5, %es:0x2(%bx)
26:1FCF  jne	0x1fdb <.text+0x1fdb>
26:1FD1  pushl	0x6(%bp)
26:1FD5  pushw	%cs
26:1FD6  callw	0x7be <.text+0x7be>
26:1FD9  jmp	0x1fe3 <.text+0x1fe3>
26:1FDB  pushl	0x6(%bp)
26:1FDF  pushw	%cs
26:1FE0  callw	0x78b <.text+0x78b>
26:1FE3  addw	$0x4, %sp
26:1FE6  popw	%di
26:1FE7  popw	%si
26:1FE8  popw	%bp
26:1FE9  decw	%bp
26:1FEA  lretw
26:1FEB  incw	%bp
26:1FEC  pushw	%bp
26:1FED  movw	%sp, %bp
26:1FEF  subw	$0x4, %sp
26:1FF2  pushw	%si
26:1FF3  pushw	%di
26:1FF4  pushw	0xa(%bp)
26:1FF7  pushl	0x6(%bp)
26:1FFB  pushw	%cs
26:1FFC  callw	0x1c40 <.text+0x1c40>
26:1FFF  addw	$0x6, %sp
26:2002  movw	%ax, %dx
26:2004  orw	%dx, %dx
26:2006  jle	0x202f <.text+0x202f>
26:2008  shlw	$0x2, %ax
26:200B  lesw	0x6(%bp), %bx
26:200E  addw	%ax, %bx
26:2010  lesw	%es:0x5c(%bx), %bx
26:2014  movl	%es:0xc(%bx), %eax
26:2019  movl	%eax, -0x4(%bp)
26:201D  pushw	$0x1
26:201F  pushl	%eax
26:2021  pushw	0xa(%bp)
26:2024  pushl	0x6(%bp)
26:2028  pushw	%cs
26:2029  callw	0x1d3c <.text+0x1d3c>
26:202C  addw	$0xc, %sp
26:202F  popw	%di
26:2030  popw	%si
26:2031  leave
26:2032  decw	%bp
26:2033  lretw
```

### DD: colisões e respostas (26:2034-2F69)

```asm
26:2034  incw	%bp
26:2035  pushw	%bp
26:2036  movw	%sp, %bp
26:2038  subw	$0x68, %sp
26:203B  pushw	%si
26:203C  pushw	%di
26:203D  movw	$0x0, -0x2(%bp)
26:2042  movw	0x6(%bp), %ax
26:2045  movw	%ax, -0x1e(%bp)
26:2048  jmp	0x23d3 <.text+0x23d3>
26:204B  movw	0x8(%bp), %es
26:204E  movw	-0x1e(%bp), %bx
26:2051  lesw	%es:0x5c(%bx), %bx
26:2055  cmpw	$0x3, %es:0x1a(%bx)
26:205A  je	0x23cc <.text+0x23cc>
26:205E  lesw	0x6(%bp), %bx
26:2061  cmpl	$0x0, %es:0xb8(%bx)
26:2068  je	0x2199 <.text+0x2199>
26:206C  lesw	%es:0xb8(%bx), %bx
26:2071  cmpw	$0x0, %es:0x10(%bx)
26:2076  jne	0x207d <.text+0x207d>
26:2078  movw	$0x1, %ax
26:207B  jmp	0x207f <.text+0x207f>
26:207D  xorw	%ax, %ax
26:207F  orw	%ax, %ax
26:2081  je	0x2199 <.text+0x2199>
26:2085  movw	0x8(%bp), %es
26:2088  movw	-0x1e(%bp), %bx
26:208B  pushl	%es:0x5c(%bx)
26:2090  pushw	%ss
26:2091  leaw	-0xa(%bp), %ax
26:2094  pushw	%ax
26:2095  pushw	%cs
26:2096  callw	0x752 <.text+0x752>
26:2099  addw	$0x8, %sp
26:209C  leaw	-0xa(%bp), %ax
26:209F  movw	%ss, -0xc(%bp)
26:20A2  movw	%ax, -0xe(%bp)
26:20A5  lesw	0x6(%bp), %bx
26:20A8  movw	%es:0x48(%bx), %ax
26:20AC  lesw	-0xe(%bp), %bx
26:20AF  cmpw	%es:0x4(%bx), %ax
26:20B3  jge	0x20e9 <.text+0x20e9>
26:20B5  lesw	0x6(%bp), %bx
26:20B8  movw	%es:0x4c(%bx), %ax
26:20BC  lesw	-0xe(%bp), %bx
26:20BF  cmpw	%es:(%bx), %ax
26:20C2  jle	0x20e9 <.text+0x20e9>
26:20C4  lesw	0x6(%bp), %bx
26:20C7  movw	%es:0x4a(%bx), %ax
26:20CB  lesw	-0xe(%bp), %bx
26:20CE  cmpw	%es:0x6(%bx), %ax
26:20D2  jge	0x20e9 <.text+0x20e9>
26:20D4  lesw	0x6(%bp), %bx
26:20D7  movw	%es:0x4e(%bx), %ax
26:20DB  lesw	-0xe(%bp), %bx
26:20DE  cmpw	%es:0x2(%bx), %ax
26:20E2  jle	0x20e9 <.text+0x20e9>
26:20E4  movw	$0x1, %ax
26:20E7  jmp	0x20eb <.text+0x20eb>
26:20E9  xorw	%ax, %ax
26:20EB  orw	%ax, %ax
26:20ED  je	0x2199 <.text+0x2199>
26:20F1  movw	0x8(%bp), %es
26:20F4  movw	-0x1e(%bp), %bx
26:20F7  pushl	%es:0x5c(%bx)
26:20FC  pushw	%ss
26:20FD  leaw	-0x2e(%bp), %ax
26:2100  pushw	%ax
26:2101  pushw	%cs
26:2102  callw	0x752 <.text+0x752>
26:2105  addw	$0x8, %sp
26:2108  leaw	-0x36(%bp), %ax
26:210B  pushw	%ss
26:210C  pushw	%ax
26:210D  leaw	-0x2e(%bp), %ax
26:2110  pushw	%ss
26:2111  pushw	%ax
26:2112  movw	$0x8, %cx
26:2115  lcallw	$0x0, $0xffff           # imm = 0xFFFF ; NE 2116=1:3E64
26:211A  leaw	-0x3e(%bp), %ax
26:211D  pushw	%ss
26:211E  pushw	%ax
26:211F  leaw	-0x2e(%bp), %ax
26:2122  pushw	%ss
26:2123  pushw	%ax
26:2124  movw	$0x8, %cx
26:2127  lcallw	$0x0, $0xffff           # imm = 0xFFFF ; NE 2128=1:3E64
26:212C  pushw	-0x2(%bp)
26:212F  pushw	%ss
26:2130  leaw	-0x3e(%bp), %ax
26:2133  pushw	%ax
26:2134  pushw	%ss
26:2135  leaw	-0x36(%bp), %ax
26:2138  pushw	%ax
26:2139  pushl	0x6(%bp)
26:213D  nop
26:213E  pushw	%cs
26:213F  callw	0x291d <.text+0x291d>
26:2142  addw	$0xe, %sp
26:2145  leaw	-0x3e(%bp), %ax
26:2148  movw	%ss, %dx
26:214A  movw	$0x8, %cx
26:214D  lcallw	$0x0, $0xffff           # imm = 0xFFFF ; NE 214E=1:3E86
26:2152  leaw	-0x36(%bp), %ax
26:2155  movw	%ss, %dx
26:2157  movw	$0x8, %cx
26:215A  lcallw	$0x0, $0xffff           # imm = 0xFFFF ; NE 215B=1:3E86
26:215F  lesw	0x6(%bp), %bx
26:2162  pushl	%es:0xb8(%bx)
26:2168  nop
26:2169  pushw	%cs
26:216A  callw	0x2e8e <.text+0x2e8e>
26:216D  addw	$0x14, %sp
26:2170  orw	%ax, %ax
26:2172  je	0x2199 <.text+0x2199>
26:2174  lesw	0x6(%bp), %bx
26:2177  pushl	%es:0xb8(%bx)
26:217D  nop
26:217E  pushw	%cs
26:217F  callw	0x2e33 <.text+0x2e33>
26:2182  addw	$0x4, %sp
26:2185  pushw	$0x64
26:2187  movw	0x8(%bp), %es
26:218A  movw	-0x1e(%bp), %bx
26:218D  pushl	%es:0x5c(%bx)
26:2192  pushw	%cs
26:2193  callw	0x4d4 <.text+0x4d4>
26:2196  addw	$0x6, %sp
26:2199  movw	0x8(%bp), %es
26:219C  movw	-0x1e(%bp), %bx
26:219F  pushl	%es:0x5c(%bx)
26:21A4  pushw	%ss
26:21A5  leaw	-0x16(%bp), %ax
26:21A8  pushw	%ax
26:21A9  pushw	%cs
26:21AA  callw	0x752 <.text+0x752>
26:21AD  addw	$0x8, %sp
26:21B0  leaw	-0x16(%bp), %ax
26:21B3  movw	%ss, -0x18(%bp)
26:21B6  movw	%ax, -0x1a(%bp)
26:21B9  lesw	0x6(%bp), %bx
26:21BC  movw	%es:0x50(%bx), %ax
26:21C0  lesw	-0x1a(%bp), %bx
26:21C3  cmpw	%es:0x4(%bx), %ax
26:21C7  jge	0x21fd <.text+0x21fd>
26:21C9  lesw	0x6(%bp), %bx
26:21CC  movw	%es:0x54(%bx), %ax
26:21D0  lesw	-0x1a(%bp), %bx
26:21D3  cmpw	%es:(%bx), %ax
26:21D6  jle	0x21fd <.text+0x21fd>
26:21D8  lesw	0x6(%bp), %bx
26:21DB  movw	%es:0x52(%bx), %ax
26:21DF  lesw	-0x1a(%bp), %bx
26:21E2  cmpw	%es:0x6(%bx), %ax
26:21E6  jge	0x21fd <.text+0x21fd>
26:21E8  lesw	0x6(%bp), %bx
26:21EB  movw	%es:0x56(%bx), %ax
26:21EF  lesw	-0x1a(%bp), %bx
26:21F2  cmpw	%es:0x2(%bx), %ax
26:21F6  jle	0x21fd <.text+0x21fd>
26:21F8  movw	$0x1, %ax
26:21FB  jmp	0x21ff <.text+0x21ff>
26:21FD  xorw	%ax, %ax
26:21FF  orw	%ax, %ax
26:2201  je	0x2228 <.text+0x2228>
26:2203  movw	0x8(%bp), %es
26:2206  movw	-0x1e(%bp), %bx
26:2209  lesw	%es:0x5c(%bx), %bx
26:220D  cmpw	$0x2, %es:0x8(%bx)
26:2212  jle	0x2228 <.text+0x2228>
26:2214  pushw	$0x2
26:2216  movw	0x8(%bp), %es
26:2219  movw	-0x1e(%bp), %bx
26:221C  pushl	%es:0x5c(%bx)
26:2221  pushw	%cs
26:2222  callw	0x700 <.text+0x700>
26:2225  addw	$0x6, %sp
26:2228  movw	$0x0, -0x28(%bp)
26:222D  movw	0x6(%bp), %ax
26:2230  movw	%ax, -0x1c(%bp)
26:2233  jmp	0x23be <.text+0x23be>
26:2236  movw	0x8(%bp), %es
26:2239  movw	-0x1c(%bp), %bx
26:223C  movw	%es:0x8e(%bx), %ax
26:2241  movw	%ax, -0x24(%bp)
26:2244  movw	%es:0x8c(%bx), %dx
26:2249  movw	%dx, -0x26(%bp)
26:224C  pushw	%ax
26:224D  pushw	%dx
26:224E  lesw	-0x26(%bp), %bx
26:2251  lesw	%es:0x2(%bx), %bx
26:2255  lcallw	*%es:0x4(%bx)
26:2259  addw	$0x4, %sp
26:225C  movw	%dx, -0x2a(%bp)
26:225F  movw	%ax, -0x2c(%bp)
26:2262  movw	-0x2a(%bp), %ax
26:2265  addw	$0x2d, %ax
26:2268  movw	%ax, -0x36(%bp)
26:226B  movw	-0x2c(%bp), %ax
26:226E  addw	$0x36, %ax
26:2271  movw	%ax, -0x38(%bp)
26:2274  movw	-0x2a(%bp), %ax
26:2277  addw	$0x14, %ax
26:227A  movw	%ax, -0x3a(%bp)
26:227D  movw	-0x2c(%bp), %ax
26:2280  addw	$0x9, %ax
26:2283  movw	%ax, -0x3c(%bp)
26:2286  movw	%ax, -0x44(%bp)
26:2289  movw	-0x3a(%bp), %ax
26:228C  movw	%ax, -0x42(%bp)
26:228F  movw	-0x38(%bp), %ax
26:2292  movw	%ax, -0x40(%bp)
26:2295  movw	-0x36(%bp), %ax
26:2298  movw	%ax, -0x3e(%bp)
26:229B  leaw	-0x34(%bp), %ax
26:229E  pushw	%ss
26:229F  pushw	%ax
26:22A0  leaw	-0x44(%bp), %ax
26:22A3  pushw	%ss
26:22A4  pushw	%ax
26:22A5  movw	$0x8, %cx
26:22A8  lcallw	$0x0, $0xffff           # imm = 0xFFFF ; NE 22A9=1:3E64
26:22AD  movw	0x8(%bp), %es
26:22B0  movw	-0x1e(%bp), %bx
26:22B3  pushl	%es:0x5c(%bx)
26:22B8  pushw	%ss
26:22B9  leaw	-0x4c(%bp), %ax
26:22BC  pushw	%ax
26:22BD  pushw	%cs
26:22BE  callw	0x752 <.text+0x752>
26:22C1  addw	$0x8, %sp
26:22C4  leaw	-0x4c(%bp), %ax
26:22C7  movw	%ss, -0x4e(%bp)
26:22CA  movw	%ax, -0x50(%bp)
26:22CD  lesw	-0x50(%bp), %bx
26:22D0  movw	%es:0x4(%bx), %ax
26:22D4  cmpw	-0x34(%bp), %ax
26:22D7  jle	0x22f8 <.text+0x22f8>
26:22D9  movw	%es:(%bx), %ax
26:22DC  cmpw	-0x30(%bp), %ax
26:22DF  jge	0x22f8 <.text+0x22f8>
26:22E1  movw	%es:0x6(%bx), %ax
26:22E5  cmpw	-0x32(%bp), %ax
26:22E8  jle	0x22f8 <.text+0x22f8>
26:22EA  movw	%es:0x2(%bx), %ax
26:22EE  cmpw	-0x2e(%bp), %ax
26:22F1  jge	0x22f8 <.text+0x22f8>
26:22F3  movw	$0x1, %ax
26:22F6  jmp	0x22fa <.text+0x22fa>
26:22F8  xorw	%ax, %ax
26:22FA  orw	%ax, %ax
26:22FC  je	0x23b7 <.text+0x23b7>
26:2300  movw	0x8(%bp), %es
26:2303  movw	-0x1e(%bp), %bx
26:2306  pushl	%es:0x5c(%bx)
26:230B  pushw	%ss
26:230C  leaw	-0x58(%bp), %ax
26:230F  pushw	%ax
26:2310  pushw	%cs
26:2311  callw	0x752 <.text+0x752>
26:2314  addw	$0x8, %sp
26:2317  leaw	-0x60(%bp), %ax
26:231A  pushw	%ss
26:231B  pushw	%ax
26:231C  leaw	-0x58(%bp), %ax
26:231F  pushw	%ss
26:2320  pushw	%ax
26:2321  movw	$0x8, %cx
26:2324  lcallw	$0x0, $0xffff           # imm = 0xFFFF ; NE 2325=1:3E64
26:2329  leaw	-0x68(%bp), %ax
26:232C  pushw	%ss
26:232D  pushw	%ax
26:232E  leaw	-0x58(%bp), %ax
26:2331  pushw	%ss
26:2332  pushw	%ax
26:2333  movw	$0x8, %cx
26:2336  lcallw	$0x0, $0xffff           # imm = 0xFFFF ; NE 2337=1:3E64
26:233B  pushw	-0x2(%bp)
26:233E  pushw	%ss
26:233F  leaw	-0x68(%bp), %ax
26:2342  pushw	%ax
26:2343  pushw	%ss
26:2344  leaw	-0x60(%bp), %ax
26:2347  pushw	%ax
26:2348  pushl	0x6(%bp)
26:234C  nop
26:234D  pushw	%cs
26:234E  callw	0x291d <.text+0x291d>
26:2351  addw	$0xe, %sp
26:2354  movw	-0x34(%bp), %ax
26:2357  cmpw	-0x5c(%bp), %ax
26:235A  jge	0x2379 <.text+0x2379>
26:235C  movw	-0x30(%bp), %ax
26:235F  cmpw	-0x60(%bp), %ax
26:2362  jle	0x2379 <.text+0x2379>
26:2364  movw	-0x32(%bp), %ax
26:2367  cmpw	-0x5a(%bp), %ax
26:236A  jge	0x2379 <.text+0x2379>
26:236C  movw	-0x2e(%bp), %ax
26:236F  cmpw	-0x5e(%bp), %ax
26:2372  jle	0x2379 <.text+0x2379>
26:2374  movw	$0x1, %ax
26:2377  jmp	0x237b <.text+0x237b>
26:2379  xorw	%ax, %ax
26:237B  orw	%ax, %ax
26:237D  je	0x23b7 <.text+0x23b7>
26:237F  movw	0x8(%bp), %es
26:2382  movw	-0x1e(%bp), %bx
26:2385  pushl	%es:0x5c(%bx)
26:238A  pushw	%cs
26:238B  callw	0x99f <.text+0x99f>
26:238E  addw	$0x4, %sp
26:2391  pushw	$0x0
26:2393  movw	0x8(%bp), %es
26:2396  movw	-0x1e(%bp), %bx
26:2399  pushl	%es:0x5c(%bx)
26:239E  pushw	%cs
26:239F  callw	0x700 <.text+0x700>
26:23A2  addw	$0x6, %sp
26:23A5  movw	0x8(%bp), %es
26:23A8  movw	-0x1e(%bp), %bx
26:23AB  pushl	%es:0x5c(%bx)
26:23B0  pushw	%cs
26:23B1  callw	0x382 <.text+0x382>
26:23B4  addw	$0x4, %sp
26:23B7  addw	$0x4, -0x1c(%bp)
26:23BB  incw	-0x28(%bp)
26:23BE  lesw	0x6(%bp), %bx
26:23C1  movw	%es:0x46(%bx), %ax
26:23C5  cmpw	-0x28(%bp), %ax
26:23C8  jg	0x2236 <.text+0x2236>
26:23CC  addw	$0x4, -0x1e(%bp)
26:23D0  incw	-0x2(%bp)
26:23D3  lesw	0x6(%bp), %bx
26:23D6  movw	%es:0x44(%bx), %ax
26:23DA  cmpw	-0x2(%bp), %ax
26:23DD  jg	0x204b <.text+0x204b>
26:23E1  movw	$0x0, -0x2(%bp)
26:23E6  movw	0x6(%bp), %ax
26:23E9  movw	%ax, -0x22(%bp)
26:23EC  jmp	0x24bb <.text+0x24bb>
26:23EF  movw	-0x2(%bp), %ax
26:23F2  incw	%ax
26:23F3  movw	%ax, -0x28(%bp)
26:23F6  movw	0x6(%bp), %ax
26:23F9  movw	-0x28(%bp), %dx
26:23FC  shlw	$0x2, %dx
26:23FF  addw	%dx, %ax
26:2401  movw	%ax, -0x20(%bp)
26:2404  jmp	0x24a6 <.text+0x24a6>
26:2407  movw	0x8(%bp), %es
26:240A  movw	-0x20(%bp), %bx
26:240D  pushl	%es:0x5c(%bx)
26:2412  pushw	%ss
26:2413  leaw	-0x30(%bp), %ax
26:2416  pushw	%ax
26:2417  pushw	%cs
26:2418  callw	0x752 <.text+0x752>
26:241B  addw	$0x8, %sp
26:241E  leaw	-0x30(%bp), %ax
26:2421  movw	%ss, -0x32(%bp)
26:2424  movw	%ax, -0x34(%bp)
26:2427  movw	0x8(%bp), %es
26:242A  movw	-0x22(%bp), %bx
26:242D  pushl	%es:0x5c(%bx)
26:2432  pushw	%ss
26:2433  leaw	-0x3c(%bp), %ax
26:2436  pushw	%ax
26:2437  pushw	%cs
26:2438  callw	0x752 <.text+0x752>
26:243B  addw	$0x8, %sp
26:243E  leaw	-0x3c(%bp), %ax
26:2441  movw	%ss, -0x3e(%bp)
26:2444  movw	%ax, -0x40(%bp)
26:2447  lesw	-0x40(%bp), %bx
26:244A  movw	%es:0x4(%bx), %ax
26:244E  lesw	-0x34(%bp), %bx
26:2451  cmpw	%es:(%bx), %ax
26:2454  jle	0x2487 <.text+0x2487>
26:2456  movw	%es:0x4(%bx), %ax
26:245A  lesw	-0x40(%bp), %bx
26:245D  cmpw	%es:(%bx), %ax
26:2460  jle	0x2487 <.text+0x2487>
26:2462  lesw	-0x34(%bp), %bx
26:2465  movw	%es:0x2(%bx), %ax
26:2469  lesw	-0x40(%bp), %bx
26:246C  cmpw	%es:0x6(%bx), %ax
26:2470  jge	0x2487 <.text+0x2487>
26:2472  lesw	-0x34(%bp), %bx
26:2475  movw	%es:0x6(%bx), %ax
26:2479  lesw	-0x40(%bp), %bx
26:247C  cmpw	%es:0x2(%bx), %ax
26:2480  jle	0x2487 <.text+0x2487>
26:2482  movw	$0x1, %ax
26:2485  jmp	0x2489 <.text+0x2489>
26:2487  xorw	%ax, %ax
26:2489  orw	%ax, %ax
26:248B  je	0x249f <.text+0x249f>
26:248D  pushw	-0x28(%bp)
26:2490  pushw	-0x2(%bp)
26:2493  pushl	0x6(%bp)
26:2497  nop
26:2498  pushw	%cs
26:2499  callw	0x26bf <.text+0x26bf>
26:249C  addw	$0x8, %sp
26:249F  addw	$0x4, -0x20(%bp)
26:24A3  incw	-0x28(%bp)
26:24A6  lesw	0x6(%bp), %bx
26:24A9  movw	%es:0x44(%bx), %ax
26:24AD  cmpw	-0x28(%bp), %ax
26:24B0  jg	0x2407 <.text+0x2407>
26:24B4  addw	$0x4, -0x22(%bp)
26:24B8  incw	-0x2(%bp)
26:24BB  lesw	0x6(%bp), %bx
26:24BE  movw	%es:0x44(%bx), %ax
26:24C2  decw	%ax
26:24C3  cmpw	-0x2(%bp), %ax
26:24C6  jg	0x23ef <.text+0x23ef>
26:24CA  popw	%di
26:24CB  popw	%si
26:24CC  leave
26:24CD  decw	%bp
26:24CE  lretw
26:24CF  incw	%bp
26:24D0  pushw	%bp
26:24D1  movw	%sp, %bp
26:24D3  subw	$0xe, %sp
26:24D6  pushw	%si
26:24D7  pushw	%di
26:24D8  movw	0xa(%bp), %ax
26:24DB  shlw	$0x2, %ax
26:24DE  lesw	0x6(%bp), %bx
26:24E1  addw	%ax, %bx
26:24E3  lesw	%es:0x5c(%bx), %bx
26:24E7  movw	%es:0x8(%bx), %ax
26:24EB  movw	%ax, -0x2(%bp)
26:24EE  cmpw	$0x1, -0x2(%bp)
26:24F2  jge	0x24f9 <.text+0x24f9>
26:24F4  movw	$0x1, -0x2(%bp)
26:24F9  pushw	-0x2(%bp)
26:24FC  movw	0xa(%bp), %ax
26:24FF  shlw	$0x2, %ax
26:2502  lesw	0x6(%bp), %bx
26:2505  addw	%ax, %bx
26:2507  lesw	%es:0x5c(%bx), %bx
26:250B  pushw	%es:0x2(%bx)
26:250F  movw	0xc(%bp), %ax
26:2512  shlw	$0x2, %ax
26:2515  lesw	0x6(%bp), %bx
26:2518  addw	%ax, %bx
26:251A  pushl	%es:0x5c(%bx)
26:251F  pushw	%cs
26:2520  callw	0x9c5 <.text+0x9c5>
26:2523  addw	$0x8, %sp
26:2526  movw	0xc(%bp), %ax
26:2529  shlw	$0x2, %ax
26:252C  lesw	0x6(%bp), %bx
26:252F  addw	%ax, %bx
26:2531  lesw	%es:0x5c(%bx), %bx
26:2535  cmpw	$0x3, %es:0x1a(%bx)
26:253A  je	0x2627 <.text+0x2627>
26:253E  movw	-0x2(%bp), %ax
26:2541  cltd
26:2542  subw	%dx, %ax
26:2544  sarw	%ax
26:2546  pushw	%ax
26:2547  movw	0xc(%bp), %ax
26:254A  shlw	$0x2, %ax
26:254D  lesw	0x6(%bp), %bx
26:2550  addw	%ax, %bx
26:2552  pushl	%es:0x5c(%bx)
26:2557  pushw	%cs
26:2558  callw	0x4d4 <.text+0x4d4>
26:255B  addw	$0x6, %sp
26:255E  cmpw	$0x1, -0x2(%bp)
26:2562  jle	0x2627 <.text+0x2627>
26:2566  movw	0x18(%bp), %ax
26:2569  addw	0x10(%bp), %ax
26:256C  cltd
26:256D  subw	%dx, %ax
26:256F  sarw	%ax
26:2571  addw	$0xfff6, %ax            # imm = 0xFFF6
26:2574  movw	%ax, -0x4(%bp)
26:2577  movw	0x16(%bp), %ax
26:257A  addw	0xe(%bp), %ax
26:257D  cltd
26:257E  subw	%dx, %ax
26:2580  sarw	%ax
26:2582  addw	$0xfffb, %ax            # imm = 0xFFFB
26:2585  movw	%ax, -0x6(%bp)
26:2588  movw	%ax, -0xa(%bp)
26:258B  movw	-0x4(%bp), %ax
26:258E  movw	%ax, -0x8(%bp)
26:2591  pushl	-0xa(%bp)
26:2595  movw	0xc(%bp), %ax
26:2598  shlw	$0x2, %ax
26:259B  lesw	0x6(%bp), %bx
26:259E  addw	%ax, %bx
26:25A0  pushl	%es:0x74(%bx)
26:25A5  lesw	%es:0x74(%bx), %bx
26:25A9  lesw	%es:0x2(%bx), %bx
26:25AD  lcallw	*%es:(%bx)
26:25B0  addw	$0x8, %sp
26:25B3  movw	0xc(%bp), %ax
26:25B6  shlw	$0x2, %ax
26:25B9  lesw	0x6(%bp), %bx
26:25BC  addw	%ax, %bx
26:25BE  lesw	%es:0x74(%bx), %bx
26:25C2  pushw	%es
26:25C3  pushw	%es:(%bx)
26:25C6  lcallw	$0x0, $0xffff           # imm = 0xFFFF ; NE 25C7=38:02CC
26:25CB  addw	$0x4, %sp
26:25CE  orw	%dx, %ax
26:25D0  jne	0x2627 <.text+0x2627>
26:25D2  movw	0xc(%bp), %ax
26:25D5  shlw	$0x2, %ax
26:25D8  lesw	0x6(%bp), %bx
26:25DB  addw	%ax, %bx
26:25DD  movw	%es:0x74(%bx), %ax
26:25E1  orw	%es:0x76(%bx), %ax
26:25E5  je	0x25f2 <.text+0x25f2>
26:25E7  lesw	%es:0x74(%bx), %bx
26:25EB  movw	%es, %dx
26:25ED  movw	%es:(%bx), %ax
26:25F0  jmp	0x2605 <.text+0x2605>
26:25F2  movw	0xc(%bp), %ax
26:25F5  shlw	$0x2, %ax
26:25F8  lesw	0x6(%bp), %bx
26:25FB  addw	%ax, %bx
26:25FD  movw	%es:0x76(%bx), %dx
26:2601  movw	%es:0x74(%bx), %ax
26:2605  pushw	%dx
26:2606  pushw	%ax
26:2607  lesw	0x6(%bp), %bx
26:260A  movw	%es:0x5a(%bx), %ax
26:260E  movw	%ax, -0xc(%bp)
26:2611  movw	%es:0x58(%bx), %dx
26:2615  movw	%dx, -0xe(%bp)
26:2618  pushw	%ax
26:2619  pushw	%dx
26:261A  lesw	-0xe(%bp), %bx
26:261D  lesw	%es:(%bx), %bx
26:2620  lcallw	*%es:0xc(%bx)
26:2624  addw	$0x8, %sp
26:2627  movw	0xa(%bp), %ax
26:262A  shlw	$0x2, %ax
26:262D  lesw	0x6(%bp), %bx
26:2630  addw	%ax, %bx
26:2632  pushl	%es:0x5c(%bx)
26:2637  pushw	%cs
26:2638  callw	0x99f <.text+0x99f>
26:263B  addw	$0x4, %sp
26:263E  pushw	$0x0
26:2640  movw	0xa(%bp), %ax
26:2643  shlw	$0x2, %ax
26:2646  lesw	0x6(%bp), %bx
26:2649  addw	%ax, %bx
26:264B  pushl	%es:0x5c(%bx)
26:2650  pushw	%cs
26:2651  callw	0x700 <.text+0x700>
26:2654  addw	$0x6, %sp
26:2657  movw	0xa(%bp), %ax
26:265A  shlw	$0x2, %ax
26:265D  lesw	0x6(%bp), %bx
26:2660  addw	%ax, %bx
26:2662  lesw	%es:0x5c(%bx), %bx
26:2666  cmpw	$0x3, %es:0x1c(%bx)
26:266B  jne	0x2682 <.text+0x2682>
26:266D  movw	0xc(%bp), %ax
26:2670  shlw	$0x2, %ax
26:2673  lesw	0x6(%bp), %bx
26:2676  addw	%ax, %bx
26:2678  lesw	%es:0x5c(%bx), %bx
26:267C  cmpw	$0x0, %es:(%bx)
26:2680  je	0x269b <.text+0x269b>
26:2682  pushw	$0x0
26:2684  movw	0xa(%bp), %ax
26:2687  shlw	$0x2, %ax
26:268A  lesw	0x6(%bp), %bx
26:268D  addw	%ax, %bx
26:268F  pushl	%es:0x5c(%bx)
26:2694  pushw	%cs
26:2695  callw	0x4bf <.text+0x4bf>
26:2698  addw	$0x6, %sp
26:269B  cmpw	$0x1, -0x2(%bp)
26:269F  jle	0x26ba <.text+0x26ba>
26:26A1  pushw	$0x2
26:26A3  movw	0xc(%bp), %ax
26:26A6  shlw	$0x2, %ax
26:26A9  lesw	0x6(%bp), %bx
26:26AC  addw	%ax, %bx
26:26AE  pushl	%es:0x5c(%bx)
26:26B3  pushw	%cs
26:26B4  callw	0x4bf <.text+0x4bf>
26:26B7  addw	$0x6, %sp
26:26BA  popw	%di
26:26BB  popw	%si
26:26BC  leave
26:26BD  decw	%bp
26:26BE  lretw
26:26BF  incw	%bp
26:26C0  pushw	%bp
26:26C1  movw	%sp, %bp
26:26C3  subw	$0x38, %sp
26:26C6  pushw	%si
26:26C7  pushw	%di
26:26C8  movw	0xa(%bp), %ax
26:26CB  shlw	$0x2, %ax
26:26CE  lesw	0x6(%bp), %bx
26:26D1  addw	%ax, %bx
26:26D3  movw	%es:0x5e(%bx), %ax
26:26D7  movw	%es:0x5c(%bx), %dx
26:26DB  movw	%ax, -0x2(%bp)
26:26DE  movw	%dx, -0x4(%bp)
26:26E1  movw	0xc(%bp), %ax
26:26E4  shlw	$0x2, %ax
26:26E7  movw	0x6(%bp), %bx
26:26EA  addw	%ax, %bx
26:26EC  movw	%es:0x5e(%bx), %ax
26:26F0  movw	%es:0x5c(%bx), %dx
26:26F4  movw	%ax, -0x6(%bp)
26:26F7  movw	%dx, -0x8(%bp)
26:26FA  pushl	-0x4(%bp)
26:26FE  pushw	%ss
26:26FF  leaw	-0x10(%bp), %ax
26:2702  pushw	%ax
26:2703  pushw	%cs
26:2704  callw	0x752 <.text+0x752>
26:2707  addw	$0x8, %sp
26:270A  leaw	-0x18(%bp), %ax
26:270D  pushw	%ss
26:270E  pushw	%ax
26:270F  leaw	-0x10(%bp), %ax
26:2712  pushw	%ss
26:2713  pushw	%ax
26:2714  movw	$0x8, %cx
26:2717  lcallw	$0x0, $0xffff           # imm = 0xFFFF ; NE 2718=1:3E64
26:271C  leaw	-0x20(%bp), %ax
26:271F  pushw	%ss
26:2720  pushw	%ax
26:2721  leaw	-0x10(%bp), %ax
26:2724  pushw	%ss
26:2725  pushw	%ax
26:2726  movw	$0x8, %cx
26:2729  lcallw	$0x0, $0xffff           # imm = 0xFFFF ; NE 272A=1:3E64
26:272E  pushw	0xa(%bp)
26:2731  pushw	%ss
26:2732  leaw	-0x20(%bp), %ax
26:2735  pushw	%ax
26:2736  pushw	%ss
26:2737  leaw	-0x18(%bp), %ax
26:273A  pushw	%ax
26:273B  pushl	0x6(%bp)
26:273F  nop
26:2740  pushw	%cs
26:2741  callw	0x291d <.text+0x291d>
26:2744  addw	$0xe, %sp
26:2747  pushl	-0x8(%bp)
26:274B  pushw	%ss
26:274C  leaw	-0x28(%bp), %ax
26:274F  pushw	%ax
26:2750  pushw	%cs
26:2751  callw	0x752 <.text+0x752>
26:2754  addw	$0x8, %sp
26:2757  leaw	-0x30(%bp), %ax
26:275A  pushw	%ss
26:275B  pushw	%ax
26:275C  leaw	-0x28(%bp), %ax
26:275F  pushw	%ss
26:2760  pushw	%ax
26:2761  movw	$0x8, %cx
26:2764  lcallw	$0x0, $0xffff           # imm = 0xFFFF ; NE 2765=1:3E64
26:2769  leaw	-0x38(%bp), %ax
26:276C  pushw	%ss
26:276D  pushw	%ax
26:276E  leaw	-0x28(%bp), %ax
26:2771  pushw	%ss
26:2772  pushw	%ax
26:2773  movw	$0x8, %cx
26:2776  lcallw	$0x0, $0xffff           # imm = 0xFFFF ; NE 2777=1:3E64
26:277B  pushw	0xc(%bp)
26:277E  pushw	%ss
26:277F  leaw	-0x38(%bp), %ax
26:2782  pushw	%ax
26:2783  pushw	%ss
26:2784  leaw	-0x30(%bp), %ax
26:2787  pushw	%ax
26:2788  pushl	0x6(%bp)
26:278C  nop
26:278D  pushw	%cs
26:278E  callw	0x291d <.text+0x291d>
26:2791  addw	$0xe, %sp
26:2794  movw	-0x38(%bp), %ax
26:2797  cmpw	-0x14(%bp), %ax
26:279A  jge	0x27b9 <.text+0x27b9>
26:279C  movw	-0x34(%bp), %ax
26:279F  cmpw	-0x18(%bp), %ax
26:27A2  jle	0x27b9 <.text+0x27b9>
26:27A4  movw	-0x36(%bp), %ax
26:27A7  cmpw	-0x12(%bp), %ax
26:27AA  jge	0x27b9 <.text+0x27b9>
26:27AC  movw	-0x32(%bp), %ax
26:27AF  cmpw	-0x16(%bp), %ax
26:27B2  jle	0x27b9 <.text+0x27b9>
26:27B4  movw	$0x1, %ax
26:27B7  jmp	0x27bb <.text+0x27bb>
26:27B9  xorw	%ax, %ax
26:27BB  orw	%ax, %ax
26:27BD  je	0x27e1 <.text+0x27e1>
26:27BF  leaw	-0x38(%bp), %ax
26:27C2  movw	%ss, %dx
26:27C4  movw	$0x8, %cx
26:27C7  lcallw	$0x0, $0xffff           # imm = 0xFFFF ; NE 27C8=1:3E86
26:27CC  leaw	-0x18(%bp), %ax
26:27CF  movw	%ss, %dx
26:27D1  movw	$0x8, %cx
26:27D4  lcallw	$0x0, $0xffff           # imm = 0xFFFF ; NE 27D5=1:3E86
26:27D9  pushw	0xc(%bp)
26:27DC  pushw	0xa(%bp)
26:27DF  jmp	0x282c <.text+0x282c>
26:27E1  movw	-0x20(%bp), %ax
26:27E4  cmpw	-0x2c(%bp), %ax
26:27E7  jge	0x2806 <.text+0x2806>
26:27E9  movw	-0x1c(%bp), %ax
26:27EC  cmpw	-0x30(%bp), %ax
26:27EF  jle	0x2806 <.text+0x2806>
26:27F1  movw	-0x1e(%bp), %ax
26:27F4  cmpw	-0x2a(%bp), %ax
26:27F7  jge	0x2806 <.text+0x2806>
26:27F9  movw	-0x1a(%bp), %ax
26:27FC  cmpw	-0x2e(%bp), %ax
26:27FF  jle	0x2806 <.text+0x2806>
26:2801  movw	$0x1, %ax
26:2804  jmp	0x2808 <.text+0x2808>
26:2806  xorw	%ax, %ax
26:2808  orw	%ax, %ax
26:280A  je	0x283a <.text+0x283a>
26:280C  leaw	-0x20(%bp), %ax
26:280F  movw	%ss, %dx
26:2811  movw	$0x8, %cx
26:2814  lcallw	$0x0, $0xffff           # imm = 0xFFFF ; NE 2815=1:3E86
26:2819  leaw	-0x30(%bp), %ax
26:281C  movw	%ss, %dx
26:281E  movw	$0x8, %cx
26:2821  lcallw	$0x0, $0xffff           # imm = 0xFFFF ; NE 2822=1:3E86
26:2826  pushw	0xa(%bp)
26:2829  pushw	0xc(%bp)
26:282C  pushl	0x6(%bp)
26:2830  pushw	%cs
26:2831  callw	0x24cf <.text+0x24cf>
26:2834  addw	$0x18, %sp
26:2837  jmp	0x2918 <.text+0x2918>
26:283A  movw	-0x30(%bp), %ax
26:283D  cmpw	-0x14(%bp), %ax
26:2840  jge	0x285f <.text+0x285f>
26:2842  movw	-0x2c(%bp), %ax
26:2845  cmpw	-0x18(%bp), %ax
26:2848  jle	0x285f <.text+0x285f>
26:284A  movw	-0x2e(%bp), %ax
26:284D  cmpw	-0x12(%bp), %ax
26:2850  jge	0x285f <.text+0x285f>
26:2852  movw	-0x2a(%bp), %ax
26:2855  cmpw	-0x16(%bp), %ax
26:2858  jle	0x285f <.text+0x285f>
26:285A  movw	$0x1, %ax
26:285D  jmp	0x2861 <.text+0x2861>
26:285F  xorw	%ax, %ax
26:2861  orw	%ax, %ax
26:2863  je	0x28ef <.text+0x28ef>
26:2867  pushl	-0x8(%bp)
26:286B  pushw	%cs
26:286C  callw	0x99f <.text+0x99f>
26:286F  addw	$0x4, %sp
26:2872  pushl	-0x4(%bp)
26:2876  pushw	%cs
26:2877  callw	0x99f <.text+0x99f>
26:287A  addw	$0x4, %sp
26:287D  pushw	$0x0
26:287F  pushl	-0x8(%bp)
26:2883  pushw	%cs
26:2884  callw	0x700 <.text+0x700>
26:2887  addw	$0x6, %sp
26:288A  pushw	$0x0
26:288C  pushl	-0x4(%bp)
26:2890  pushw	%cs
26:2891  callw	0x700 <.text+0x700>
26:2894  addw	$0x6, %sp
26:2897  lesw	-0x8(%bp), %bx
26:289A  cmpw	$0x3, %es:0x1c(%bx)
26:289F  je	0x28ae <.text+0x28ae>
26:28A1  pushw	$0x0
26:28A3  pushw	-0x6(%bp)
26:28A6  pushw	%bx
26:28A7  pushw	%cs
26:28A8  callw	0x4bf <.text+0x4bf>
26:28AB  addw	$0x6, %sp
26:28AE  lesw	-0x4(%bp), %bx
26:28B1  cmpw	$0x3, %es:0x1c(%bx)
26:28B6  je	0x28c5 <.text+0x28c5>
26:28B8  pushw	$0x0
26:28BA  pushw	-0x2(%bp)
26:28BD  pushw	%bx
26:28BE  pushw	%cs
26:28BF  callw	0x4bf <.text+0x4bf>
26:28C2  addw	$0x6, %sp
26:28C5  lesw	-0x8(%bp), %bx
26:28C8  cmpw	$0x0, %es:(%bx)
26:28CC  jne	0x28d9 <.text+0x28d9>
26:28CE  pushw	-0x6(%bp)
26:28D1  pushw	%bx
26:28D2  pushw	%cs
26:28D3  callw	0x851 <.text+0x851>
26:28D6  addw	$0x4, %sp
26:28D9  lesw	-0x4(%bp), %bx
26:28DC  cmpw	$0x0, %es:(%bx)
26:28E0  jne	0x2918 <.text+0x2918>
26:28E2  pushw	-0x2(%bp)
26:28E5  pushw	%bx
26:28E6  pushw	%cs
26:28E7  callw	0x851 <.text+0x851>
26:28EA  addw	$0x4, %sp
26:28ED  jmp	0x2918 <.text+0x2918>
26:28EF  movw	-0x20(%bp), %ax
26:28F2  cmpw	-0x34(%bp), %ax
26:28F5  jge	0x2914 <.text+0x2914>
26:28F7  movw	-0x1c(%bp), %ax
26:28FA  cmpw	-0x38(%bp), %ax
26:28FD  jle	0x2914 <.text+0x2914>
26:28FF  movw	-0x1e(%bp), %ax
26:2902  cmpw	-0x32(%bp), %ax
26:2905  jge	0x2914 <.text+0x2914>
26:2907  movw	-0x1a(%bp), %ax
26:290A  cmpw	-0x36(%bp), %ax
26:290D  jle	0x2914 <.text+0x2914>
26:290F  movw	$0x1, %ax
26:2912  jmp	0x2916 <.text+0x2916>
26:2914  xorw	%ax, %ax
26:2916  orw	%ax, %ax
26:2918  popw	%di
26:2919  popw	%si
26:291A  leave
26:291B  decw	%bp
26:291C  lretw
26:291D  incw	%bp
26:291E  pushw	%bp
26:291F  movw	%sp, %bp
26:2921  subw	$0x1e, %sp
26:2924  pushw	%si
26:2925  pushw	%di
26:2926  movw	0x12(%bp), %dx
26:2929  movw	%dx, %ax
26:292B  shlw	$0x2, %ax
26:292E  lesw	0x6(%bp), %bx
26:2931  addw	%ax, %bx
26:2933  lesw	%es:0x5c(%bx), %bx
26:2937  movw	%es:0x2(%bx), %ax
26:293B  movw	%ax, -0x2(%bp)
26:293E  movw	%dx, %ax
26:2940  shlw	$0x2, %ax
26:2943  lesw	0x6(%bp), %bx
26:2946  addw	%ax, %bx
26:2948  pushl	%es:0x5c(%bx)
26:294D  pushw	%ss
26:294E  leaw	-0xa(%bp), %ax
26:2951  pushw	%ax
26:2952  pushw	%cs
26:2953  callw	0x752 <.text+0x752>
26:2956  addw	$0x8, %sp
26:2959  movw	-0x8(%bp), %ax
26:295C  movw	%ax, -0x10(%bp)
26:295F  movw	-0xa(%bp), %ax
26:2962  movw	%ax, -0x12(%bp)
26:2965  movw	%ax, -0x16(%bp)
26:2968  movw	-0x10(%bp), %ax
26:296B  movw	%ax, -0x14(%bp)
26:296E  movl	-0x16(%bp), %eax
26:2972  movl	%eax, -0xe(%bp)
26:2976  pushw	-0x2(%bp)
26:2979  pushl	0xa(%bp)
26:297D  pushw	%cs
26:297E  callw	0x0 <.text>
26:2981  addw	$0x6, %sp
26:2984  movl	0xa(%bp), %eax
26:2988  movl	%eax, -0x1a(%bp)
26:298C  lesw	-0x1a(%bp), %bx
26:298F  movw	-0xe(%bp), %ax
26:2992  addw	%ax, %es:(%bx)
26:2995  addw	%ax, %es:0x4(%bx)
26:2999  movw	-0xc(%bp), %ax
26:299C  addw	%ax, %es:0x2(%bx)
26:29A0  addw	%ax, %es:0x6(%bx)
26:29A4  pushw	-0x2(%bp)
26:29A7  pushl	0xe(%bp)
26:29AB  pushw	%cs
26:29AC  callw	0x80 <.text+0x80>
26:29AF  addw	$0x6, %sp
26:29B2  movl	0xe(%bp), %eax
26:29B6  movl	%eax, -0x1e(%bp)
26:29BA  lesw	-0x1e(%bp), %bx
26:29BD  movw	-0xe(%bp), %ax
26:29C0  addw	%ax, %es:(%bx)
26:29C3  addw	%ax, %es:0x4(%bx)
26:29C7  movw	-0xc(%bp), %ax
26:29CA  addw	%ax, %es:0x2(%bx)
26:29CE  addw	%ax, %es:0x6(%bx)
26:29D2  popw	%di
26:29D3  popw	%si
26:29D4  leave
26:29D5  decw	%bp
26:29D6  lretw
26:29D7  incw	%bp
26:29D8  pushw	%bp
26:29D9  movw	%sp, %bp
26:29DB  pushw	%si
26:29DC  pushw	%di
26:29DD  cmpl	$0x0, 0x6(%bp)
26:29E2  jne	0x29f6 <.text+0x29f6>
26:29E4  pushw	$0x2a
26:29E6  lcallw	$0x0, $0xffff           # imm = 0xFFFF ; NE 29E7=30:005C
26:29EB  popw	%cx
26:29EC  movw	%dx, 0x8(%bp)
26:29EF  movw	%ax, 0x6(%bp)
26:29F2  orw	%dx, %ax
26:29F4  je	0x2a1b <.text+0x2a1b>
26:29F6  pushw	$0x0
26:29F8  pushl	0x6(%bp)
26:29FC  lcallw	$0x0, $0xffff           # imm = 0xFFFF ; NE 29FD=32:000D
26:2A01  addw	$0x6, %sp
26:2A04  lesw	0x6(%bp), %bx
26:2A07  movw	$0x0, %es:0x2(%bx) ; NE 2A0B=26:2F9D
26:2A0D  movw	$0x2f9d, %es:(%bx)      # imm = 0x2F9D
26:2A12  movl	0xa(%bp), %eax
26:2A16  movl	%eax, %es:0x26(%bx)
26:2A1B  movw	0x8(%bp), %dx
26:2A1E  movw	0x6(%bp), %ax
26:2A21  popw	%di
26:2A22  popw	%si
26:2A23  popw	%bp
26:2A24  decw	%bp
26:2A25  lretw
26:2A26  incw	%bp
26:2A27  pushw	%bp
26:2A28  movw	%sp, %bp
26:2A2A  pushw	%si
26:2A2B  pushw	%di
26:2A2C  lesw	0x6(%bp), %bx
26:2A2F  lesw	%es:0x26(%bx), %bx
26:2A33  movw	$0x1, %es:(%bx)
26:2A38  popw	%di
26:2A39  popw	%si
26:2A3A  popw	%bp
26:2A3B  decw	%bp
26:2A3C  lretw
26:2A3D  incw	%bp
26:2A3E  pushw	%bp
26:2A3F  movw	%sp, %bp
26:2A41  subw	$0x8, %sp
26:2A44  pushw	%si
26:2A45  pushw	%di
26:2A46  pushl	$0x0
26:2A49  movw	$0x0, %ax ; NE 2A4A=49:1204
26:2A4C  movw	%ax, %es
26:2A4E  pushl	%es:0x1204
26:2A54  pushw	$0x2a
26:2A56  lcallw	$0x0, $0xffff           # imm = 0xFFFF ; NE 2A57=30:0113
26:2A5B  addw	$0xa, %sp
26:2A5E  movw	%dx, -0x6(%bp)
26:2A61  movw	%ax, -0x8(%bp)
26:2A64  orw	%dx, %ax
26:2A66  je	0x2a7d <.text+0x2a7d>
26:2A68  lesw	0x6(%bp), %bx
26:2A6B  pushl	%es:0x26(%bx)
26:2A70  pushw	%dx
26:2A71  pushw	-0x8(%bp)
26:2A74  pushw	%cs
26:2A75  callw	0x29d7 <.text+0x29d7>
26:2A78  addw	$0x8, %sp
26:2A7B  jmp	0x2a83 <.text+0x2a83>
26:2A7D  movw	-0x6(%bp), %dx
26:2A80  movw	-0x8(%bp), %ax
26:2A83  movw	%dx, -0x2(%bp)
26:2A86  movw	%ax, -0x4(%bp)
26:2A89  popw	%di
26:2A8A  popw	%si
26:2A8B  leave
26:2A8C  decw	%bp
26:2A8D  lretw
26:2A8E  incw	%bp
26:2A8F  pushw	%bp
26:2A90  movw	%sp, %bp
26:2A92  subw	$0x10, %sp
26:2A95  pushw	%si
26:2A96  pushw	%di
26:2A97  cmpl	$0x0, 0x6(%bp)
26:2A9C  jne	0x2ab2 <.text+0x2ab2>
26:2A9E  pushw	$0x22
26:2AA0  lcallw	$0x0, $0xffff           # imm = 0xFFFF ; NE 2AA1=30:005C
26:2AA5  popw	%cx
26:2AA6  movw	%dx, 0x8(%bp)
26:2AA9  movw	%ax, 0x6(%bp)
26:2AAC  orw	%dx, %ax
26:2AAE  je	0x2b6a <.text+0x2b6a>
26:2AB2  lesw	0x6(%bp), %bx
26:2AB5  movl	0xe(%bp), %eax
26:2AB9  movl	%eax, %es:0x8(%bx)
26:2ABE  movl	0xa(%bp), %eax
26:2AC2  movl	%eax, %es:0x4(%bx)
26:2AC7  pushl	$0x0
26:2ACA  movw	$0x0, %ax ; NE 2ACB=49:1204
26:2ACD  movw	%ax, %es
26:2ACF  pushl	%es:0x1204
26:2AD5  pushw	$0x2a
26:2AD7  lcallw	$0x0, $0xffff           # imm = 0xFFFF ; NE 2AD8=30:0113
26:2ADC  addw	$0xa, %sp
26:2ADF  movw	%dx, -0x2(%bp)
26:2AE2  movw	%ax, -0x4(%bp)
26:2AE5  orw	%dx, %ax
26:2AE7  je	0x2af9 <.text+0x2af9>
26:2AE9  pushw	$0x0
26:2AEB  pushw	%dx
26:2AEC  pushw	-0x4(%bp)
26:2AEF  lcallw	$0x0, $0xffff           # imm = 0xFFFF ; NE 2AF0=38:0C79
26:2AF4  addw	$0x6, %sp
26:2AF7  jmp	0x2aff <.text+0x2aff>
26:2AF9  movw	-0x2(%bp), %dx
26:2AFC  movw	-0x4(%bp), %ax
26:2AFF  lesw	0x6(%bp), %bx
26:2B02  movw	%dx, %es:0x2(%bx)
26:2B06  movw	%ax, %es:(%bx)
26:2B09  pushw	$0x8
26:2B0B  lesw	%es:(%bx), %bx
26:2B0E  movw	%es, -0xa(%bp)
26:2B11  movw	%bx, -0xc(%bp)
26:2B14  pushw	%es
26:2B15  pushw	%es:(%bx)
26:2B18  movw	%es:(%bx), %bx
26:2B1B  lesw	%es:(%bx), %bx
26:2B1E  lcallw	*%es:0x34(%bx)
26:2B22  addw	$0x6, %sp
26:2B25  movw	$0xfa, -0x8(%bp)
26:2B2A  movw	$0x136, -0x6(%bp)       # imm = 0x136
26:2B2F  pushl	-0x8(%bp)
26:2B33  lesw	0x6(%bp), %bx
26:2B36  movw	%es:0x2(%bx), %ax
26:2B3A  movw	%ax, -0xe(%bp)
26:2B3D  movw	%es:(%bx), %dx
26:2B40  movw	%dx, -0x10(%bp)
26:2B43  pushw	%ax
26:2B44  pushw	%dx
26:2B45  lesw	-0x10(%bp), %bx
26:2B48  lesw	%es:0x2(%bx), %bx
26:2B4C  lcallw	*%es:(%bx)
26:2B4F  addw	$0x8, %sp
26:2B52  pushl	0x6(%bp)
26:2B56  nop
26:2B57  pushw	%cs
26:2B58  callw	0x2b75 <.text+0x2b75>
26:2B5B  addw	$0x4, %sp
26:2B5E  pushl	0x6(%bp)
26:2B62  nop
26:2B63  pushw	%cs
26:2B64  callw	0x2d90 <.text+0x2d90>
26:2B67  addw	$0x4, %sp
26:2B6A  movw	0x8(%bp), %dx
26:2B6D  movw	0x6(%bp), %ax
26:2B70  popw	%di
26:2B71  popw	%si
26:2B72  leave
26:2B73  decw	%bp
26:2B74  lretw
26:2B75  incw	%bp
26:2B76  pushw	%bp
26:2B77  movw	%sp, %bp
26:2B79  subw	$0x12, %sp
26:2B7C  pushw	%si
26:2B7D  pushw	%di
26:2B7E  lesw	0x6(%bp), %bx
26:2B81  movw	$0x0, %es:0x10(%bx)
26:2B87  pushw	$0x492a                 # imm = 0x492A
26:2B8A  pushw	%ss
26:2B8B  leaw	-0xa(%bp), %ax
26:2B8E  pushw	%ax
26:2B8F  lcallw	$0x0, $0xffff           # imm = 0xFFFF ; NE 2B90=34:0036
26:2B94  addw	$0x6, %sp
26:2B97  pushw	%dx
26:2B98  pushw	%ax
26:2B99  lesw	0x6(%bp), %bx
26:2B9C  movw	%es:0x2(%bx), %ax
26:2BA0  movw	%ax, -0xc(%bp)
26:2BA3  movw	%es:(%bx), %dx
26:2BA6  movw	%dx, -0xe(%bp)
26:2BA9  addw	$0xa, %dx
26:2BAC  pushw	%ax
26:2BAD  pushw	%dx
26:2BAE  lesw	-0xe(%bp), %bx
26:2BB1  lesw	%es:0xc(%bx), %bx
26:2BB5  lcallw	*%es:(%bx)
26:2BB8  addw	$0x8, %sp
26:2BBB  pushw	$0x0
26:2BBD  pushw	%ss
26:2BBE  leaw	-0xa(%bp), %ax
26:2BC1  pushw	%ax
26:2BC2  lcallw	$0x0, $0xffff           # imm = 0xFFFF ; NE 2BC3=35:159E
26:2BC7  addw	$0x6, %sp
26:2BCA  lesw	0x6(%bp), %bx
26:2BCD  lesw	%es:(%bx), %bx
26:2BD0  pushw	%es
26:2BD1  pushw	%es:(%bx)
26:2BD4  lcallw	$0x0, $0xffff           # imm = 0xFFFF ; NE 2BD5=38:02CC
26:2BD9  addw	$0x4, %sp
26:2BDC  orw	%dx, %ax
26:2BDE  jne	0x2c1d <.text+0x2c1d>
26:2BE0  lesw	0x6(%bp), %bx
26:2BE3  cmpl	$0x0, %es:(%bx)
26:2BE8  je	0x2bf1 <.text+0x2bf1>
26:2BEA  lesw	%es:(%bx), %bx
26:2BED  movw	%es, %dx
26:2BEF  jmp	0x2bf8 <.text+0x2bf8>
26:2BF1  lesw	0x6(%bp), %bx
26:2BF4  movw	%es:0x2(%bx), %dx
26:2BF8  movw	%es:(%bx), %ax
26:2BFB  pushw	%dx
26:2BFC  pushw	%ax
26:2BFD  lesw	0x6(%bp), %bx
26:2C00  movw	%es:0x6(%bx), %ax
26:2C04  movw	%ax, -0x10(%bp)
26:2C07  movw	%es:0x4(%bx), %dx
26:2C0B  movw	%dx, -0x12(%bp)
26:2C0E  pushw	%ax
26:2C0F  pushw	%dx
26:2C10  lesw	-0x12(%bp), %bx
26:2C13  lesw	%es:(%bx), %bx
26:2C16  lcallw	*%es:0xc(%bx)
26:2C1A  addw	$0x8, %sp
26:2C1D  popw	%di
26:2C1E  popw	%si
26:2C1F  leave
26:2C20  decw	%bp
26:2C21  lretw
26:2C22  incw	%bp
26:2C23  pushw	%bp
26:2C24  movw	%sp, %bp
26:2C26  subw	$0x8, %sp
26:2C29  pushw	%si
26:2C2A  pushw	%di
26:2C2B  cmpl	$0x0, 0x6(%bp)
26:2C30  je	0x2ccb <.text+0x2ccb>
26:2C34  lesw	0x6(%bp), %bx
26:2C37  cmpl	$0x0, %es:(%bx)
26:2C3C  je	0x2cb9 <.text+0x2cb9>
26:2C3E  lesw	%es:(%bx), %bx
26:2C41  pushw	%es
26:2C42  pushw	%es:(%bx)
26:2C45  lcallw	$0x0, $0xffff           # imm = 0xFFFF ; NE 2C46=38:02CC
26:2C4A  addw	$0x4, %sp
26:2C4D  orw	%dx, %ax
26:2C4F  je	0x2c8e <.text+0x2c8e>
26:2C51  lesw	0x6(%bp), %bx
26:2C54  cmpl	$0x0, %es:(%bx)
26:2C59  je	0x2c62 <.text+0x2c62>
26:2C5B  lesw	%es:(%bx), %bx
26:2C5E  movw	%es, %dx
26:2C60  jmp	0x2c69 <.text+0x2c69>
26:2C62  lesw	0x6(%bp), %bx
26:2C65  movw	%es:0x2(%bx), %dx
26:2C69  movw	%es:(%bx), %ax
26:2C6C  pushw	%dx
26:2C6D  pushw	%ax
26:2C6E  lesw	0x6(%bp), %bx
26:2C71  movw	%es:0x6(%bx), %ax
26:2C75  movw	%ax, -0x2(%bp)
26:2C78  movw	%es:0x4(%bx), %dx
26:2C7C  movw	%dx, -0x4(%bp)
26:2C7F  pushw	%ax
26:2C80  pushw	%dx
26:2C81  lesw	-0x4(%bp), %bx
26:2C84  lesw	%es:(%bx), %bx
26:2C87  lcallw	*%es:0x10(%bx)
26:2C8B  addw	$0x8, %sp
26:2C8E  lesw	0x6(%bp), %bx
26:2C91  cmpl	$0x0, %es:(%bx)
26:2C96  je	0x2cb9 <.text+0x2cb9>
26:2C98  pushw	$0x3
26:2C9A  movw	%es:0x2(%bx), %ax
26:2C9E  movw	%ax, -0x6(%bp)
26:2CA1  movw	%es:(%bx), %dx
26:2CA4  movw	%dx, -0x8(%bp)
26:2CA7  pushw	%ax
26:2CA8  pushw	%dx
26:2CA9  lesw	-0x8(%bp), %bx
26:2CAC  lesw	%es:0x2(%bx), %bx
26:2CB0  lcallw	*%es:0x8(%bx)
26:2CB4  addw	$0x6, %sp
26:2CB7  jmp	0x2cb9 <.text+0x2cb9>
26:2CB9  testb	$0x1, 0xa(%bp)
26:2CBD  je	0x2ccb <.text+0x2ccb>
26:2CBF  pushl	0x6(%bp)
26:2CC3  lcallw	$0x0, $0xffff           # imm = 0xFFFF ; NE 2CC4=30:0272
26:2CC8  addw	$0x4, %sp
26:2CCB  popw	%di
26:2CCC  popw	%si
26:2CCD  leave
26:2CCE  decw	%bp
26:2CCF  lretw
26:2CD0  incw	%bp
26:2CD1  pushw	%bp
26:2CD2  movw	%sp, %bp
26:2CD4  subw	$0x24, %sp
26:2CD7  pushw	%si
26:2CD8  pushw	%di
26:2CD9  lesw	0x6(%bp), %bx
26:2CDC  movl	0xa(%bp), %eax
26:2CE0  movl	%eax, %es:0xc(%bx)
26:2CE5  movw	%es:0x2(%bx), %ax
26:2CE9  movw	%ax, -0x1a(%bp)
26:2CEC  movw	%es:(%bx), %dx
26:2CEF  movw	%dx, -0x1c(%bp)
26:2CF2  pushw	%ax
26:2CF3  pushw	%dx
26:2CF4  pushw	%ss
26:2CF5  leaw	-0xa(%bp), %ax
26:2CF8  pushw	%ax
26:2CF9  lesw	-0x1c(%bp), %bx
26:2CFC  lesw	%es:0x2(%bx), %bx
26:2D00  lcallw	*%es:0xc(%bx)
26:2D04  addw	$0x8, %sp
26:2D07  movw	-0x6(%bp), %ax
26:2D0A  subw	-0xa(%bp), %ax
26:2D0D  movw	%ax, -0x2(%bp)
26:2D10  lesw	0x6(%bp), %bx
26:2D13  movw	%es:0x2(%bx), %ax
26:2D17  movw	%ax, -0x1e(%bp)
26:2D1A  movw	%es:(%bx), %dx
26:2D1D  movw	%dx, -0x20(%bp)
26:2D20  pushw	%ax
26:2D21  pushw	%dx
26:2D22  pushw	%ss
26:2D23  leaw	-0x14(%bp), %ax
26:2D26  pushw	%ax
26:2D27  lesw	-0x20(%bp), %bx
26:2D2A  lesw	%es:0x2(%bx), %bx
26:2D2E  lcallw	*%es:0xc(%bx)
26:2D32  addw	$0x8, %sp
26:2D35  movw	-0xe(%bp), %ax
26:2D38  subw	-0x12(%bp), %ax
26:2D3B  movw	%ax, -0xc(%bp)
26:2D3E  movl	0xa(%bp), %eax
26:2D42  movl	%eax, -0x18(%bp)
26:2D46  movw	-0x2(%bp), %ax
26:2D49  cltd
26:2D4A  subw	%dx, %ax
26:2D4C  sarw	%ax
26:2D4E  subw	%ax, -0x18(%bp)
26:2D51  movw	-0xc(%bp), %ax
26:2D54  cltd
26:2D55  subw	%dx, %ax
26:2D57  sarw	%ax
26:2D59  subw	%ax, -0x16(%bp)
26:2D5C  pushl	-0x18(%bp)
26:2D60  lesw	0x6(%bp), %bx
26:2D63  movw	%es:0x2(%bx), %ax
26:2D67  movw	%ax, -0x22(%bp)
26:2D6A  movw	%es:(%bx), %dx
26:2D6D  movw	%dx, -0x24(%bp)
26:2D70  pushw	%ax
26:2D71  pushw	%dx
26:2D72  lesw	-0x24(%bp), %bx
26:2D75  lesw	%es:0x2(%bx), %bx
26:2D79  lcallw	*%es:(%bx)
26:2D7C  addw	$0x8, %sp
26:2D7F  pushl	0x6(%bp)
26:2D83  nop
26:2D84  pushw	%cs
26:2D85  callw	0x2d90 <.text+0x2d90>
26:2D88  addw	$0x4, %sp
26:2D8B  popw	%di
26:2D8C  popw	%si
26:2D8D  leave
26:2D8E  decw	%bp
26:2D8F  lretw
26:2D90  incw	%bp
26:2D91  pushw	%bp
26:2D92  movw	%sp, %bp
26:2D94  subw	$0xc, %sp
26:2D97  pushw	%si
26:2D98  pushw	%di
26:2D99  lesw	0x6(%bp), %bx
26:2D9C  movw	%es:0x2(%bx), %ax
26:2DA0  movw	%ax, -0xa(%bp)
26:2DA3  movw	%es:(%bx), %dx
26:2DA6  movw	%dx, -0xc(%bp)
26:2DA9  pushw	%ax
26:2DAA  pushw	%dx
26:2DAB  pushw	%ss
26:2DAC  leaw	-0x8(%bp), %ax
26:2DAF  pushw	%ax
26:2DB0  lesw	-0xc(%bp), %bx
26:2DB3  lesw	%es:0x2(%bx), %bx
26:2DB7  lcallw	*%es:0xc(%bx)
26:2DBB  addw	$0x8, %sp
26:2DBE  movw	0x6(%bp), %ax
26:2DC1  addw	$0x12, %ax
26:2DC4  pushw	0x8(%bp)
26:2DC7  pushw	%ax
26:2DC8  leaw	-0x8(%bp), %ax
26:2DCB  pushw	%ss
26:2DCC  pushw	%ax
26:2DCD  movw	$0x8, %cx
26:2DD0  lcallw	$0x0, $0xffff           # imm = 0xFFFF ; NE 2DD1=1:3E64
26:2DD5  lesw	0x6(%bp), %bx
26:2DD8  addw	$0x14, %es:0x12(%bx)
26:2DDD  addw	$0x11, %es:0x14(%bx)
26:2DE2  movw	-0x8(%bp), %ax
26:2DE5  addw	$0x29, %ax
26:2DE8  movw	%ax, %es:0x16(%bx)
26:2DEC  movw	-0x6(%bp), %ax
26:2DEF  addw	$0x2b, %ax
26:2DF2  movw	%ax, %es:0x18(%bx)
26:2DF6  movw	0x6(%bp), %ax
26:2DF9  addw	$0x1a, %ax
26:2DFC  pushw	0x8(%bp)
26:2DFF  pushw	%ax
26:2E00  leaw	-0x8(%bp), %ax
26:2E03  pushw	%ss
26:2E04  pushw	%ax
26:2E05  movw	$0x8, %cx
26:2E08  lcallw	$0x0, $0xffff           # imm = 0xFFFF ; NE 2E09=1:3E64
26:2E0D  lesw	0x6(%bp), %bx
26:2E10  addw	$0xf, %es:0x1a(%bx)
26:2E15  addw	$0x20, %es:0x1c(%bx)
26:2E1A  movw	-0x8(%bp), %ax
26:2E1D  addw	$0x1f, %ax
26:2E20  movw	%ax, %es:0x1e(%bx)
26:2E24  movw	-0x6(%bp), %ax
26:2E27  addw	$0x35, %ax
26:2E2A  movw	%ax, %es:0x20(%bx)
26:2E2E  popw	%di
26:2E2F  popw	%si
26:2E30  leave
26:2E31  decw	%bp
26:2E32  lretw
26:2E33  incw	%bp
26:2E34  pushw	%bp
26:2E35  movw	%sp, %bp
26:2E37  subw	$0x4, %sp
26:2E3A  pushw	%si
26:2E3B  pushw	%di
26:2E3C  lesw	0x6(%bp), %bx
26:2E3F  cmpw	$0x0, %es:0x10(%bx)
26:2E44  jne	0x2e89 <.text+0x2e89>
26:2E46  cmpl	$0x0, %es:(%bx)
26:2E4B  je	0x2e54 <.text+0x2e54>
26:2E4D  lesw	%es:(%bx), %bx
26:2E50  movw	%es, %dx
26:2E52  jmp	0x2e5b <.text+0x2e5b>
26:2E54  lesw	0x6(%bp), %bx
26:2E57  movw	%es:0x2(%bx), %dx
26:2E5B  movw	%es:(%bx), %ax
26:2E5E  pushw	%dx
26:2E5F  pushw	%ax
26:2E60  lesw	0x6(%bp), %bx
26:2E63  movw	%es:0x6(%bx), %ax
26:2E67  movw	%ax, -0x2(%bp)
26:2E6A  movw	%es:0x4(%bx), %dx
26:2E6E  movw	%dx, -0x4(%bp)
26:2E71  pushw	%ax
26:2E72  pushw	%dx
26:2E73  lesw	-0x4(%bp), %bx
26:2E76  lesw	%es:(%bx), %bx
26:2E79  lcallw	*%es:0x10(%bx)
26:2E7D  addw	$0x8, %sp
26:2E80  lesw	0x6(%bp), %bx
26:2E83  movw	$0x1, %es:0x10(%bx)
26:2E89  popw	%di
26:2E8A  popw	%si
26:2E8B  leave
26:2E8C  decw	%bp
26:2E8D  lretw
26:2E8E  incw	%bp
26:2E8F  pushw	%bp
26:2E90  movw	%sp, %bp
26:2E92  pushw	%si
26:2E93  pushw	%di
26:2E94  lesw	0x6(%bp), %bx
26:2E97  movw	%es:0x12(%bx), %ax
26:2E9B  cmpw	0xe(%bp), %ax
26:2E9E  jge	0x2ec0 <.text+0x2ec0>
26:2EA0  movw	%es:0x16(%bx), %ax
26:2EA4  cmpw	0xa(%bp), %ax
26:2EA7  jle	0x2ec0 <.text+0x2ec0>
26:2EA9  movw	%es:0x14(%bx), %ax
26:2EAD  cmpw	0x10(%bp), %ax
26:2EB0  jge	0x2ec0 <.text+0x2ec0>
26:2EB2  movw	%es:0x18(%bx), %ax
26:2EB6  cmpw	0xc(%bp), %ax
26:2EB9  jle	0x2ec0 <.text+0x2ec0>
26:2EBB  movw	$0x1, %ax
26:2EBE  jmp	0x2ec2 <.text+0x2ec2>
26:2EC0  xorw	%ax, %ax
26:2EC2  orw	%ax, %ax
26:2EC4  jne	0x2f5e <.text+0x2f5e>
26:2EC8  lesw	0x6(%bp), %bx
26:2ECB  movw	%es:0x1a(%bx), %ax
26:2ECF  cmpw	0xe(%bp), %ax
26:2ED2  jge	0x2ef4 <.text+0x2ef4>
26:2ED4  movw	%es:0x1e(%bx), %ax
26:2ED8  cmpw	0xa(%bp), %ax
26:2EDB  jle	0x2ef4 <.text+0x2ef4>
26:2EDD  movw	%es:0x1c(%bx), %ax
26:2EE1  cmpw	0x10(%bp), %ax
26:2EE4  jge	0x2ef4 <.text+0x2ef4>
26:2EE6  movw	%es:0x20(%bx), %ax
26:2EEA  cmpw	0xc(%bp), %ax
26:2EED  jle	0x2ef4 <.text+0x2ef4>
26:2EEF  movw	$0x1, %ax
26:2EF2  jmp	0x2ef6 <.text+0x2ef6>
26:2EF4  xorw	%ax, %ax
26:2EF6  orw	%ax, %ax
26:2EF8  jne	0x2f5e <.text+0x2f5e>
26:2EFA  lesw	0x6(%bp), %bx
26:2EFD  movw	%es:0x12(%bx), %ax
26:2F01  cmpw	0x16(%bp), %ax
26:2F04  jge	0x2f26 <.text+0x2f26>
26:2F06  movw	%es:0x16(%bx), %ax
26:2F0A  cmpw	0x12(%bp), %ax
26:2F0D  jle	0x2f26 <.text+0x2f26>
26:2F0F  movw	%es:0x14(%bx), %ax
26:2F13  cmpw	0x18(%bp), %ax
26:2F16  jge	0x2f26 <.text+0x2f26>
26:2F18  movw	%es:0x18(%bx), %ax
26:2F1C  cmpw	0x14(%bp), %ax
26:2F1F  jle	0x2f26 <.text+0x2f26>
26:2F21  movw	$0x1, %ax
26:2F24  jmp	0x2f28 <.text+0x2f28>
26:2F26  xorw	%ax, %ax
26:2F28  orw	%ax, %ax
26:2F2A  jne	0x2f5e <.text+0x2f5e>
26:2F2C  lesw	0x6(%bp), %bx
26:2F2F  movw	%es:0x1a(%bx), %ax
26:2F33  cmpw	0x16(%bp), %ax
26:2F36  jge	0x2f58 <.text+0x2f58>
26:2F38  movw	%es:0x1e(%bx), %ax
26:2F3C  cmpw	0x12(%bp), %ax
26:2F3F  jle	0x2f58 <.text+0x2f58>
26:2F41  movw	%es:0x1c(%bx), %ax
26:2F45  cmpw	0x18(%bp), %ax
26:2F48  jge	0x2f58 <.text+0x2f58>
26:2F4A  movw	%es:0x20(%bx), %ax
26:2F4E  cmpw	0x14(%bp), %ax
26:2F51  jle	0x2f58 <.text+0x2f58>
26:2F53  movw	$0x1, %ax
26:2F56  jmp	0x2f5a <.text+0x2f5a>
26:2F58  xorw	%ax, %ax
26:2F5A  orw	%ax, %ax
26:2F5C  je	0x2f63 <.text+0x2f63>
26:2F5E  movw	$0x1, %ax
26:2F61  jmp	0x2f65 <.text+0x2f65>
26:2F63  xorw	%ax, %ax
26:2F65  popw	%di
26:2F66  popw	%si
26:2F67  popw	%bp
26:2F68  decw	%bp
26:2F69  lretw
```

### GW: coordenadas, giros e deslocamentos (18:1B5D-22A3)

```asm
18:1B5D  incw	%bp
18:1B5E  pushw	%bp
18:1B5F  movw	%sp, %bp
18:1B61  subw	$0x6, %sp
18:1B64  pushw	%si
18:1B65  pushw	%di
18:1B66  movw	0xa(%bp), %dx
18:1B69  lesw	0x6(%bp), %bx
18:1B6C  lesw	%es:0x8e(%bx), %bx
18:1B71  movw	%es:0xa(%bx), %ax
18:1B75  movw	%ax, -0x2(%bp)
18:1B78  movw	%es:0xc(%bx), %cx
18:1B7C  cmpw	$-0x1, %dx
18:1B7F  je	0x1b84 <.text+0x1b84>
18:1B81  movw	%dx, -0x2(%bp)
18:1B84  cmpw	$-0x1, 0xc(%bp)
18:1B88  je	0x1b8d <.text+0x1b8d>
18:1B8A  movw	0xc(%bp), %cx
18:1B8D  cmpw	$0x7, %cx
18:1B90  jle	0x1b95 <.text+0x1b95>
18:1B92  movw	$0x7, %cx
18:1B95  lesw	0x6(%bp), %bx
18:1B98  movw	%es:0x92(%bx), %ax
18:1B9D  movw	-0x2(%bp), %dx
18:1BA0  imulw	$0x30, %dx, %dx
18:1BA3  addw	%dx, %ax
18:1BA5  movw	%ax, -0x6(%bp)
18:1BA8  movw	%es:0x98(%bx), %ax
18:1BAD  movw	%cx, %dx
18:1BAF  incw	%dx
18:1BB0  movw	$0x8, %bx
18:1BB3  subw	%dx, %bx
18:1BB5  imulw	$0x30, %bx, %bx
18:1BB8  subw	%bx, %ax
18:1BBA  movw	%ax, -0x4(%bp)
18:1BBD  movl	-0x6(%bp), %eax
18:1BC1  shldl	$0x10, %eax, %edx
18:1BC6  popw	%di
18:1BC7  popw	%si
18:1BC8  leave
18:1BC9  decw	%bp
18:1BCA  lretw
18:1BCB  incw	%bp
18:1BCC  pushw	%bp
18:1BCD  movw	%sp, %bp
18:1BCF  subw	$0x36, %sp
18:1BD2  pushw	%si
18:1BD3  pushw	%di
18:1BD4  movw	$0x1, -0xa(%bp)
18:1BD9  lesw	0x6(%bp), %bx
18:1BDC  cmpw	$0x0, %es:0xa8(%bx)
18:1BE2  jne	0x1e4a <.text+0x1e4a>
18:1BE6  pushl	$0x0
18:1BE9  pushl	$0x14394                # imm = 0x14394
18:1BEF  pushl	%es:0x86(%bx)
18:1BF5  lcallw	$0x0, $0xffff           # imm = 0xFFFF ; NE 1BF6=36:28F7
18:1BFA  addw	$0xc, %sp
18:1BFD  lesw	0x6(%bp), %bx
18:1C00  lesw	%es:0x8e(%bx), %bx
18:1C05  lesw	%es:0x1e(%bx), %bx
18:1C09  cmpw	$0x0, %es:0x8(%bx)
18:1C0E  je	0x1e4a <.text+0x1e4a>
18:1C12  lesw	0x6(%bp), %bx
18:1C15  lesw	%es:0x8e(%bx), %bx
18:1C1A  movl	%es:0xa(%bx), %eax
18:1C1F  movl	%eax, -0x4(%bp)
18:1C23  lesw	%es:0x1e(%bx), %bx
18:1C27  movl	%es:0xa(%bx), %eax
18:1C2C  movl	%eax, -0x8(%bp)
18:1C30  pushl	-0x4(%bp)
18:1C34  pushl	0x6(%bp)
18:1C38  pushw	%cs
18:1C39  callw	0x1b5d <.text+0x1b5d>
18:1C3C  addw	$0x8, %sp
18:1C3F  movw	%dx, -0x30(%bp)
18:1C42  movw	%ax, -0x32(%bp)
18:1C45  lesw	0x6(%bp), %bx
18:1C48  lesw	%es:0x8e(%bx), %bx
18:1C4D  movw	%es, -0xc(%bp)
18:1C50  movw	%bx, -0xe(%bp)
18:1C53  pushl	%es:0x4(%bx)
18:1C58  lesw	%es:0x4(%bx), %bx
18:1C5C  lesw	%es:0x2(%bx), %bx
18:1C60  lcallw	*%es:0x4(%bx)
18:1C64  addw	$0x4, %sp
18:1C67  movw	%dx, -0x34(%bp)
18:1C6A  movw	%ax, -0x36(%bp)
18:1C6D  movw	-0x34(%bp), %ax
18:1C70  cmpw	-0x30(%bp), %ax
18:1C73  jle	0x1c7b <.text+0x1c7b>
18:1C75  incw	-0x2(%bp)
18:1C78  incw	-0x6(%bp)
18:1C7B  movw	-0x4(%bp), %ax
18:1C7E  cmpw	-0x8(%bp), %ax
18:1C81  jge	0x1ce0 <.text+0x1ce0>
18:1C83  movw	-0x2(%bp), %ax
18:1C86  incw	%ax
18:1C87  cmpw	$0x8, %ax
18:1C8A  jge	0x1e03 <.text+0x1e03>
18:1C8E  movw	-0x8(%bp), %ax
18:1C91  shlw	$0x5, %ax
18:1C94  lesw	0x6(%bp), %bx
18:1C97  addw	%ax, %bx
18:1C99  movw	-0x2(%bp), %ax
18:1C9C  incw	%ax
18:1C9D  shlw	$0x2, %ax
18:1CA0  addw	%ax, %bx
18:1CA2  lesw	%es:0x104(%bx), %bx
18:1CA7  cmpw	$0x0, %es:0x8(%bx)
18:1CAC  jne	0x1e08 <.text+0x1e08>
18:1CB0  lesw	0x6(%bp), %bx
18:1CB3  lesw	%es:0x8e(%bx), %bx
18:1CB8  movw	%es, -0x10(%bp)
18:1CBB  movw	%bx, -0x12(%bp)
18:1CBE  movw	%es:0xa(%bx), %ax
18:1CC2  lesw	%es:0x1e(%bx), %bx
18:1CC6  movw	%ax, %es:0xa(%bx)
18:1CCA  lesw	0x6(%bp), %bx
18:1CCD  lesw	%es:0x8e(%bx), %bx
18:1CD2  movw	%es, -0x14(%bp)
18:1CD5  movw	%bx, -0x16(%bp)
18:1CD8  movw	%es:0xc(%bx), %ax
18:1CDC  incw	%ax
18:1CDD  jmp	0x1df9 <.text+0x1df9>
18:1CE0  movw	-0x4(%bp), %ax
18:1CE3  cmpw	-0x8(%bp), %ax
18:1CE6  jle	0x1d42 <.text+0x1d42>
18:1CE8  movw	-0x2(%bp), %ax
18:1CEB  decw	%ax
18:1CEC  jl	0x1e03 <.text+0x1e03>
18:1CF0  movw	-0x8(%bp), %ax
18:1CF3  shlw	$0x5, %ax
18:1CF6  lesw	0x6(%bp), %bx
18:1CF9  addw	%ax, %bx
18:1CFB  movw	-0x2(%bp), %ax
18:1CFE  decw	%ax
18:1CFF  shlw	$0x2, %ax
18:1D02  addw	%ax, %bx
18:1D04  lesw	%es:0x104(%bx), %bx
18:1D09  cmpw	$0x0, %es:0x8(%bx)
18:1D0E  jne	0x1e08 <.text+0x1e08>
18:1D12  lesw	0x6(%bp), %bx
18:1D15  lesw	%es:0x8e(%bx), %bx
18:1D1A  movw	%es, -0x18(%bp)
18:1D1D  movw	%bx, -0x1a(%bp)
18:1D20  movw	%es:0xa(%bx), %ax
18:1D24  lesw	%es:0x1e(%bx), %bx
18:1D28  movw	%ax, %es:0xa(%bx)
18:1D2C  lesw	0x6(%bp), %bx
18:1D2F  lesw	%es:0x8e(%bx), %bx
18:1D34  movw	%es, -0x1c(%bp)
18:1D37  movw	%bx, -0x1e(%bp)
18:1D3A  movw	%es:0xc(%bx), %ax
18:1D3E  decw	%ax
18:1D3F  jmp	0x1df9 <.text+0x1df9>
18:1D42  movw	-0x2(%bp), %ax
18:1D45  cmpw	-0x6(%bp), %ax
18:1D48  jge	0x1da3 <.text+0x1da3>
18:1D4A  movw	-0x4(%bp), %ax
18:1D4D  decw	%ax
18:1D4E  jl	0x1e03 <.text+0x1e03>
18:1D52  movw	-0x4(%bp), %ax
18:1D55  decw	%ax
18:1D56  shlw	$0x5, %ax
18:1D59  lesw	0x6(%bp), %bx
18:1D5C  addw	%ax, %bx
18:1D5E  movw	-0x2(%bp), %ax
18:1D61  shlw	$0x2, %ax
18:1D64  addw	%ax, %bx
18:1D66  lesw	%es:0x104(%bx), %bx
18:1D6B  cmpw	$0x0, %es:0x8(%bx)
18:1D70  jne	0x1e08 <.text+0x1e08>
18:1D74  lesw	0x6(%bp), %bx
18:1D77  lesw	%es:0x8e(%bx), %bx
18:1D7C  movw	%es, -0x20(%bp)
18:1D7F  movw	%bx, -0x22(%bp)
18:1D82  movw	%es:0xa(%bx), %ax
18:1D86  decw	%ax
18:1D87  lesw	%es:0x1e(%bx), %bx
18:1D8B  movw	%ax, %es:0xa(%bx)
18:1D8F  lesw	0x6(%bp), %bx
18:1D92  lesw	%es:0x8e(%bx), %bx
18:1D97  movw	%es, -0x24(%bp)
18:1D9A  movw	%bx, -0x26(%bp)
18:1D9D  movw	%es:0xc(%bx), %ax
18:1DA1  jmp	0x1df9 <.text+0x1df9>
18:1DA3  movw	-0x4(%bp), %ax
18:1DA6  incw	%ax
18:1DA7  cmpw	$0x5, %ax
18:1DAA  jge	0x1e03 <.text+0x1e03>
18:1DAC  movw	-0x4(%bp), %ax
18:1DAF  incw	%ax
18:1DB0  shlw	$0x5, %ax
18:1DB3  lesw	0x6(%bp), %bx
18:1DB6  addw	%ax, %bx
18:1DB8  movw	-0x2(%bp), %ax
18:1DBB  shlw	$0x2, %ax
18:1DBE  addw	%ax, %bx
18:1DC0  lesw	%es:0x104(%bx), %bx
18:1DC5  cmpw	$0x0, %es:0x8(%bx)
18:1DCA  jne	0x1e08 <.text+0x1e08>
18:1DCC  lesw	0x6(%bp), %bx
18:1DCF  lesw	%es:0x8e(%bx), %bx
18:1DD4  movw	%es, -0x28(%bp)
18:1DD7  movw	%bx, -0x2a(%bp)
18:1DDA  movw	%es:0xa(%bx), %ax
18:1DDE  incw	%ax
18:1DDF  lesw	%es:0x1e(%bx), %bx
18:1DE3  movw	%ax, %es:0xa(%bx)
18:1DE7  lesw	0x6(%bp), %bx
18:1DEA  lesw	%es:0x8e(%bx), %bx
18:1DEF  movw	%es, -0x2c(%bp)
18:1DF2  movw	%bx, -0x2e(%bp)
18:1DF5  movw	%es:0xc(%bx), %ax
18:1DF9  lesw	%es:0x1e(%bx), %bx
18:1DFD  movw	%ax, %es:0xc(%bx)
18:1E01  jmp	0x1e08 <.text+0x1e08>
18:1E03  movw	$0x0, -0xa(%bp)
18:1E08  cmpw	$0x0, -0xa(%bp)
18:1E0C  je	0x1e4a <.text+0x1e4a>
18:1E0E  lesw	0x6(%bp), %bx
18:1E11  lesw	%es:0x8e(%bx), %bx
18:1E16  lesw	%es:0x1e(%bx), %bx
18:1E1A  movl	%es:0xa(%bx), %eax
18:1E1F  movl	%eax, -0x8(%bp)
18:1E23  pushw	$0x1
18:1E25  movw	0x8(%bp), %dx
18:1E28  movw	0x6(%bp), %ax
18:1E2B  addw	$0x92, %ax
18:1E2E  movw	$0x8, %cx
18:1E31  lcallw	$0x0, $0xffff           # imm = 0xFFFF ; NE 1E32=1:3E86
18:1E36  pushl	-0x8(%bp)
18:1E3A  lesw	0x6(%bp), %bx
18:1E3D  pushl	%es:0x8e(%bx)
18:1E43  pushw	%cs
18:1E44  callw	0x528 <.text+0x528>
18:1E47  addw	$0x12, %sp
18:1E4A  popw	%di
18:1E4B  popw	%si
18:1E4C  leave
18:1E4D  decw	%bp
18:1E4E  lretw
18:1E4F  incw	%bp
18:1E50  pushw	%bp
18:1E51  movw	%sp, %bp
18:1E53  subw	$0x36, %sp
18:1E56  pushw	%si
18:1E57  pushw	%di
18:1E58  movw	$0x1, -0xa(%bp)
18:1E5D  lesw	0x6(%bp), %bx
18:1E60  cmpw	$0x0, %es:0xa8(%bx)
18:1E66  jne	0x20ad <.text+0x20ad>
18:1E6A  pushl	$0x0
18:1E6D  pushl	$0x14394                # imm = 0x14394
18:1E73  pushl	%es:0x86(%bx)
18:1E79  lcallw	$0x0, $0xffff           # imm = 0xFFFF ; NE 1E7A=36:28F7
18:1E7E  addw	$0xc, %sp
18:1E81  lesw	0x6(%bp), %bx
18:1E84  lesw	%es:0x8e(%bx), %bx
18:1E89  lesw	%es:0x1e(%bx), %bx
18:1E8D  cmpw	$0x0, %es:0x8(%bx)
18:1E92  je	0x20ad <.text+0x20ad>
18:1E96  lesw	0x6(%bp), %bx
18:1E99  lesw	%es:0x8e(%bx), %bx
18:1E9E  movl	%es:0xa(%bx), %eax
18:1EA3  movl	%eax, -0x4(%bp)
18:1EA7  lesw	%es:0x1e(%bx), %bx
18:1EAB  movl	%es:0xa(%bx), %eax
18:1EB0  movl	%eax, -0x8(%bp)
18:1EB4  pushl	-0x4(%bp)
18:1EB8  pushl	0x6(%bp)
18:1EBC  pushw	%cs
18:1EBD  callw	0x1b5d <.text+0x1b5d>
18:1EC0  addw	$0x8, %sp
18:1EC3  movw	%dx, -0x30(%bp)
18:1EC6  movw	%ax, -0x32(%bp)
18:1EC9  lesw	0x6(%bp), %bx
18:1ECC  lesw	%es:0x8e(%bx), %bx
18:1ED1  movw	%es, -0xc(%bp)
18:1ED4  movw	%bx, -0xe(%bp)
18:1ED7  pushl	%es:0x4(%bx)
18:1EDC  lesw	%es:0x4(%bx), %bx
18:1EE0  lesw	%es:0x2(%bx), %bx
18:1EE4  lcallw	*%es:0x4(%bx)
18:1EE8  addw	$0x4, %sp
18:1EEB  movw	%dx, -0x34(%bp)
18:1EEE  movw	%ax, -0x36(%bp)
18:1EF1  movw	-0x34(%bp), %ax
18:1EF4  cmpw	-0x30(%bp), %ax
18:1EF7  jl	0x1eff <.text+0x1eff>
18:1EF9  incw	-0x2(%bp)
18:1EFC  incw	-0x6(%bp)
18:1EFF  movw	-0x4(%bp), %ax
18:1F02  cmpw	-0x8(%bp), %ax
18:1F05  jge	0x1f47 <.text+0x1f47>
18:1F07  cmpw	$0x0, -0x2(%bp)
18:1F0B  jle	0x2066 <.text+0x2066>
18:1F0F  lesw	0x6(%bp), %bx
18:1F12  lesw	%es:0x8e(%bx), %bx
18:1F17  movw	%es, -0x10(%bp)
18:1F1A  movw	%bx, -0x12(%bp)
18:1F1D  movw	%es:0xa(%bx), %ax
18:1F21  lesw	%es:0x1e(%bx), %bx
18:1F25  movw	%ax, %es:0xa(%bx)
18:1F29  lesw	0x6(%bp), %bx
18:1F2C  lesw	%es:0x8e(%bx), %bx
18:1F31  movw	%es, -0x14(%bp)
18:1F34  movw	%bx, -0x16(%bp)
18:1F37  movw	%es:0xc(%bx), %ax
18:1F3B  decw	%ax
18:1F3C  lesw	%es:0x1e(%bx), %bx
18:1F40  movw	%ax, %es:0xc(%bx)
18:1F44  jmp	0x206b <.text+0x206b>
18:1F47  movw	-0x4(%bp), %ax
18:1F4A  cmpw	-0x8(%bp), %ax
18:1F4D  jle	0x1fab <.text+0x1fab>
18:1F4F  movw	-0x2(%bp), %ax
18:1F52  incw	%ax
18:1F53  cmpw	$0x8, %ax
18:1F56  jge	0x2066 <.text+0x2066>
18:1F5A  movw	-0x8(%bp), %ax
18:1F5D  shlw	$0x5, %ax
18:1F60  lesw	0x6(%bp), %bx
18:1F63  addw	%ax, %bx
18:1F65  movw	-0x2(%bp), %ax
18:1F68  incw	%ax
18:1F69  shlw	$0x2, %ax
18:1F6C  addw	%ax, %bx
18:1F6E  lesw	%es:0x104(%bx), %bx
18:1F73  cmpw	$0x0, %es:0x8(%bx)
18:1F78  jne	0x206b <.text+0x206b>
18:1F7C  lesw	0x6(%bp), %bx
18:1F7F  lesw	%es:0x8e(%bx), %bx
18:1F84  movw	%es, -0x18(%bp)
18:1F87  movw	%bx, -0x1a(%bp)
18:1F8A  movw	%es:0xa(%bx), %ax
18:1F8E  lesw	%es:0x1e(%bx), %bx
18:1F92  movw	%ax, %es:0xa(%bx)
18:1F96  lesw	0x6(%bp), %bx
18:1F99  lesw	%es:0x8e(%bx), %bx
18:1F9E  movw	%es, -0x1c(%bp)
18:1FA1  movw	%bx, -0x1e(%bp)
18:1FA4  movw	%es:0xc(%bx), %ax
18:1FA8  incw	%ax
18:1FA9  jmp	0x1f3c <.text+0x1f3c>
18:1FAB  movw	-0x2(%bp), %ax
18:1FAE  cmpw	-0x6(%bp), %ax
18:1FB1  jge	0x2010 <.text+0x2010>
18:1FB3  movw	-0x4(%bp), %ax
18:1FB6  incw	%ax
18:1FB7  cmpw	$0x5, %ax
18:1FBA  jge	0x2066 <.text+0x2066>
18:1FBE  movw	-0x4(%bp), %ax
18:1FC1  incw	%ax
18:1FC2  shlw	$0x5, %ax
18:1FC5  lesw	0x6(%bp), %bx
18:1FC8  addw	%ax, %bx
18:1FCA  movw	-0x2(%bp), %ax
18:1FCD  shlw	$0x2, %ax
18:1FD0  addw	%ax, %bx
18:1FD2  lesw	%es:0x104(%bx), %bx
18:1FD7  cmpw	$0x0, %es:0x8(%bx)
18:1FDC  jne	0x206b <.text+0x206b>
18:1FE0  lesw	0x6(%bp), %bx
18:1FE3  lesw	%es:0x8e(%bx), %bx
18:1FE8  movw	%es, -0x20(%bp)
18:1FEB  movw	%bx, -0x22(%bp)
18:1FEE  movw	%es:0xa(%bx), %ax
18:1FF2  incw	%ax
18:1FF3  lesw	%es:0x1e(%bx), %bx
18:1FF7  movw	%ax, %es:0xa(%bx)
18:1FFB  lesw	0x6(%bp), %bx
18:1FFE  lesw	%es:0x8e(%bx), %bx
18:2003  movw	%es, -0x24(%bp)
18:2006  movw	%bx, -0x26(%bp)
18:2009  movw	%es:0xc(%bx), %ax
18:200D  jmp	0x1f3c <.text+0x1f3c>
18:2010  movw	-0x4(%bp), %ax
18:2013  decw	%ax
18:2014  jl	0x2066 <.text+0x2066>
18:2016  movw	-0x4(%bp), %ax
18:2019  decw	%ax
18:201A  shlw	$0x5, %ax
18:201D  lesw	0x6(%bp), %bx
18:2020  addw	%ax, %bx
18:2022  movw	-0x2(%bp), %ax
18:2025  shlw	$0x2, %ax
18:2028  addw	%ax, %bx
18:202A  lesw	%es:0x104(%bx), %bx
18:202F  cmpw	$0x0, %es:0x8(%bx)
18:2034  jne	0x206b <.text+0x206b>
18:2036  lesw	0x6(%bp), %bx
18:2039  lesw	%es:0x8e(%bx), %bx
18:203E  movw	%es, -0x28(%bp)
18:2041  movw	%bx, -0x2a(%bp)
18:2044  movw	%es:0xa(%bx), %ax
18:2048  decw	%ax
18:2049  lesw	%es:0x1e(%bx), %bx
18:204D  movw	%ax, %es:0xa(%bx)
18:2051  lesw	0x6(%bp), %bx
18:2054  lesw	%es:0x8e(%bx), %bx
18:2059  movw	%es, -0x2c(%bp)
18:205C  movw	%bx, -0x2e(%bp)
18:205F  movw	%es:0xc(%bx), %ax
18:2063  jmp	0x1f3c <.text+0x1f3c>
18:2066  movw	$0x0, -0xa(%bp)
18:206B  cmpw	$0x0, -0xa(%bp)
18:206F  je	0x20ad <.text+0x20ad>
18:2071  lesw	0x6(%bp), %bx
18:2074  lesw	%es:0x8e(%bx), %bx
18:2079  lesw	%es:0x1e(%bx), %bx
18:207D  movl	%es:0xa(%bx), %eax
18:2082  movl	%eax, -0x8(%bp)
18:2086  pushw	$0x1
18:2088  movw	0x8(%bp), %dx
18:208B  movw	0x6(%bp), %ax
18:208E  addw	$0x92, %ax
18:2091  movw	$0x8, %cx
18:2094  lcallw	$0x0, $0xffff           # imm = 0xFFFF ; NE 2095=1:3E86
18:2099  pushl	-0x8(%bp)
18:209D  lesw	0x6(%bp), %bx
18:20A0  pushl	%es:0x8e(%bx)
18:20A6  pushw	%cs
18:20A7  callw	0x528 <.text+0x528>
18:20AA  addw	$0x12, %sp
18:20AD  popw	%di
18:20AE  popw	%si
18:20AF  leave
18:20B0  decw	%bp
18:20B1  lretw
18:20B2  incw	%bp
18:20B3  pushw	%bp
18:20B4  movw	%sp, %bp
18:20B6  subw	$0x8, %sp
18:20B9  pushw	%si
18:20BA  pushw	%di
18:20BB  lesw	0x6(%bp), %bx
18:20BE  lesw	%es:0x8e(%bx), %bx
18:20C3  movl	%es:0xa(%bx), %eax
18:20C8  movl	%eax, -0x4(%bp)
18:20CC  lesw	%es:0x1e(%bx), %bx
18:20D0  movl	%es:0xa(%bx), %eax
18:20D5  movl	%eax, -0x8(%bp)
18:20D9  pushl	$0x0
18:20DC  pushl	$0x14392                # imm = 0x14392
18:20E2  lesw	0x6(%bp), %bx
18:20E5  pushl	%es:0x86(%bx)
18:20EB  lcallw	$0x0, $0xffff           # imm = 0xFFFF ; NE 20EC=36:28F7
18:20F0  addw	$0xc, %sp
18:20F3  lesw	0x6(%bp), %bx
18:20F6  lesw	%es:0x8e(%bx), %bx
18:20FB  lesw	%es:0x1e(%bx), %bx
18:20FF  movw	%es:0xa(%bx), %ax
18:2103  decw	%ax
18:2104  jl	0x21a3 <.text+0x21a3>
18:2108  movw	-0x4(%bp), %ax
18:210B  decw	%ax
18:210C  jl	0x21a3 <.text+0x21a3>
18:2110  movw	-0x4(%bp), %ax
18:2113  decw	%ax
18:2114  shlw	$0x5, %ax
18:2117  lesw	0x6(%bp), %bx
18:211A  addw	%ax, %bx
18:211C  movw	-0x2(%bp), %ax
18:211F  shlw	$0x2, %ax
18:2122  addw	%ax, %bx
18:2124  lesw	%es:0x104(%bx), %bx
18:2129  cmpw	$0x0, %es:0x8(%bx)
18:212E  jne	0x21a3 <.text+0x21a3>
18:2130  movw	-0x8(%bp), %ax
18:2133  decw	%ax
18:2134  shlw	$0x5, %ax
18:2137  lesw	0x6(%bp), %bx
18:213A  addw	%ax, %bx
18:213C  movw	-0x6(%bp), %ax
18:213F  shlw	$0x2, %ax
18:2142  addw	%ax, %bx
18:2144  lesw	%es:0x104(%bx), %bx
18:2149  cmpw	$0x0, %es:0x8(%bx)
18:214E  je	0x2163 <.text+0x2163>
18:2150  lesw	0x6(%bp), %bx
18:2153  lesw	%es:0x8e(%bx), %bx
18:2158  lesw	%es:0x1e(%bx), %bx
18:215C  cmpw	$0x0, %es:0x8(%bx)
18:2161  jne	0x21a3 <.text+0x21a3>
18:2163  lesw	0x6(%bp), %bx
18:2166  lesw	%es:0x8e(%bx), %bx
18:216B  decw	%es:0xa(%bx)
18:216F  lesw	%es:0x1e(%bx), %bx
18:2173  decw	%es:0xa(%bx)
18:2177  movw	$0xffff, -0x4(%bp)      # imm = 0xFFFF
18:217C  pushw	$0x0
18:217E  movw	0x8(%bp), %dx
18:2181  movw	0x6(%bp), %ax
18:2184  addw	$0x92, %ax
18:2187  movw	$0x8, %cx
18:218A  lcallw	$0x0, $0xffff           # imm = 0xFFFF ; NE 218B=1:3E86
18:218F  pushl	-0x4(%bp)
18:2193  lesw	0x6(%bp), %bx
18:2196  pushl	%es:0x8e(%bx)
18:219C  pushw	%cs
18:219D  callw	0x528 <.text+0x528>
18:21A0  addw	$0x12, %sp
18:21A3  popw	%di
18:21A4  popw	%si
18:21A5  leave
18:21A6  decw	%bp
18:21A7  lretw
18:21A8  incw	%bp
18:21A9  pushw	%bp
18:21AA  movw	%sp, %bp
18:21AC  subw	$0x8, %sp
18:21AF  pushw	%si
18:21B0  pushw	%di
18:21B1  lesw	0x6(%bp), %bx
18:21B4  lesw	%es:0x8e(%bx), %bx
18:21B9  movl	%es:0xa(%bx), %eax
18:21BE  movl	%eax, -0x4(%bp)
18:21C2  lesw	%es:0x1e(%bx), %bx
18:21C6  movl	%es:0xa(%bx), %eax
18:21CB  movl	%eax, -0x8(%bp)
18:21CF  pushl	$0x0
18:21D2  pushl	$0x14392                # imm = 0x14392
18:21D8  lesw	0x6(%bp), %bx
18:21DB  pushl	%es:0x86(%bx)
18:21E1  lcallw	$0x0, $0xffff           # imm = 0xFFFF ; NE 21E2=36:28F7
18:21E6  addw	$0xc, %sp
18:21E9  lesw	0x6(%bp), %bx
18:21EC  lesw	%es:0x8e(%bx), %bx
18:21F1  lesw	%es:0x1e(%bx), %bx
18:21F5  movw	%es:0xa(%bx), %ax
18:21F9  incw	%ax
18:21FA  cmpw	$0x5, %ax
18:21FD  jge	0x229f <.text+0x229f>
18:2201  movw	-0x4(%bp), %ax
18:2204  incw	%ax
18:2205  cmpw	$0x5, %ax
18:2208  jge	0x229f <.text+0x229f>
18:220C  movw	-0x4(%bp), %ax
18:220F  incw	%ax
18:2210  shlw	$0x5, %ax
18:2213  lesw	0x6(%bp), %bx
18:2216  addw	%ax, %bx
18:2218  movw	-0x2(%bp), %ax
18:221B  shlw	$0x2, %ax
18:221E  addw	%ax, %bx
18:2220  lesw	%es:0x104(%bx), %bx
18:2225  cmpw	$0x0, %es:0x8(%bx)
18:222A  jne	0x229f <.text+0x229f>
18:222C  movw	-0x8(%bp), %ax
18:222F  incw	%ax
18:2230  shlw	$0x5, %ax
18:2233  lesw	0x6(%bp), %bx
18:2236  addw	%ax, %bx
18:2238  movw	-0x6(%bp), %ax
18:223B  shlw	$0x2, %ax
18:223E  addw	%ax, %bx
18:2240  lesw	%es:0x104(%bx), %bx
18:2245  cmpw	$0x0, %es:0x8(%bx)
18:224A  je	0x225f <.text+0x225f>
18:224C  lesw	0x6(%bp), %bx
18:224F  lesw	%es:0x8e(%bx), %bx
18:2254  lesw	%es:0x1e(%bx), %bx
18:2258  cmpw	$0x0, %es:0x8(%bx)
18:225D  jne	0x229f <.text+0x229f>
18:225F  lesw	0x6(%bp), %bx
18:2262  lesw	%es:0x8e(%bx), %bx
18:2267  incw	%es:0xa(%bx)
18:226B  lesw	%es:0x1e(%bx), %bx
18:226F  incw	%es:0xa(%bx)
18:2273  movw	$0xffff, -0x4(%bp)      # imm = 0xFFFF
18:2278  pushw	$0x0
18:227A  movw	0x8(%bp), %dx
18:227D  movw	0x6(%bp), %ax
18:2280  addw	$0x92, %ax
18:2283  movw	$0x8, %cx
18:2286  lcallw	$0x0, $0xffff           # imm = 0xFFFF ; NE 2287=1:3E86
18:228B  pushl	-0x4(%bp)
18:228F  lesw	0x6(%bp), %bx
18:2292  pushl	%es:0x8e(%bx)
18:2298  pushw	%cs
18:2299  callw	0x528 <.text+0x528>
18:229C  addw	$0x12, %sp
18:229F  popw	%di
18:22A0  popw	%si
18:22A1  leave
18:22A2  decw	%bp
18:22A3  lretw
```

### GW: gravidade e fixação (18:22A4-2A29)

```asm
18:22A4  incw	%bp
18:22A5  pushw	%bp
18:22A6  movw	%sp, %bp
18:22A8  subw	$0x58, %sp
18:22AB  pushw	%si
18:22AC  pushw	%di
18:22AD  movw	$0x1, -0x2(%bp)
18:22B2  lesw	0x6(%bp), %bx
18:22B5  pushl	%es:0x8e(%bx)
18:22BB  pushw	%cs
18:22BC  callw	0x4c8 <.text+0x4c8>
18:22BF  addw	$0x4, %sp
18:22C2  lesw	0x6(%bp), %bx
18:22C5  movw	%es:0x8e(%bx), %ax
18:22CA  addw	$0xe, %ax
18:22CD  pushw	%es:0x90(%bx)
18:22D2  pushw	%ax
18:22D3  lcallw	$0x0, $0xffff           # imm = 0xFFFF ; NE 22D4=11:224F
18:22D8  addw	$0x4, %sp
18:22DB  orw	%ax, %ax
18:22DD  je	0x2a25 <.text+0x2a25>
18:22E1  lesw	0x6(%bp), %bx
18:22E4  lesw	%es:0x8e(%bx), %bx
18:22E9  cmpw	$0x0, %es:0x28(%bx)
18:22EE  je	0x2a25 <.text+0x2a25>
18:22F2  movl	%es:0xa(%bx), %eax
18:22F7  movl	%eax, -0x6(%bp)
18:22FB  lesw	%es:0x1e(%bx), %bx
18:22FF  movl	%es:0xa(%bx), %eax
18:2304  movl	%eax, -0xa(%bp)
18:2308  lesw	0x6(%bp), %bx
18:230B  lesw	%es:0x8e(%bx), %bx
18:2310  movw	%es, -0x24(%bp)
18:2313  movw	%bx, -0x26(%bp)
18:2316  pushl	%es:0x4(%bx)
18:231B  lesw	%es:0x4(%bx), %bx
18:231F  lesw	%es:0x2(%bx), %bx
18:2323  lcallw	*%es:0x4(%bx)
18:2327  addw	$0x4, %sp
18:232A  movw	%dx, -0xc(%bp)
18:232D  movw	%ax, -0xe(%bp)
18:2330  lesw	0x6(%bp), %bx
18:2333  lesw	%es:0x8e(%bx), %bx
18:2338  movw	%es, -0x28(%bp)
18:233B  movw	%bx, -0x2a(%bp)
18:233E  lesw	%es:0x1e(%bx), %bx
18:2342  pushl	%es:0x4(%bx)
18:2347  lesw	-0x2a(%bp), %bx
18:234A  lesw	%es:0x1e(%bx), %bx
18:234E  lesw	%es:0x4(%bx), %bx
18:2352  lesw	%es:0x2(%bx), %bx
18:2356  lcallw	*%es:0x4(%bx)
18:235A  addw	$0x4, %sp
18:235D  movw	%dx, -0x10(%bp)
18:2360  movw	%ax, -0x12(%bp)
18:2363  movw	-0x4(%bp), %ax
18:2366  incw	%ax
18:2367  pushw	%ax
18:2368  pushw	-0x6(%bp)
18:236B  pushl	0x6(%bp)
18:236F  pushw	%cs
18:2370  callw	0x1b5d <.text+0x1b5d>
18:2373  addw	$0x8, %sp
18:2376  movw	%dx, -0x14(%bp)
18:2379  movw	%ax, -0x16(%bp)
18:237C  movw	-0x8(%bp), %ax
18:237F  incw	%ax
18:2380  pushw	%ax
18:2381  pushw	-0x6(%bp)
18:2384  pushl	0x6(%bp)
18:2388  pushw	%cs
18:2389  callw	0x1b5d <.text+0x1b5d>
18:238C  addw	$0x8, %sp
18:238F  movw	%dx, -0x18(%bp)
18:2392  movw	%ax, -0x1a(%bp)
18:2395  lesw	0x6(%bp), %bx
18:2398  lesw	%es:0x1a4(%bx), %bx
18:239D  cmpl	$-0x1, %es:0x42(%bx)
18:23A3  jne	0x23af <.text+0x23af>
18:23A5  addw	$0x5, -0xc(%bp)
18:23A9  addw	$0x5, -0x10(%bp)
18:23AD  jmp	0x23be <.text+0x23be>
18:23AF  movw	-0x14(%bp), %ax
18:23B2  subw	-0xc(%bp), %ax
18:23B5  addw	%ax, -0x10(%bp)
18:23B8  movw	-0x14(%bp), %ax
18:23BB  movw	%ax, -0xc(%bp)
18:23BE  movw	-0xa(%bp), %ax
18:23C1  shlw	$0x5, %ax
18:23C4  lesw	0x6(%bp), %bx
18:23C7  addw	%ax, %bx
18:23C9  movw	-0x8(%bp), %ax
18:23CC  incw	%ax
18:23CD  shlw	$0x2, %ax
18:23D0  addw	%ax, %bx
18:23D2  lesw	%es:0x104(%bx), %bx
18:23D7  cmpw	$0x0, %es:0x8(%bx)
18:23DC  jne	0x23fe <.text+0x23fe>
18:23DE  movw	-0x6(%bp), %ax
18:23E1  shlw	$0x5, %ax
18:23E4  lesw	0x6(%bp), %bx
18:23E7  addw	%ax, %bx
18:23E9  movw	-0x4(%bp), %ax
18:23EC  incw	%ax
18:23ED  shlw	$0x2, %ax
18:23F0  addw	%ax, %bx
18:23F2  lesw	%es:0x104(%bx), %bx
18:23F7  cmpw	$0x0, %es:0x8(%bx)
18:23FC  je	0x2409 <.text+0x2409>
18:23FE  movw	$0x1, -0x2(%bp)
18:2403  movw	-0x14(%bp), %ax
18:2406  movw	%ax, -0xc(%bp)
18:2409  movw	-0xc(%bp), %ax
18:240C  cmpw	-0x14(%bp), %ax
18:240F  jge	0x241b <.text+0x241b>
18:2411  movw	-0x10(%bp), %ax
18:2414  cmpw	-0x18(%bp), %ax
18:2417  jl	0x25a8 <.text+0x25a8>
18:241B  movw	-0x8(%bp), %ax
18:241E  incw	%ax
18:241F  cmpw	$0x8, %ax
18:2422  jge	0x2534 <.text+0x2534>
18:2426  movw	-0x4(%bp), %ax
18:2429  incw	%ax
18:242A  cmpw	$0x8, %ax
18:242D  jge	0x2534 <.text+0x2534>
18:2431  lesw	0x6(%bp), %bx
18:2434  lesw	%es:0x8e(%bx), %bx
18:2439  movw	%es, -0x2c(%bp)
18:243C  movw	%bx, -0x2e(%bp)
18:243F  lesw	%es:0x1e(%bx), %bx
18:2443  movw	%es:0xa(%bx), %ax
18:2447  shlw	$0x5, %ax
18:244A  lesw	0x6(%bp), %bx
18:244D  addw	%ax, %bx
18:244F  pushw	%es
18:2450  lesw	-0x2e(%bp), %si
18:2453  lesw	%es:0x1e(%si), %si
18:2457  movw	%es:0xc(%si), %ax
18:245B  incw	%ax
18:245C  shlw	$0x2, %ax
18:245F  addw	%ax, %bx
18:2461  popw	%es
18:2462  lesw	%es:0x104(%bx), %bx
18:2467  cmpw	$0x0, %es:0x8(%bx)
18:246C  je	0x2483 <.text+0x2483>
18:246E  lesw	0x6(%bp), %bx
18:2471  lesw	%es:0x8e(%bx), %bx
18:2476  lesw	%es:0x1e(%bx), %bx
18:247A  cmpw	$0x0, %es:0x8(%bx)
18:247F  jne	0x2534 <.text+0x2534>
18:2483  movw	-0x6(%bp), %ax
18:2486  shlw	$0x5, %ax
18:2489  lesw	0x6(%bp), %bx
18:248C  addw	%ax, %bx
18:248E  movw	-0x4(%bp), %ax
18:2491  incw	%ax
18:2492  shlw	$0x2, %ax
18:2495  addw	%ax, %bx
18:2497  lesw	%es:0x104(%bx), %bx
18:249C  cmpw	$0x0, %es:0x8(%bx)
18:24A1  jne	0x2534 <.text+0x2534>
18:24A5  lesw	0x6(%bp), %bx
18:24A8  lesw	%es:0x8e(%bx), %bx
18:24AD  incw	%es:0xc(%bx)
18:24B1  lesw	%es:0x1e(%bx), %bx
18:24B5  cmpw	$0x0, %es:0x8(%bx)
18:24BA  jne	0x24f3 <.text+0x24f3>
18:24BC  lesw	0x6(%bp), %bx
18:24BF  lesw	%es:0x8e(%bx), %bx
18:24C4  movw	%es, -0x30(%bp)
18:24C7  movw	%bx, -0x32(%bp)
18:24CA  movw	%es:0xc(%bx), %ax
18:24CE  decw	%ax
18:24CF  lesw	%es:0x1e(%bx), %bx
18:24D3  movw	%ax, %es:0xc(%bx)
18:24D7  lesw	0x6(%bp), %bx
18:24DA  lesw	%es:0x8e(%bx), %bx
18:24DF  movw	%es, -0x34(%bp)
18:24E2  movw	%bx, -0x36(%bp)
18:24E5  movw	%es:0xa(%bx), %ax
18:24E9  lesw	%es:0x1e(%bx), %bx
18:24ED  movw	%ax, %es:0xa(%bx)
18:24F1  jmp	0x2503 <.text+0x2503>
18:24F3  lesw	0x6(%bp), %bx
18:24F6  lesw	%es:0x8e(%bx), %bx
18:24FB  lesw	%es:0x1e(%bx), %bx
18:24FF  incw	%es:0xc(%bx)
18:2503  movw	$0x0, -0x2(%bp)
18:2508  movw	$0xffff, -0x6(%bp)      # imm = 0xFFFF
18:250D  pushw	$0x1
18:250F  movw	0x8(%bp), %dx
18:2512  movw	0x6(%bp), %ax
18:2515  addw	$0x92, %ax
18:2518  movw	$0x8, %cx
18:251B  lcallw	$0x0, $0xffff           # imm = 0xFFFF ; NE 251C=1:3E86
18:2520  pushl	-0x6(%bp)
18:2524  lesw	0x6(%bp), %bx
18:2527  pushl	%es:0x8e(%bx)
18:252D  pushw	%cs
18:252E  callw	0x528 <.text+0x528>
18:2531  addw	$0x12, %sp
18:2534  lesw	0x6(%bp), %bx
18:2537  lesw	%es:0x8e(%bx), %bx
18:253C  movl	%es:0xa(%bx), %eax
18:2541  movl	%eax, -0x6(%bp)
18:2545  lesw	%es:0x1e(%bx), %bx
18:2549  movl	%es:0xa(%bx), %eax
18:254E  movl	%eax, -0xa(%bp)
18:2552  cmpw	$0x8, -0x8(%bp)
18:2556  jge	0x255e <.text+0x255e>
18:2558  cmpw	$0x8, -0x4(%bp)
18:255C  jl	0x2566 <.text+0x2566>
18:255E  movw	$0x1, -0x2(%bp)
18:2563  jmp	0x260e <.text+0x260e>
18:2566  movw	-0xa(%bp), %ax
18:2569  shlw	$0x5, %ax
18:256C  lesw	0x6(%bp), %bx
18:256F  addw	%ax, %bx
18:2571  movw	-0x8(%bp), %ax
18:2574  incw	%ax
18:2575  shlw	$0x2, %ax
18:2578  addw	%ax, %bx
18:257A  lesw	%es:0x104(%bx), %bx
18:257F  cmpw	$0x0, %es:0x8(%bx)
18:2584  jne	0x255e <.text+0x255e>
18:2586  movw	-0x6(%bp), %ax
18:2589  shlw	$0x5, %ax
18:258C  lesw	0x6(%bp), %bx
18:258F  addw	%ax, %bx
18:2591  movw	-0x4(%bp), %ax
18:2594  incw	%ax
18:2595  shlw	$0x2, %ax
18:2598  addw	%ax, %bx
18:259A  lesw	%es:0x104(%bx), %bx
18:259F  cmpw	$0x0, %es:0x8(%bx)
18:25A4  je	0x260e <.text+0x260e>
18:25A6  jmp	0x255e <.text+0x255e>
18:25A8  movl	-0xe(%bp), %eax
18:25AC  movl	%eax, -0x1e(%bp)
18:25B0  pushl	%eax
18:25B2  lesw	0x6(%bp), %bx
18:25B5  lesw	%es:0x8e(%bx), %bx
18:25BA  movw	%es, -0x38(%bp)
18:25BD  movw	%bx, -0x3a(%bp)
18:25C0  pushl	%es:0x4(%bx)
18:25C5  lesw	%es:0x4(%bx), %bx
18:25C9  lesw	%es:0x2(%bx), %bx
18:25CD  lcallw	*%es:(%bx)
18:25D0  addw	$0x8, %sp
18:25D3  movl	-0x12(%bp), %eax
18:25D7  movl	%eax, -0x22(%bp)
18:25DB  pushl	%eax
18:25DD  lesw	0x6(%bp), %bx
18:25E0  lesw	%es:0x8e(%bx), %bx
18:25E5  movw	%es, -0x3c(%bp)
18:25E8  movw	%bx, -0x3e(%bp)
18:25EB  lesw	%es:0x1e(%bx), %bx
18:25EF  pushl	%es:0x4(%bx)
18:25F4  lesw	-0x3e(%bp), %bx
18:25F7  lesw	%es:0x1e(%bx), %bx
18:25FB  lesw	%es:0x4(%bx), %bx
18:25FF  lesw	%es:0x2(%bx), %bx
18:2603  lcallw	*%es:(%bx)
18:2606  addw	$0x8, %sp
18:2609  movw	$0x0, -0x2(%bp)
18:260E  cmpw	$0x0, -0x2(%bp)
18:2612  je	0x2a25 <.text+0x2a25>
18:2616  lesw	0x6(%bp), %bx
18:2619  pushl	%es:0x86(%bx)
18:261F  lcallw	$0x0, $0xffff           # imm = 0xFFFF ; NE 2620=36:2A8F
18:2624  addw	$0x4, %sp
18:2627  lesw	0x6(%bp), %bx
18:262A  lesw	%es:0x8e(%bx), %bx
18:262F  cmpw	$0x0, %es:0x16(%bx)
18:2634  je	0x26fc <.text+0x26fc>
18:2638  lesw	0x6(%bp), %bx
18:263B  pushl	%es:0x8e(%bx)
18:2641  pushw	%cs
18:2642  callw	0x2ba <.text+0x2ba>
18:2645  addw	$0x4, %sp
18:2648  lesw	0x6(%bp), %bx
18:264B  pushl	%es:0x1a4(%bx)
18:2651  lcallw	$0x0, $0xffff           # imm = 0xFFFF ; NE 2652=37:189F
18:2656  addw	$0x4, %sp
18:2659  movw	%dx, -0x48(%bp)
18:265C  movw	%ax, -0x4a(%bp)
18:265F  lesw	0x6(%bp), %bx
18:2662  lesw	%es:0x1a4(%bx), %bx
18:2667  cmpw	$0x1, %es:0x26(%bx)
18:266C  je	0x267c <.text+0x267c>
18:266E  cmpw	$0x2, %es:0x26(%bx)
18:2673  je	0x267c <.text+0x267c>
18:2675  cmpw	$0x3, %es:0x26(%bx)
18:267A  jne	0x2681 <.text+0x2681>
18:267C  movw	$0x1, %ax
18:267F  jmp	0x2683 <.text+0x2683>
18:2681  xorw	%ax, %ax
18:2683  orw	%ax, %ax
18:2685  je	0x2693 <.text+0x2693>
18:2687  pushl	-0x4a(%bp)
18:268B  lcallw	$0x0, $0xffff           # imm = 0xFFFF ; NE 268C=32:009A
18:2690  addw	$0x4, %sp
18:2693  cmpl	$0x0, -0x4a(%bp)
18:2698  je	0x26ae <.text+0x26ae>
18:269A  pushw	$0x3
18:269C  pushl	-0x4a(%bp)
18:26A0  lesw	-0x4a(%bp), %bx
18:26A3  lesw	%es:(%bx), %bx
18:26A6  lcallw	*%es:(%bx)
18:26A9  addw	$0x6, %sp
18:26AC  jmp	0x26ae <.text+0x26ae>
18:26AE  lesw	0x6(%bp), %bx
18:26B1  lesw	%es:0x8e(%bx), %bx
18:26B6  movw	%es:0xc(%bx), %ax
18:26BA  lesw	0x6(%bp), %bx
18:26BD  lesw	%es:0x1b0(%bx), %bx
18:26C2  movw	%ax, %es:0x42(%bx)
18:26C6  lesw	0x6(%bp), %bx
18:26C9  pushl	%es:0x1b0(%bx)
18:26CF  lcallw	$0x0, $0xffff           # imm = 0xFFFF ; NE 26D0=37:17EC
18:26D4  addw	$0x4, %sp
18:26D7  movw	%dx, -0x4c(%bp)
18:26DA  movw	%ax, -0x4e(%bp)
18:26DD  pushw	%dx
18:26DE  pushw	%ax
18:26DF  lcallw	$0x0, $0xffff           # imm = 0xFFFF ; NE 26E0=32:009A
18:26E4  addw	$0x4, %sp
18:26E7  cmpl	$0x0, -0x4e(%bp)
18:26EC  je	0x29e6 <.text+0x29e6>
18:26F0  pushw	$0x3
18:26F2  pushl	-0x4e(%bp)
18:26F6  lesw	-0x4e(%bp), %bx
18:26F9  jmp	0x29db <.text+0x29db>
18:26FC  movw	$0x0, -0x48(%bp)
18:2701  movw	$0xffff, -0x6(%bp)      # imm = 0xFFFF
18:2706  pushw	$0x1
18:2708  movw	0x8(%bp), %dx
18:270B  movw	0x6(%bp), %ax
18:270E  addw	$0x92, %ax
18:2711  movw	$0x8, %cx
18:2714  lcallw	$0x0, $0xffff           # imm = 0xFFFF ; NE 2715=1:3E86
18:2719  pushl	-0x6(%bp)
18:271D  lesw	0x6(%bp), %bx
18:2720  pushl	%es:0x8e(%bx)
18:2726  pushw	%cs
18:2727  callw	0x528 <.text+0x528>
18:272A  addw	$0x12, %sp
18:272D  pushl	$0x0
18:2730  pushl	$0x14391                # imm = 0x14391
18:2736  lesw	0x6(%bp), %bx
18:2739  pushl	%es:0x86(%bx)
18:273F  lcallw	$0x0, $0xffff           # imm = 0xFFFF ; NE 2740=36:28F7
18:2744  addw	$0xc, %sp
18:2747  lesw	0x6(%bp), %bx
18:274A  lesw	%es:0x8e(%bx), %bx
18:274F  movl	%es:0xa(%bx), %eax
18:2754  movl	%eax, -0x6(%bp)
18:2758  lesw	%es:0x1e(%bx), %bx
18:275C  movl	%es:0xa(%bx), %eax
18:2761  movl	%eax, -0xa(%bp)
18:2765  movw	-0x6(%bp), %ax
18:2768  shlw	$0x5, %ax
18:276B  lesw	0x6(%bp), %bx
18:276E  addw	%ax, %bx
18:2770  movw	-0x4(%bp), %ax
18:2773  shlw	$0x2, %ax
18:2776  addw	%ax, %bx
18:2778  lesw	%es:0x104(%bx), %bx
18:277D  cmpw	$0x0, %es:0x8(%bx)
18:2782  jne	0x27a3 <.text+0x27a3>
18:2784  movw	-0xa(%bp), %ax
18:2787  shlw	$0x5, %ax
18:278A  lesw	0x6(%bp), %bx
18:278D  addw	%ax, %bx
18:278F  movw	-0x8(%bp), %ax
18:2792  shlw	$0x2, %ax
18:2795  addw	%ax, %bx
18:2797  lesw	%es:0x104(%bx), %bx
18:279C  cmpw	$0x0, %es:0x8(%bx)
18:27A1  je	0x27a8 <.text+0x27a8>
18:27A3  movw	$0x1, -0x48(%bp)
18:27A8  lesw	0x6(%bp), %bx
18:27AB  lesw	%es:0x8e(%bx), %bx
18:27B0  pushw	%es:0x8(%bx)
18:27B4  movw	-0x6(%bp), %ax
18:27B7  shlw	$0x5, %ax
18:27BA  lesw	0x6(%bp), %bx
18:27BD  addw	%ax, %bx
18:27BF  movw	-0x4(%bp), %ax
18:27C2  shlw	$0x2, %ax
18:27C5  addw	%ax, %bx
18:27C7  pushl	%es:0x104(%bx)
18:27CD  pushw	%cs
18:27CE  callw	0x323 <.text+0x323>
18:27D1  addw	$0x6, %sp
18:27D4  lesw	0x6(%bp), %bx
18:27D7  lesw	%es:0x8e(%bx), %bx
18:27DC  lesw	%es:0x1e(%bx), %bx
18:27E0  pushw	%es:0x8(%bx)
18:27E4  movw	-0xa(%bp), %ax
18:27E7  shlw	$0x5, %ax
18:27EA  lesw	0x6(%bp), %bx
18:27ED  addw	%ax, %bx
18:27EF  movw	-0x8(%bp), %ax
18:27F2  shlw	$0x2, %ax
18:27F5  addw	%ax, %bx
18:27F7  pushl	%es:0x104(%bx)
18:27FD  pushw	%cs
18:27FE  callw	0x323 <.text+0x323>
18:2801  addw	$0x6, %sp
18:2804  lesw	0x6(%bp), %bx
18:2807  pushl	%es:0x8e(%bx)
18:280D  pushw	%cs
18:280E  callw	0x2ba <.text+0x2ba>
18:2811  addw	$0x4, %sp
18:2814  lesw	0x6(%bp), %bx
18:2817  lesw	%es:0x8e(%bx), %bx
18:281C  pushl	%es:0x1e(%bx)
18:2821  pushw	%cs
18:2822  callw	0x2ba <.text+0x2ba>
18:2825  addw	$0x4, %sp
18:2828  lesw	0x6(%bp), %bx
18:282B  lesw	%es:0x8e(%bx), %bx
18:2830  movw	%es, -0x40(%bp)
18:2833  movw	%bx, -0x42(%bp)
18:2836  pushl	%es:0x4(%bx)
18:283B  lesw	%es:0x4(%bx), %bx
18:283F  lesw	%es:0x2(%bx), %bx
18:2843  lcallw	*%es:0x4(%bx)
18:2847  addw	$0x4, %sp
18:284A  movw	%dx, -0x4a(%bp)
18:284D  movw	%ax, -0x4c(%bp)
18:2850  pushl	-0x4c(%bp)
18:2854  movw	-0x6(%bp), %ax
18:2857  shlw	$0x5, %ax
18:285A  lesw	0x6(%bp), %bx
18:285D  addw	%ax, %bx
18:285F  movw	-0x4(%bp), %ax
18:2862  shlw	$0x2, %ax
18:2865  addw	%ax, %bx
18:2867  lesw	%es:0x104(%bx), %bx
18:286C  pushl	%es:0x4(%bx)
18:2871  movw	-0x6(%bp), %ax
18:2874  shlw	$0x5, %ax
18:2877  lesw	0x6(%bp), %bx
18:287A  addw	%ax, %bx
18:287C  movw	-0x4(%bp), %ax
18:287F  shlw	$0x2, %ax
18:2882  addw	%ax, %bx
18:2884  lesw	%es:0x104(%bx), %bx
18:2889  lesw	%es:0x4(%bx), %bx
18:288D  lesw	%es:0x2(%bx), %bx
18:2891  lcallw	*%es:(%bx)
18:2894  addw	$0x8, %sp
18:2897  lesw	0x6(%bp), %bx
18:289A  lesw	%es:0x8e(%bx), %bx
18:289F  movw	%es, -0x44(%bp)
18:28A2  movw	%bx, -0x46(%bp)
18:28A5  lesw	%es:0x1e(%bx), %bx
18:28A9  pushl	%es:0x4(%bx)
18:28AE  lesw	-0x46(%bp), %bx
18:28B1  lesw	%es:0x1e(%bx), %bx
18:28B5  lesw	%es:0x4(%bx), %bx
18:28B9  lesw	%es:0x2(%bx), %bx
18:28BD  lcallw	*%es:0x4(%bx)
18:28C1  addw	$0x4, %sp
18:28C4  movw	%dx, -0x4e(%bp)
18:28C7  movw	%ax, -0x50(%bp)
18:28CA  pushl	-0x50(%bp)
18:28CE  movw	-0xa(%bp), %ax
18:28D1  shlw	$0x5, %ax
18:28D4  lesw	0x6(%bp), %bx
18:28D7  addw	%ax, %bx
18:28D9  movw	-0x8(%bp), %ax
18:28DC  shlw	$0x2, %ax
18:28DF  addw	%ax, %bx
18:28E1  lesw	%es:0x104(%bx), %bx
18:28E6  pushl	%es:0x4(%bx)
18:28EB  movw	-0xa(%bp), %ax
18:28EE  shlw	$0x5, %ax
18:28F1  lesw	0x6(%bp), %bx
18:28F4  addw	%ax, %bx
18:28F6  movw	-0x8(%bp), %ax
18:28F9  shlw	$0x2, %ax
18:28FC  addw	%ax, %bx
18:28FE  lesw	%es:0x104(%bx), %bx
18:2903  lesw	%es:0x4(%bx), %bx
18:2907  lesw	%es:0x2(%bx), %bx
18:290B  lcallw	*%es:(%bx)
18:290E  addw	$0x8, %sp
18:2911  movw	-0x6(%bp), %ax
18:2914  shlw	$0x5, %ax
18:2917  lesw	0x6(%bp), %bx
18:291A  addw	%ax, %bx
18:291C  movw	-0x4(%bp), %ax
18:291F  shlw	$0x2, %ax
18:2922  addw	%ax, %bx
18:2924  pushl	%es:0x104(%bx)
18:292A  pushw	%cs
18:292B  callw	0x24a <.text+0x24a>
18:292E  addw	$0x4, %sp
18:2931  movw	-0xa(%bp), %ax
18:2934  shlw	$0x5, %ax
18:2937  lesw	0x6(%bp), %bx
18:293A  addw	%ax, %bx
18:293C  movw	-0x8(%bp), %ax
18:293F  shlw	$0x2, %ax
18:2942  addw	%ax, %bx
18:2944  pushl	%es:0x104(%bx)
18:294A  pushw	%cs
18:294B  callw	0x24a <.text+0x24a>
18:294E  addw	$0x4, %sp
18:2951  cmpw	$0x0, -0x48(%bp)
18:2955  jne	0x2980 <.text+0x2980>
18:2957  pushl	0x6(%bp)
18:295B  nop
18:295C  pushw	%cs
18:295D  callw	0x3538 <.text+0x3538>
18:2960  addw	$0x4, %sp
18:2963  lesw	0x6(%bp), %bx
18:2966  lesw	%es:0x8e(%bx), %bx
18:296B  cmpw	$0x0, %es:0x28(%bx)
18:2970  je	0x29e6 <.text+0x29e6>
18:2972  pushl	0x6(%bp)
18:2976  nop
18:2977  pushw	%cs
18:2978  callw	0x318f <.text+0x318f>
18:297B  addw	$0x4, %sp
18:297E  jmp	0x29e6 <.text+0x29e6>
18:2980  pushl	$0x0
18:2983  movw	$0x0, %ax ; NE 2984=49:1204
18:2986  movw	%ax, %es
18:2988  pushl	%es:0x1204
18:298E  pushw	$0x2c
18:2990  lcallw	$0x0, $0xffff           # imm = 0xFFFF ; NE 2991=30:0113
18:2995  addw	$0xa, %sp
18:2998  movw	%dx, -0x56(%bp)
18:299B  movw	%ax, -0x58(%bp)
18:299E  orw	%dx, %ax
18:29A0  je	0x29b5 <.text+0x29b5>
18:29A2  pushw	$0x0
18:29A4  pushl	0x6(%bp)
18:29A8  pushw	%dx
18:29A9  pushw	-0x58(%bp)
18:29AC  pushw	%cs
18:29AD  callw	0x0 <.text>
18:29B0  addw	$0xa, %sp
18:29B3  jmp	0x29bb <.text+0x29bb>
18:29B5  movw	-0x56(%bp), %dx
18:29B8  movw	-0x58(%bp), %ax
18:29BB  movw	%dx, -0x52(%bp)
18:29BE  movw	%ax, -0x54(%bp)
18:29C1  pushw	%dx
18:29C2  pushw	%ax
18:29C3  lcallw	$0x0, $0xffff           # imm = 0xFFFF ; NE 29C4=32:009A
18:29C8  addw	$0x4, %sp
18:29CB  cmpl	$0x0, -0x54(%bp)
18:29D0  je	0x29e6 <.text+0x29e6>
18:29D2  pushw	$0x3
18:29D4  pushl	-0x54(%bp)
18:29D8  lesw	-0x54(%bp), %bx
18:29DB  lesw	%es:(%bx), %bx
18:29DE  lcallw	*%es:(%bx)
18:29E1  addw	$0x6, %sp
18:29E4  jmp	0x29e6 <.text+0x29e6>
18:29E6  lesw	0x6(%bp), %bx
18:29E9  lesw	%es:0x1a4(%bx), %bx
18:29EE  cmpl	$-0x1, %es:0x42(%bx)
18:29F4  je	0x2a25 <.text+0x2a25>
18:29F6  pushl	%es:0x42(%bx)
18:29FB  lesw	0x6(%bp), %bx
18:29FE  movw	%es:0x8e(%bx), %ax
18:2A03  addw	$0xe, %ax
18:2A06  pushw	%es:0x90(%bx)
18:2A0B  pushw	%ax
18:2A0C  lcallw	$0x0, $0xffff           # imm = 0xFFFF ; NE 2A0D=11:2295
18:2A11  addw	$0x8, %sp
18:2A14  lesw	0x6(%bp), %bx
18:2A17  lesw	%es:0x1a4(%bx), %bx
18:2A1C  movl	$0xffffffff, %es:0x42(%bx) # imm = 0xFFFFFFFF
18:2A25  popw	%di
18:2A26  popw	%si
18:2A27  leave
18:2A28  decw	%bp
18:2A29  lretw
```

## Alcance da conclusão

Estão registrados os caminhos de sorteio e distribuição VM, prioridade e continuação do empate SG, geometria/ordem dos lados, decisões e passes de colisão DD e subcasos de giro/gravidade GW. A preservação das instruções espaciais acima é intencional: as diferenças entre ocupação e limite, os sorteios sequenciais e a ordem dos callbacks não devem ser substituídos por regras genéricas. Sem executar o original, a conclusão é de cobertura estática desses caminhos; não de validação visual/temporal quadro a quadro. VM não demonstra limite de erros nem derrota automática; títulos de filmes externos não foram inferidos de falas.

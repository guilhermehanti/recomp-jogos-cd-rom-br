# Mapa comportamental do Big Job brasileiro

Os endereços usam segmento NE em decimal e offset em hexadecimal. A fonte é o BIGJOBNG.EXE brasileiro, SHA-256 `e483ac5986faea03f683168b6b05eae1e7ca9a5eed8449b0e880849362d65a1a`. Os offsets MMS são relativos a BIGJOB.MMS, SHA-256 `f969e2509c80893a0e46b2a625491f8b311911e37ee29bafa2ffcd137ffff427`.

Esta revisão substitui as conclusões anteriores desta página. O carregador, os comandos de mídia, as saídas de cena e as regras foram conferidos no executável e nos dados da ISO. Os documentos vinculados preservam os endereços de construção, registro, execução e conclusão de cada cadeia.

## Correções da análise anterior

O carregador 37:0456 chama 39:13AB em37:04C3 e inverte os bytes de cada DWORD. Para seis palavras big-endian W0..W5 no disco, a ordem das palavras em memória é W1,W0,W3,W2,W5,W4. O opcode está em W1 e o primeiro argumento de16bits em W3. O registro0x23AFF realmente executa o comando4 da cabine. A conclusão anterior que o descartava estava errada.

A global DS02A0 seleciona tema. O cartaz0 escolhe incêndio,1 fazenda e2 construção. A escolha de dificuldade ocorre na cena14 e fornece um argumento separado. As tabelas de estrada e filmes dependentes de02A0 seguem o tema.

O campo+64 da classe de atividade5:14FF é um identificador de progresso. A função6:026C registra esse progresso e apresenta uma recompensa. O campo+7C executado por5:1A2F contém um comando sobre a animação introdutória. As saídas manuais usam os controles construídos por3:35A4. Esses três mecanismos têm efeitos próprios e estão rastreados em18-NAVEGACAO.md.

Os identificadores de som e filme usam banco=`id>>14` e índice=`id&3FFF`, na palavra baixa. Os bancos efetivos vêm dos recursos NE702/703/704 e dos cabeçalhos dos arquivos.

## Do evento Windows ao manipulador original

38:3698 cria o procedimento38:33BC para MMAppCanvasClass. As mensagens0200,0201,0202,0204,0205 passam por38:3433, que compara o estado dos botões, transforma as coordenadas e chama o slot+A4 da aplicação. Na aplicação2:190A esse slot é2:15B1, que encaminha a31:15E5.

31:15E5 usa o hit-test+14 da raiz para selecionar o objeto. Mantém captura em+30 e entrega os eventos pelo slot+4. O objeto determina se a ação acontece no pressionamento ou na soltura. O botão33:1773/17CC mantém captura, atualiza a imagem durante o arraste e só executa na soltura dentro da área. A área33:1903/1956 executa no pressionamento dentro da região. A ação resolve seu comando por33:1750→32:009A. O método32:009A respeita as guardas e escolhe execução imediata pelo slot+8 ou cópia/enfileiramento.

O contrato de botões, incluindo cancelar a seleção ao soltar fora, está em21-BOTOES-JUKEBOX.md. Os receptores de teclado são métodos distintos, listados por cena em15-CENAS.md. Códigos ASCII4/6/8/2 e eventos65..68 são preservados como códigos recebidos pelo original.

## Atividades e cadeias de controle

| Cena e módulo | Entrada e manipulador | Regras e conclusão original | Mídia ligada ao comportamento | Saída manual até a oficina |
|---|---|---|---|---|
|1 SB, tabuleiro de criação|23:030F monta controles;23:1110 recebe teclado; controladores do segmento24 recebem as seleções|25 grades8010, conversor23:31E1, edição de células,10agentes autônomos e interações por tipo/célula; limpar/transição é24:1D2B/1EAB|Interações23:1F70 selecionam animações MMS e callbacks de desbloqueio; não atribuir vitória a uma operação de limpeza|Botão comum grupo0, parâmetro5→0|
|2 Cabine e direção|12:0C00 aceita1/2/3/R/r;12:0D5B recebe65..68; comandos12:3328→3385|15:0589→14:1D89 testa movimento/colisão; aliases da grade escolhem descritores; medidor13:10B6 ao120% dispara a sequência de saída|Comando5 escolhe filme+125;21 filmes divididos em3 temas; áudio e callbacks da cabine descritos em12|Botão comum→0; combustível→comando10→400/401→espera áudio→402→callback4→comando11→0|
|5 VM, memória|12 objetos27:2070, receptor27:1EBC, comandos27:1C10→0DC3|Dois itens revelados são comparados pelo tipo; pares iguais são retirados;12 retiradas concluem; erro restaura a tentativa|MMS01A4 contém vozes de acerto/erro/conclusão; animação0113 controla confirmação e saída|Botão comum→0; comando registrado no slot1 do controlador0113→0|
|7 CG, vacas e portões|Comandos2:2D91→2:2DE0→2:34EE alternam controles e chegam a6:32D5|8 grades8012; apenas a cor selecionada abre; outra cor fecha; decisão aplicada a cada10 subpassos; geração, flores, saída e término em6/7|Áudios do caminho/flores; resultado chega ao mecanismo comum; prêmio de progresso31 seleciona filme32|Grupo2→2; botão da cabine→0|
|13 DD, entidades adversárias|25:3799 e3755 gravam entradas;26:0343 consome uma por atualização|4entidades, equipe e HP;26:04D4 aplica dano;26:1B87 perde sem equipe1 e vence sem equipe0; empate por eliminação simultânea segue primeiro ramo de derrota|Áudio436A/436D e animações de dano; progresso12→filme55|Grupo1→4; botão do painel→0|
|15 SG, arestas e caixas|Objetos20:160A e comandos20:14BF→21:1C13→1C91|Fechar4arestas concede ponto e mantém turno; uma aresta pode fechar2caixas;21:2341 compara placares ao esgotar arestas|MMS0175/0176 e áudio por turno; progresso32→filme23|Grupo2→2→0|
|16 GW, blocos de grãos|18:14FE e controles de giro/deslocamento/plunger→38C0/398E/3A5C/3B2A|Grade5×8, par ativo, rotações guardadas, pedidos verticais de2/3 peças; terceiro caminhão concluído vence; entrada bloqueada perde|Tutoriais0184/0185; sons4392/4393/4394/400F; progresso19→filme5|Grupo2→2→0|
|17 HW, caminho de mangueira|Células19:2CCE→ação3628/367C→comando2582→16CC|Grade10×6; substituição só aceita célula ainda livre e estado permitido; tipos5..10 têm conexões exatas;1958 avança o fluxo e só vence ao alcançar destino tipo0|MMS0138 e animações/sons do controlador; progresso18→filme21|Grupo2→2→0|
|18 WB, demolição|27:3907 e botões→27:3BF0→28:1457|Movimento e balanço com estados/intertravamentos; atualizador125B conclui ao contadorE4=60;27:3691 compara placares|Sons4031/400C e estados visuais; progresso15→filme57|Grupo1→4→0|
|19 WC, canhão de água|29:088E/botões→29:1DC2; disparo exige dois eixos parados|Grade5×4;5fogos iniciais, propagação para vizinhos;15fogos ativos perde; zero vence quando o decremento tem a flag de resultado; jato reduz estágios e reserva|43A0 acompanha jato;401B recusa;4018 execução; progresso4→filme49|Grupo1→4→0|
|20 FL, pilhas e captura|16:0EDE aceita4/6; comandos04F9/05C4 e controladores de objetos|Quatro pilhas,26 objetos;16:1642 apura scene+86>=3; a produção e proteção contra dupla contagem estão em20|MMS01B6/0117,4389 e efeitos dos objetos; progresso6→filme25|Grupo2→2→0|
|21 FM, resgate|17:00C1/0653 e comandos0D50→0DA6→11EC→122E|22 grades8016; limites, empurrar, machado e pessoa; pessoa na porta→animação22C4→callback→resultado positivo|TuOoomph,TuRescuePerson,TuPersonOutSafely,FMpush01; progresso23→filme41|Grupo2→2→0|

O estado de saída normal é parte da regra de navegação. DD/WB/WC voltam ao painel. CG/SG/GW/HW/FL/FM voltam à cabine. O botão do pai completa a volta à oficina. A reprodução de recompensa e a volta manual têm registros separados.

## Cenas auxiliares, menus e objetos da oficina

| Seletor | Conteúdo e fluxo |
|---:|---|
|0|Oficina8:010D/024D,8 áreas de navegação e4 áreas que solicitam rótulos aleatórios2/3 de animações0141/014F. Ver17.|
|3|Cartazes CS. Clique→11:0BC3→0921→tarefa002E; espera som4387;11:09FD grava tema por3:2D4A e retorna0.|
|4|Painel CP. Seus estados selecionam filme20/18/8 por tema e abrem o menu de atividades; botão comum→0.|
|6|Impressão CT. Componentes e comandos do controlador10:23C7; área própria11:1203→0. Ver23.|
|8|Imagem parametrizada. PIC e destino vêm dos argumentos; área de tela inteira executa a transição.|
|9|Entrada LG. Animação0139 e ação de pular encaminham à tela11.|
|10|TimeClockScene. Seleção/criação de cartão e nome; confirmação0120 e comando11 levam à cena3. Ver22.|
|11|Apresentação JY. Encaminha ao relógio/cartões10.|
|12|Jukebox VJ. Quatro botões na soltura→filmes2/9/19/22; saída comum→0. Ver21.|
|14|Seleção de dificuldade em cache. E/e e H/h executam2:1B08 com flag1/0 e a cena selecionada em211E.|
|22|Menu CP. Os nove controles selecionam13/15/17/18/16/19/20/7/21 por meio da dificuldade14.|
|23|Créditos CR. Clique na área de tela inteira→0.|
|24|Cena-base com método+0C vazio,1:3EE2.|

O segmento22 contém também uma sobreposição de opções. O número do segmento não é o seletor22. A sobreposição de opções não é a saída comum das atividades.

## Scripts, grades e recursos

As11 regiões antes opacas são tabelas com registros de12bytes e strings auxiliares. Os28blocos8009 são estradas, o800E tem6 linhas, os25blocos8010 têm20×10 símbolos, os8blocos8012 têm13×13 símbolos e os22blocos8016 têm10×14 símbolos. Os dois8008 contêm cinepak.mov/indeo.mov; o carregamento normal do banco de filmes vem do recurso704 BIGJOB.MOV.

A leitura integral é reproduzível em mapa-jogo-v3.json. mms-control-flow.json decodifica122 scripts de raiz8003, com os argumentos já na ordem usada pelo interpretador e441 gatilhos explícitos de som. Referências de imagem e som resolvidas pelo carregador preservam a identidade do banco. As linhas nativas anotadas e13.487 locais de relocação estão em disassembly.

Os opcodes000C..0014, os saltos, a espera e os comandos numerados estão em16-INTERPRETADOR.md. A captura do evento e a construção do comando são necessárias para dar significado ao callback de um script. O mesmo número de slot pode pertencer a comandos diferentes em instâncias distintas.

## Filmes e sons

14-MIDIA.md e media-catalogue.json identificam os58 filmes. A partição é21 na cabine,3 no painel,4 na jukebox e30 na tabela do objeto de recompensa do segmento3. Todos os ordinais0..57 aparecem uma vez nessa partição. Os30 ordinais da última tabela pertencem a36contextos, com6 sentinelasFFFF. Os9IDs de progresso das atividades estão ligados aos respectivos filmes na tabela acima.

A reprodução passa pelo comando4:1929, executor4:14A1 e player33:2FBA. O ordinal seleciona um offset moov no cabeçalho BIGJOB.MOV. Os8bytes guardados em4:1710 são geometria da janela. Eles não são o identificador do filme.

O catálogo contém962entradas de som nos dois bancos,441 gatilhos MMS e85 locais de chamadas nativas de áudio com o contexto das instruções. As falas brasileiras são referenciadas pelos IDs originais. A interpretação semântica de uma fala não substitui a condição nativa que a dispara.

## Evidências e reprodução da verificação

| Arquivo | Conteúdo |
|---|---|
|[12-CABINE.md](12-CABINE.md)|Movimento, estradas, descritores, comandos, mídias e saída da cabine|
|[13-GRADES.md](13-GRADES.md)|SB/CG/FM, símbolos, agentes, regras e resultados|
|[14-MIDIA.md](14-MIDIA.md)|Bancos efetivos,58 filmes, seleção e reprodução|
|[15-CENAS.md](15-CENAS.md)|Construtores, tabelas virtuais e entradas dos25 seletores|
|[16-INTERPRETADOR.md](16-INTERPRETADOR.md)|Ordem dos dados,9 opcodes, saltos e callbacks|
|[17-OFICINA.md](17-OFICINA.md)|Áreas, retângulos, comandos e animações da oficina|
|[18-NAVEGACAO.md](18-NAVEGACAO.md)|Destinos e saídas, dificuldade e cartazes|
|[19-ATIVIDADES-A.md](19-ATIVIDADES-A.md)|VM/DD/SG/GW|
|[20-ATIVIDADES-B.md](20-ATIVIDADES-B.md)|HW/WB/WC/FL|
|[21-BOTOES-JUKEBOX.md](21-BOTOES-JUKEBOX.md)|Captura/soltura e seleção da jukebox|
|[22-TC-PERFIL.md](22-TC-PERFIL.md)|Abertura, cartões, nome e confirmação|
|[23-IMPRESSAO.md](23-IMPRESSAO.md)|Controles e caminho de impressão CT|
|[24-HW-CELULAS.md](24-HW-CELULAS.md)|Clique em célula e conectividade da mangueira|
|[25-PERSISTENCIA.md](25-PERSISTENCIA.md)|Formato SAV, cartões, INI, gravação e falhas|
|[26-AUDIO-NATIVO.md](26-AUDIO-NATIVO.md)|85 sites de áudio:61 literais,19 seleções,5 interfaces parametrizadas|
|[27-RECOMPENSAS.md](27-RECOMPENSAS.md)|30 contextos:21 filmes da cabine e9 atividades, sem lacunas na tabela|
|[28-FRONTEIRAS-APIS.md](28-FRONTEIRAS-APIS.md)|110 imports identificados, ABI e98 fixups de compatibilidade x87|
|[29-AUDIO-RUNTIME.md](29-AUDIO-RUNTIME.md)|PCM, três canais, filas, backend silencioso e962 cabeçalhos|
|[30-VIDEO-FRONTEIRA.md](30-VIDEO-FRONTEIRA.md)|17 stubs referenciados, argumentos, parada e término|
|[32-OFICINA-BASQUETE.md](32-OFICINA-BASQUETE.md)|Clique na bola, física, colisões, cesta, série, animação e18 falas candidatas|
|[34-AUDIO-EVENTOS.md](34-AUDIO-EVENTOS.md)|85 chamadas nativas, condições nos controladores e evidência de origem|
|[35-MMS-EVENTOS.md](35-MMS-EVENTOS.md)|98 raízes sonoras, origem de carregamento e 441 instruções de áudio|
|[36-QUICKTIME.md](36-QUICKTIME.md)|Despacho dos 17 seletores referenciados e contratos por componente|
|[37-WIN16-ESTRUTURAS.md](37-WIN16-ESTRUTURAS.md)|Estruturas Win16 e convenção dos callbacks de janela|

`python tools/annotate_ne.py` expande e anota as cadeias de relocação sem executar o jogo. `python tools/trace_mms.py` gera a decodificação dos scripts. `python tools/verify_behavior_map.py` relê os extents da ISO, compara56.006 registros, confere os58 offsets moov e verifica a partição dos seletores de filmes. O resultado é behavior-verification.json.

A validação executada é estática. A comparação de uma futura implementação com o jogo em execução continua sendo uma etapa própria. Os detalhes de condições e os limites específicos de cada módulo devem ser consultados nos documentos vinculados antes de implementar.

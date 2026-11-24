# Navegação: controles compartilhados e retornos nativos

Análise exclusivamente estática de BIGJOBNG.EXE brasileiro. Segmentos em decimal, offsets em hexadecimal. Não houve execução do original nem compilação.

## Controle real de saída das atividades

5:1657 constrói a base das atividades. Em5:1771 passa o ID de cena +66 ao slot virtual+3C, herdado como5:0EF1. Este monta ajuda/animação e o controle de saída.5:130B consulta3:37E0, que lê DS:0338[scene]. Quando habilitado,5:1318 passa cena e comando opcional a3:35A4; o controle retornado é anexado à cena por5:132E->5:0967->35:3B22.

3:35A4 busca a posição lógica por DS:02A2[scene] e5:2F58 (leitura do descritor MMS pela global221C). Se o comando opcional é nulo,3:35C9 resolve o parâmetro por3:36BA (DS:0306[scene]) e o grupo por3:36A6 (DS:02D4[scene]), então chama3:327F.3:3631-3667 resolve as duas imagens por34:0036 e cria um botão33:10F8 com o comando e a posição. Portanto o retorno está em um botão próprio do controle comum, e não depende de terminar o minijogo.

A fábrica3:327F tem cinco grupos.0 cria2:1AC3 com destino0 em3:3300/330B;1 destino4 em3:3397/33A2;2 destino2 em3:342E/3439;3 destino3 em3:34C5/34D0;4 constrói a classe3:3838, não uma troca para cena22. Os grupos0..3 guardam FFFF no segundo argumento e o parâmetro específico no terceiro.2:1AC3 é a classe de comando; seu executor chega à transição2:0697. O clique usa o receptor do botão, enquanto áreas32:3735 usam33:1956; ambos desembocam na execução de comando32:009A.

## Tabelas completas dos25 seletores

Extração direta do segmento49 do EXE (offset físico652256), words little endian. Habilitado significa o ramo comum5:130B; não garante que toda cena invoque esse construtor, nem elimina controles próprios.

|Cena|Grupo DS02D4|Parâmetro DS0306|Botão comum DS0338|Posição lógica DS02A2|Destino normal do grupo|
|---:|---:|---:|---:|---:|---|
|0|4|0|1|402|Comando3:3838|
|1|0|5|1|403|0|
|2|0|9|1|404|0|
|3|0|3|0|405|0, botão comum desabilitado|
|4|0|9|1|406|0|
|5|0|0|1|407|0|
|6|0|4|1|408|0|
|7|2|0|1|409|2|
|8|FFFF|0|0|FFFF|Sem grupo|
|9|FFFF|0|0|FFFF|Sem grupo|
|10|3|0|0|410|3, botão comum desabilitado|
|11|FFFF|0|0|FFFF|Sem grupo|
|12|0|9|1|411|0|
|13|1|9|1|412|4|
|14|FFFF|0|0|FFFF|Sem grupo|
|15|2|0|1|413|2|
|16|2|0|1|414|2|
|17|2|0|1|415|2|
|18|1|9|1|416|4|
|19|1|9|1|417|4|
|20|2|0|1|418|2|
|21|2|0|1|419|2|
|22|0|0|0|FFFF|0, botão comum desabilitado|
|23|FFFF|0|0|FFFF|Sem grupo|
|24|0|0|0|0|0, botão comum desabilitado|

Assim as atividades13/DD,18/WB,19/WC voltam ao painel4; o botão de4 volta à oficina0. As atividades7,15/SG,16/GW,17/HW,20/FL,21 voltam à cabine2; o botão comum da cabine usa grupo0/parâmetro9 e volta à oficina0. O retorno adicional da cabine por combustível está demonstrado no relatório12. Não se deve implementar todas as atividades voltando diretamente a0.

## Conclusão e falha não são o botão de saída

Correção ao relatório16:5:17A5->MMS020112->2:2671->6:026C concede progresso/prêmio. O ID +64 é progresso. O comando+7C executado por5:1A2F foi criado em5:1724 por30:101E sobre a animação introdutória01018C; não é um comando de troca de cena.30:101E cria uma ação sobre a animação, usando30:2509. Não há fundamento para tratar o sucesso como retorno automático à oficina.

A falha020116->2:25BC->5:1B5E registra tentativa por6:03E2 e anexa a sobreposição+76. Essa sobreposição foi construída por5:198F->3:24DE: primeiro comando2:1BDF recebe a própria cena+66 (repetir/selecionar dificuldade); segundo comando vem de3:36A6->3:327F com o grupo da tabela acima (sair ao pai).3:25BB liga o primeiro ao slot0 da animação010113;3:25D1 liga o segundo ao slot1.3:28BD e29AA criam comandos de animação para rótulos2/4, associados a controles espaciais4:0F7E e32:3735. O interpretador executa os slots por30:1E70->30:11C4->32:009A. Esse é um segundo caminho de retorno ao mesmo pai, não uma prova de retorno direto0 em todos os casos.

## Sobreposição22:0000 é opções

Não confundir segmento22 com seletor de cena22.5:0305 recebe código2A e, se cena+20=0,+54 nulo e gerenciador0432 existente,5:0642 cria22:0000 com owner e duração10000, guarda+54 e anexa.22:04B3 despacha seletores0..3 para configurações, outra ação de opções, stub de impressão22:0607 e instalação de protetor de tela pelo Windows. O segmento não constrói2:1AC3. Portanto esse caminho não é a saída geral.

O ramo c/C em5:03E9 cria transição0, porém está sob o teste da global008A e exige aplicação+566!=0. É inadequado apresentá-lo como controle normal de retorno sem preservar as guardas.5:3294, da tela parametrizada8, possui caminhos próprios K/#->22 e c/C->0, distintos desse ramo global.

## Entradas já amarradas

Oficina0: o comando2:1BDF criado em8:062E escolhe cena5 e abre a dificuldade14. A dificuldade é a instância cache aplicação+512 criada em2:04C8->5:1E47. E/e/H/h no handler5:2433 chegam2:1B08->2:1B5C->2:0697, usando o seletor global211E. Os contextos9FFFF e flag1/0 devem ser preservados.

Menu22: construído por15:11E0. Os seletores13,15,17,18,16,19,20,7,21 passam pelo cache14. O controle de oficina criado em15:1458 é separado do comando cache14 criado em15:14AB, conforme relatório15.

## Limites explícitos

A tabela cobre todos os25 seletores, mas não é uma prova de botão comum para os nove desabilitados. Os retornos próprios de5,6,8,23 estão nos relatórios12/15; LG9/JY11 formam ligações específicas20:0C47->11,20:0CF0->11,20:0F60->9,20:1009->11,20:12A8->10,20:1376->10. O retorno próprio deCS3 está fechado na seção adicional abaixo; TC10 é tratado no relatório22. A linha da tabela não substitui essas cadeias específicas. Essas limitações são distintas da saída das sete atividades com base5:14FF, cujo botão comum foi fechado acima.

## Fechamento adicional: seleção de cenário CS3 e retorno0

A cena3 não precisa do botão comum desabilitado. O clique em qualquer cartaz confirma o cenário e retorna à oficina por uma tarefa cooperativa própria:

1.11:0278 constrói três botões especiais2:370B. O cartaz de construção usa imagens47F9/47FA e comando com seletor2 (11:0321); fazenda usa47FB/47FC com seletor1; incêndio usa47FD/47FE com seletor0. Os nomes do banco PIC são, respectivamente,CSconstructionPosterBD/BU,CSfarmPosterBD/BU,CSfirePosterBD/BU (índices2041..2046, referência lógica4000+índice).
2.Os comandos pertencem à vtable11:0AC8. Seu slot+8 aponta11:0BC3, que extrai owner+26 e seletor+2A e chama11:0921 em11:0BD7.11:0921 copia o botão escolhido (cena+70/+6C/+68 para seletores2/1/0) ao ponteiro+42 do filho e grava2/1/0 em filho+78. Esse é o gatilho concreto da confirmação; não é apenas decoração do cartaz.
3.O filho foi criado pelo construtor11:0113:11:01A1 inicializa a base37:154E,11:01AF instala vtable11:0BE2,11:01CD zera+42,11:01D6 configura+24=0800 e11:01FB inicia a tarefa por37:1678. A vtable tem slot+14 em11:0BF6->11:002E;37:1E91/1EA1 é o invocador desse slot. Trata-se de processamento cooperativo registrado no agendador, não de exigir um segundo clique.
4.11:002E inicialmente chama37:1971 com espera0 e repete enquanto+42 permanece nulo (11:0039-0057). Quando o clique preenche+42,11:0059 usa+78 para escolher a imagem do cartaz via11:0000:0->47FD,1->47FB,2->47F9.11:0085 aplica a imagem ao objeto visual do botão e11:00AF inicia o som lógico4387 no objeto de áudio embutido+46.
5.11:00B6-00DC cede por37:1971 e repete enquanto o estado+5E do objeto permanece não zero. Esse campo é+18 do objeto de áudio construído em filho+46. Após essa espera,11:00DE copia o seletor atual+78 para o comando construído por11:09B0;11:00F7 executa-o por32:009A.
6.A vtable do comando é11:0A84, slot+8=11:09FD.11:0A06 passa comando+26 a3:2D4A.3:2D50 compara com global02A0; se mudou,3:2D65 chama2:0FB5 com argumento2 para invalidar/reconstruir a cabine;3:2D70 grava o novo valor em02A0.11:0A10-0A27 chama2:0697 para cena0 com argumentosFFFF e3.

|Valor global02A0|Tema confirmado|Imagens do cartaz|Cadeia de saída|
|---:|---|---|---|
|0|Incêndio|47FD/47FE|Clique ->11:0921 -> tarefa11:002E ->11:09FD ->0|
|1|Fazenda|47FB/47FC|Clique ->11:0921 -> tarefa11:002E ->11:09FD ->0|
|2|Construção|47F9/47FA|Clique ->11:0921 -> tarefa11:002E ->11:09FD ->0|

Isso também corrige qualquer leitura de02A0 como dificuldade geral: na cadeia CS esse valor seleciona o tema0/1/2. A dificuldade das atividades passa pela tela14 e sua própria seleção.


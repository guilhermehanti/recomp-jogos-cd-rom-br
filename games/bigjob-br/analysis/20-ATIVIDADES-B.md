# Atividades B — regras nativas de HW, WB, WC e FL

Análise exclusivamente estática de BIGJOBNG.EXE BR, segmentos decimais e offsets hexadecimais. Fontes: `analysis/disassembly/seg16.asm`, `seg19.asm`, `seg27.asm`, `seg28.asm`, `seg29.asm`; identidade de cenas e slots conforme 15-CENAS.md. Nenhum original foi executado e nenhuma compilação foi feita. Os offsets de campos são relativos ao objeto indicado, não globais. Os produtores de estado e os predicados terminais são documentados com seus caminhos nativos.

## Resultados terminais demonstrados

| Cena | Disparador nativo | Predicado entregue a +4C |
|---|---|---|
| 17 HW | 19:1958, depois que célula atual +12 supera +18 e não existe continuação aceitável | Verdadeiro somente se destino está em 0..59 e tipo destino +10 é 0; falso nos demais casos. Chamada final 19:1D46. |
| 18 WB | Atualizador 28:125B, ramo de repouso 1292: contador +E4 == 60, muda +AA para 12 e executa comando +EC (12BC) | Callback 27:3691: `scene.+8E->+4 < scene.+8E->+6`, em 36B9–36D5. |
| 19 WC | Contador global estrutura 49:213A: 29:0033 incrementa e encerra ao atingir 15; 29:005F decrementa e encerra quando <=0 e flag recebida é não zero | 0033 passa falso a 29:0FCE; 005F passa verdadeiro. 0FCE limpa controladores/som e chama +4C em 10EA. |
| 20 FL | Comando executor 16:02FD chama 1642; outro consumidor 1F54 também chama 1642 | 16:164B: `scene.+86 >= 3`, independentemente do booleano recebido pelo callback; chama +4C em1677 após parar os controladores. |

+4C é o caminho de resultado compartilhado, não destino de cena. A base usa IDs de progresso HW18, WB15, WC4, FL6. Resultado positivo/negativo continua pela base descrita em 15-CENAS.md/16-INTERPRETADOR.md. Os comandos de MMS devem usar o DWORD convertido pelo carregador39:13AB; a terceira palavra bruta do arquivo não é diretamente seu operando.

## HW — caminho de mangueira em grade

A regra em19:1958 demonstra uma grade lógica de60 posições. `scene.+184` é índice atual, `scene.+80+4*i` aponta para célula e `scene.+192` bloqueia novas avaliações depois de encerrar. A rotina retorna imediatamente quando+192 é não zero. Só tenta avançar quando `current.+12 > current.+18`.

Direção atual é `cell.+16`:19:1DA8 retorna deslocamentos {-1,+1,-10,+10} para valores0,1,2,3 (tabela1DD8). `19:1DE0` valida índice de0 a59 inclusive. A direção de entrada oposta é calculada por19:1DFE como {1,0,3,2}. A continuação exige simultaneamente: destino válido; `19:311E(destination)` verdadeiro, isto é `4 < tipo+10 < 11`; `destination.+12 <= destination.+18`; e `19:3141(destination,oposta)` verdadeiro. Se todos passam,19:31C1 grava entrada em+14 e nova saída em+16, e19:1A89 atualiza índice+184. O processamento continua sem chamar resultado.

Se alguma condição falha,19:1A91 zera global0C3A, define resultado local falso e trava+192=1. Se destino ainda é válido e seu tipo+10==0, chama20:04D1 no controlador+18A e define resultado verdadeiro em1AE3. Tipos restantes ou saída da grade produzem falso. Os ramos1AE8–1D33 montam a animação do fluxo/vazamento usando imagens4B13,4B0A,4B0D,4B10 conforme direção, armazenando no controlador+18A seus campos+50,+56,+58,+5A. Finalmente1D46 chama+4C com o resultado. Assim o objetivo é conectar o percurso ao tipo terminal0; não há prova de uma regra de pressão ou de mirar o fogo nesta rotina.

O teclado normal desta cena usa base5:1B9D. A manipulação de peças por mouse passa pelos filhos, não por esse teclado. Os métodos de pausa19:1FA7 e retomada20A9 atuam em três controladores+186,+18A,+18E. A pausa registra em+1B2 se o primeiro estava ativo (estado+26 em1/2/3), e usa seu virtual+4; a retomada do primeiro exige+1B2, estado1/3, DWORD+32=7FFFFFFF e+36/+38=0. Isso é controle de execução de animação, não predicado de sucesso.

## WB — bola e parede

Entrada por caracteres em27:3907 cria comando27:3BF0: `4→0`, `6→1`, espaço→2, `5→3`; executor27:3C88 chama28:1457. O mesmo controlador recebe os comandos construídos para os botões WBswingBD/BU. Não reinterpretar caracteres como teclas físicas antes do despachante global.

28:1457 ignora tudo se+A8!=0 ou+AA==12. Seletores0/1 só mudam direção se+A6 ainda difere de0/1; copiam posição+CE para+D2/+D6 e fazem alvo X±37; estados+AA=3/4. Som4031, parâmetro200, depende do limite correspondente global2132/2134. Seletor2 durante3/4 define+E0=-1/+1 e estado5. Durante estado2 calcula coluna `(X-global2132)/37`; se ponteiro+B0+4*coluna existe, muda estado para10 e solicita4031; se vazio solicita400C parâmetro1. Seletor3 apenas grava+A6=3 em1627, portanto não é correto chamá-lo disparo.

28:163E reinicia:+C8=1,+E4=0,+A6=3,+A8=0,+AA=2, copia as seis referências da primeira linha para+B0..+C4 e zera global2136. O atualizador28:125B começa em2 e usa espera de50; movimento3/4/5 usa espera20 e auxiliares19F9/1B73/1CED; fases posteriores6..11 usam1E09/1FEC/2059/21F7/2258/232C, com bloqueio+A8=1. Quando regressa ao ramo2, testa+E4==60, muda para12 e executa+EC. Este é um critério numérico explícito de60, não um limiar visual estimado.

O callback+EC é construído na cena em27:35A7–35B9 com vtable3BAC e owner+26. A apuração27:3691 primeiro para o controlador+8A por37:1778 e compara os campos+4 e+6 do objeto+8E. Os produtores dos dois acumuladores e de+E4 são fechados no complemento abaixo.

## WC — extinção e propagação

Entradas `4,6,8,2,espaço` chegam por fábricas29:02EB,03B6,0220,0155,0481 ao controlador29:1DC2. Seus seletores são respectivamente0,1,4,3,2. Direções0/1 gravam eixo+48=3/4 e zeram+44;3/4 gravam eixo+4A=5/6 e zeram+46. Todos rejeitam enquanto+48==7. Disparo2 só aceita ambos eixos em{1,2}, gravando+48=7. Recusa toca401B parâmetro1 em1EAB.

Atualizador29:1B8B começa eixos em1/1. Enquanto horizontal não é7 atualiza mira por28:344E. Estados3/4 consultam28:2E6A/2FB1, aplicam passo com28:3470 e DWORD1; estados5/6 consultam28:31ED/30F8 e aplicam passo com DWORD10000. Flags+44/+46 permitem parar no estado2 após passo. Estado7 chama28:3367; quando retorna verdadeiro, ou quando global2138!=2 e flag+42==0, retorna horizontal a2 e chama28:33F8; limpa+42. Cada iteração espera10 por37:1971; estado8 encerra e chama33F8 novamente.

A grade é montada em29:176B como5 colunas×4 linhas, com vizinhos por ponteiros+8,+C,+10,+14. Inicialização29:0F31 seleciona5 células aleatórias sem fogo (`+26==0`), muda para estado1 ou2 por29:2100 e incrementa contador por29:0033. Estado0 em2100 troca imagem e decrementa contador via005F; só dispara vitória se a flag passada ao decremento está ligada e contador ficou<=0. Crescimento29:2331 escolhe direção aleatória0..3; candidato deve existir, estar em estado0 e ter+24==0. Ativa estado1 e incrementa0033; ao atingir15 fogos0033 encerra com falso. Portanto são15 fogos ativos para falhar e zero para vencer, com flag de resultado explicitamente respeitada.

O gerenciador de fogo29:1F89 percorre a malha, chamando26D7 em cada célula, com espera60. Uma célula que retorna falso provoca+42=3 e interrompe.29:26D7 diferencia estados1/2/3, usa contador+28 e flag+24 para crescimento; no estado3 chama2331 e volta a2, zerando contador. A flag+20 altera limiares/tempos. Esses tempos são de atualização do controlador, não necessariamente segundos.

29:0FCE chama1DA7 no controlador+86; solicita comandos37:183E dos controladores+86/+8A quando ativos em1/2/3, executa por32:009A e destrói temporários. Cancela som emscene+22 por36:2A8F e encaminha resultado ao virtual+4C. Pausa+20 usa37:189F nos dois controladores; retomada+24 usa37:18FF e solicita4018 com parâmetro7FFF em0C59.0D7A também solicita4018. Som4018 não é ID de32bits7FFF4018.

## FL — pilhas, deslocamento e resultado

Conteúdo16:0AE2 carrega FLbackground49B2 e MMS01B6/0117. Teclado16:0EDE: `4→fábrica04F9`, `6→05C4`; espaço é consumido sem criação de comando nesse ramo. Outros caracteres seguem base5:1B9D.

A cena mantém lista livre+98 e quatro pilhas em+9C,+A0,+A4,+A8.16:1BAB cria26 objetos de imagem49DA: um inicial mais25 no laço, ligados por+10/+14.16:1C2E retira elemento da lista livre;1C8F retira último elemento da pilha selecionada (ou+94 quando argumento=-1), reanexa à livre e retorna1.16:1F1A testa se as quatro pilhas estão vazias.16:1B90 estima pilha corrente+94 pela posição horizontal do objeto+B0: para colunas3..1 compara X+40 com `+CC + coluna*+84 + coluna*15`. Não usa apenas proximidade visual.

16:1E8E escolhe+96 (aleatório1..3 se argumento=-1), retira uma peça da lista livre, posiciona por1D90 e associa ao controlador+B8 por36DA quando índice explícito.1D90 calcula posição usando1FFC e21B8; há suporte de empilhamento/altura.16:1642 apura `+86>=3`, chama14A8 para parar controladores e passa booleano a+4C. O chamador1F54 passa0, mas1642 recalcula o resultado. O produtor indireto do contador+86 está em15:2CD1, fechado no complemento abaixo.

Pausa16:0FF6 solicita189F/101E aos controladores. Retomada11D3 solicita18FF em+B8 e+BC; só retoma objeto+88 se+D2 e+C0 se+D4. Limpeza14A8 solicita183E em+B8,+C0,+BC e101E no objeto+88. A rotina1683 solicita áudio0005 parâmetro1 no receptor+B0->+10; controlador333F usa4389 em3362 quando retirada1C8F retorna não zero. Nomes FLBudgie/FLrumble sozinhos não provam filmes nem resultado.

## Limites para uma implementação fiel

Os predicados terminais acima e os produtores WB/FL e a seleção WC são fechados neste documento. A grade, a tabela completa de entradas/saídas e os comandos de mouse HW estão em [24-HW-CELULAS.md](24-HW-CELULAS.md). O mapa de callbacks MMS e recursos deve ser lido com16-INTERPRETADOR.md e os inventários de recursos. Cada módulo usa seu controlador próprio.

## Complemento: efeito de água WC fechado

28:3367 não é um teste geométrico genérico: espera temporizador `controller.+C` por11:224F. Quando vence e reserva+24>0, atualiza efeito visual+1C por28:27DB, solicita som43A0 parâmetro1, chama29:223F no alvo `controller.+4` com flag1 e reduz reserva por28:32E2(-1).32E2 limita o indicador a0..100 e atualiza imagem por29:0DE2. Se reserva já<=0, cancela som e retorna1. Isso explica a retenção do estado7 e o bloqueio de outras entradas durante o jato.

29:223F guarda flag de água emcell.+24. Com flag não zero e estado de fogo não zero, subtrai3 de+28 quando+20!=0, ou10 quando+20==0. Ao chegar<=0 reduz estágio:1→0 com contador0;2→1 com contador50/150;3→2 com contador75/200 (primeiro valor quando+20!=0). Chama29:2100 com flagsDWORD1, portanto estágio1→0 pode desencadear vitória pelo contador global. A cadeia exata é espaço → comando seletor2 → estado7 →28:3367 →29:223F →29:2100 →29:005F →29:0FCE → cena+4C. O consumo de uma unidade de reserva por pulso e a redução3/10 são comprovados; a escolha do ponteiro de célula+4 é fechada na tabela de seleção WC abaixo.

## Fechamento dos produtores WB e FL e da seleção WC

### WB: destruição, atribuição de pontos e alternância

O produtor de `controller.+E4` é **28:187F**, dentro de **28:16B8**. Essa rotina percorre linhas9..0 e colunas0..5; chama28:0419 em cada bloco. A tabela28:18A7 despacha retornos1/3/4 para1884 (ainda há trabalho) e retorno2 para1712 (bloco acabou).0419 retorna2 exclusivamente pelo ramo04A4-04CB: chama064F para desligar as ligações do bloco, grava `block.+26=0`, troca a imagem para4056. Antes disso o crescimento de `block.+20` até8, ou `block.+2A==4`, conduz a esse ramo. Portanto E4 conta blocos efetivamente retirados, uma única vez: bloco em estado0 já não devolve2. O limite60 corresponde aos10×6 blocos percorridos.

Se o bloco retirado era a referência de topo da coluna `+B0+4*c`,1712-17CC procura a primeira linha0..9 com `block.+26!=0` e substitui o topo (ou grava ponteiro nulo). Em17D1, o turno `+C8` seleciona o acumulador do placar apontado por`+E6`: turno não zero soma ao campo+4 por5:2D43 (1823); turno zero soma ao campo+6 por5:2D72 (1874). O peso é2 quando`block.+2A==4`, senão1. Em seguida187F incrementaE4 independentemente do peso. O alias do placar é fechado por27:27B8 (empilha scene.+8E),27:27CB (chama353D),27:3601 (repassa argumento+12 a28:1060) e28:10D7-10DB (armazena em+E6); `+C8` começa1 (118A/164A) e28:1FD8-1FE2 grava `+C8=(+C8==0)`, alternando o turno. Não se deve substituir esse booleano por uma escolha aleatória nem somar pontos por quadro.

```text
para linha de 9 até 0:
  para coluna de 0 até 5:
    r = atualizar_bloco(grade[linha][coluna])
    se r == 2:
      atualizar_topo_se_necessario(coluna)
      peso = (bloco.tipo_2A == 4 ? 2 : 1)
      se turno_C8 != 0: placar.campo4 += peso
      senão: placar.campo6 += peso
      removidos_E4 += 1
    senão se r em {1,3,4}: existe_trabalho = verdadeiro
```

Os setters5:2D43 e2D72 gravam os WORDs+4/+6 e atualizam sua apresentação;27:2DA4-2DC2 zera ambos ao reiniciar. A comparação terminal continua estrita `placar.campo4 < placar.campo6`: empate é falso. Estes são acumuladores ponderados dos blocos retirados por turno, não contador de tempo ou quantidade de comandos recebidos.

### WC: alvo discreto da mira e travessia entre células

`28:3367` aplica água diretamente à célula no ponteiro`controller.+4`. Esse ponteiro muda pelas quatro rotinas direcionais; a mira não escolhe novamente uma célula por colisão no instante do disparo. A seleção inicial é mantida no controlador e a travessia segue os vizinhos da grade5×4.

| Rotina | Passo no objeto da mira | Condição para cruzar a borda da célula atual | Vizinho copiado para controller.+4 |
|---|---|---|---|
|28:2E6A|X−4 nos objetos+14 e+18|novo X do objeto+18 <= rect.left−60|cell.+8, escrita2F7E/2F82|
|28:2FB1|X+4 nos objetos+14 e+18|novo X do objeto+18 >= rect.right−4|cell.+C, escrita30C5/30C9|
|28:30F8|Y−4 no objeto+18|novo Y <= rect.top−60|cell.+10, escrita31B7/31BB|
|28:31ED|Y+4 no objeto+18|novo Y >= rect.bottom−4|cell.+14, escrita32AC/32B0|

Os retângulos vêm de`cell.+4->virtual+C`; as posições dos objetos visuais vêm do virtual+4 e são gravadas pelo virtual0. Os limites globais de movimento são os WORDs`controller.+26/+2A` para X e`+28/+2C` para Y. Ao atingir limite a rotina devolve1 sem mover além dele. Ao cruzar uma borda, só substitui o alvo se o vizinho existe, devolve1 e chama28:3470 com DWORD1 (horizontal) ou10000 (vertical), usando o alvo resultante. Antes de cruzar, devolve0 e conserva o alvo. Isso fecha a ligação entre comandos de direção, mira visual, seleção de célula, jato e decremento terminal.

### FL: incremento indireto e proteção contra repetição

O produtor que faltava está no segmento15, **15:2CD1**, e não num acesso direto a+86 do segmento16. O atualizador16:316C, estado vertical7 (tabela35EC, destino342B), empilha `controller.+44` (cena), `controller.+50` (pilha) e `controller.+4C` (objeto móvel), chamando **15:2C43** em343D. Assim o terceiro argumento da rotina, FAR pointer em`bp+E`, é comprovadamente a cena FL.

15:2C43 só entra quando`objeto.+48!=1`; marca+48=1 enquanto processa. Só executa o teste de captura enquanto`objeto.+4A==0`. Lê a posição do visual`objeto.+4` pelo virtual+4 e o retângulo do visual`pilha.+4` pelo virtual+C. Em2CBB-2CC1 compara **X do objeto <= borda direita do retângulo da pilha**. Se passa, chama15:22E3(pilha) e incrementa`scene.+86` por`les bp+E; inc [es:bx+86]` em2CCE-2CD1. Se falha, devolve1 para o chamador executar16:20AA. Nos dois ramos restaura+48=0, escolhe imagem49DF, chama16:1B74(scene) e grava`objeto.+4A=1`; esta flag impede recontar a mesma tentativa em atualizações seguintes.

15:22E3 incrementa`pilha.+12` em22EF. Ao alcançar3 pede parada ao controlador`pilha.+E` por37:17EC e executa o comando por32:009A; atualiza a apresentação por15:2378. Portanto`scene.+86` conta capturas que passaram por esse teste durante a animação de transferência. Não é o total de26 peças da lista livre nem o número de comandos de teclado: o produtor é o evento nativo de captura confirmado. O resultado positivo exige ao menos3 desses eventos, por16:164B.

```text
processar_transferencia(objeto, pilha, cena):
  se objeto.ocupado_48 == 1: retornar falso
  objeto.ocupado_48 = 1
  se objeto.processado_4A == 0:
    falhou = posicao(objeto.visual_4).x > retangulo(pilha.visual_4).right
    se não falhou:
      pilha.contagem_12 += 1
      se pilha.contagem_12 >= 3: parar(pilha.controlador_E)
      cena.capturas_86 += 1
    objeto.ocupado_48 = 0
    atualizar_cena_1B74(cena)
    objeto.processado_4A = 1
  atualizar_imagem(objeto)
  retornar falhou
```

O pseudocódigo usa nomes descritivos para os campos demonstrados, sem inferir unidade de tempo. As instruções e chamadas acima são evidência estática; nenhuma execução ou compilação foi usada.



## Callbacks e áudio dos scripts FL/HW

A tabela usa a ordem de operandos após39:13AB. Os registros foram comparados ao MMS da ISO porverify_behavior_map.py.

| Script e registro | Comando instalado pelo nativo | Efeito |
|---|---|---|
|FL0117,0x59587,slot0|16:0CDE constrói16:0166;16:0CF9 registra; tabela16:2CBA,+8=16:01C5|Retira/posiciona o objeto e configura o controlador, por1F6C,1C2E,11:22AC,1D90,2206 e15:1DB5.|
|FL0117,0x59DEB,slot1|16:0D31 constrói16:00BB;16:0D4C registra; tabela16:2CFE,+8=16:010A|Executor010A retorna sem operação. O callback tem um comando vazio nesta montagem.|
|FL0117,0x5A343/0x5A8EF/0x5AE9B/0x5B447,slot2|16:0D84 constrói16:0000;16:0D9F registra; tabela16:2D42,+8=16:004F|Chama16:36F1 no controlador+B8, grava estado+42=0 e rearma temporizador da cena+8C por11:22AC.|
|FL01B6,conclusão|16:0C29 constrói16:0371;16:0C42 registra por30:11E5; tabela16:2C32,+8=16:03C0|Chama16:1E6D para selecionar/configurar o próximo objeto e16:1726 para retomar os controladores.|
|HW0138,0x58DE7,slot0|19:0DB8 obtém37:17EC para scene+186;19:0DCD registra por30:1136|Executa o comando de controle da tarefa scene+186.|
|HW0138,0x58A3F,slot1|A montagem19:0C4A registra apenas slot0 no objeto+1B6|O registro consulta slot1. O executor30:1E70 chama32:009A somente quando o ponteiro retornado não é nulo. A presença do registro não cria um segundo comando.|

FL01B6 contém áudio em0x58E3F, ALexplosion01, e nenhum opcode0011. Seu retorno nativo usa o comando de conclusão acima.

FL0117 solicita ALbulldozer em0x5906B, FLdirtSound em0x5936B, FLthud em0x59497, CdRmOopsSorry01 em0x59E0F, CdRmOopsSorry02 em0x5A373, CdRmYeeIkes01 em0x5A91F e CdRmYeeIkes02 em0x5AECB. Os rótulos e saltos que condicionam esses registros estão em mms-control-flow.json.

HW0138 não contém opcode0010 de reprodução de som. Seus eventos visuais e comandos são tratados pelo interpretador e seus controladores nativos. A recompensa comum segue o ID de progresso18 e o filme21.

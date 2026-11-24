# Gatilhos de áudio nos scripts MMS

`mms-audio-event-map.json` registra os 441 gatilhos identificados nos 122 scripts de raiz 8003. Cada linha mantém raiz, posição no script, offset no arquivo, ID de áudio, todos os rótulos de entrada com caminho estrutural até o gatilho, seletores de slot possíveis e testes de slot no caminho.

Para reproduzir, executar `python tools/trace_mms.py` e `python tools/map_mms_audio_events.py`. Os programas leem as tabelas já identificadas. O verificador `tools/verify_behavior_map.py` compara os dados da tabela com a ISO.

## Alcance dentro do interpretador

O salto incondicional opcode 12h segue o primeiro marco com o rótulo, como em 30:1AE0. FFFF ou rótulo inexistente encerra o fluxo. O opcode 13h faz o salto quando o slot está ocioso e segue para o próximo registro quando ocupado. Os demais registros seguem a sequência. Um salto pode cruzar marcos intermediários. Por isso, o marco textual anterior a um som não basta para identificar quem o aciona.

Há 322 gatilhos com caminho estrutural a partir do primeiro marco. Dos outros 119, todos têm caminho a partir de algum rótulo. Os 441 ficam cobertos por origem interna ao script. Um caminho estrutural considera as duas respostas possíveis em cada teste de slot. Para provar execução em uma jogada também é necessário demonstrar quem entra no rótulo, os resultados dos testes e a continuação após os callbacks. `selector_predecessors` guarda as últimas instruções 0Fh possíveis antes de cada 10h. `entry_can_retain_slot` indica caminho sem seletor entre alguma entrada candidata e a reprodução.

## Entradas demonstradas no código de cena

| Cena / origem | Raiz | Rótulo ou entrada | Evidência |
|---|---|---|---|
| Oficina, quatro áreas de efeito | 0141, 014F | 2 ou 3 | 8:0F86, 1068, 114A e 122C constroem 2:2772. O comando sorteia entre os dois rótulos e aplica 30:0FBD à animação associada. As coordenadas de cada área estão em 17. |
| Cabine, fim do combustível | 0111 | 400 | 13:10B6 dispara comando 10 quando o acumulado cruza o limite. 12:2AD8 para o movimento, marca +ED e agenda rótulo 400. O registro 23667 solicita 4373. A cadeia 401/402 aguarda o som e dispara slot 4 em 23AFF. |
| Memória, acerto | 01A4 | 8, 10, 12, 14, 16 | 27:1481 seleciona tabela DS:10BA, evitando repetir o índice +14C. Os sons são 4073, 4074, 406E, 407A e 4085. |
| Memória, erro | 01A4 | 18, 20, 22, 24 | 27:1512 seleciona tabela DS:10C4, evitando repetir +14E. Os sons são 4099, 409D, 409E e 409F. |
| Memória, remoção do último par | 01A4 | 26 ou 28 | 27:11E4 verifica +13A==12 e chama 16E3, que sorteia a tabela DS:10CC. Os sons são 42E0 ou 42E1. |
| Memória, rotina alternativa | 01A4 | 30 ou 32 | 27:1753 usa DS:10D0 e escolhe uma das duas entradas, com sons 4092 e 42D9. A origem da chamada à rotina exige confirmação separada. |
| TC, montagem e comando 10 | 0119 | 7 sem cartão, 2 com cartão | 25:09D7–0A09 e 1586–15C6 chamam a seleção via 1656. A entrada 7 alcança AF853/42CE. A entrada 2 alcança AE24B/42CA e AEADF/42CC. |
| TC, comandos 1 e 7 | 0119 | 1 | 25:0C7F e 1546. O grafo a partir desse rótulo não alcança instrução 10h. |
| TC, comando 2 | 0119 | 7 | 25:0F67. Alcança AF853/42CE. |
| TC, comando 3 | 0119 | 9 | 25:1060. Caminhos estruturais chegam a B037B/42C8, B0BC7/42CD, B403B/42CB e B4893/42CF. |
| TC, comando 6 | 0119 | 13 | 25:1468. Caminhos estruturais chegam a B15CF/42C9 e B1D43/42D2. |
| TC, comando 9 sem cartão | 0119 | 17 | 25:13F7. Alcança B2703/42C7. |
| TC, confirmação válida | 0120 | início | 25:1362 instala TC11 como comando final e 1375 inicia a animação. |
| CT, rejeição do ciclo | 01A5 | 9 ou 25 | 10:2997 testa composição e seleciona 9 se o primeiro teste falha, 25 se o segundo falha. Os sites A644B/4219 e A821B/4218 pertencem aos respectivos ramos. |
| CT, fases do ciclo | 01A5 | 12 e 13 | 10:272B encaminha a 11:186E no controlador +8E. Os rótulos têm caminho estrutural até A76C3/4041. |
| GW, tutorial conforme variante | 0184 ou 0185 | entrada do tutorial | 18:38A2 seleciona pelo campo +A8. O caminho com tutorial 0184 contém a instrução de giro, ausente em 0185. |
| FM, rota de resgate | 011C, 011D, 011E, 011F | início da animação direcional | 17:22C4 seleciona por direção 3, 2, 1, 0. As quatro raízes disparam 43A0 e registram comandos no owner. |
| Atividades da base, sucesso | 0112 | início | 5:1829 registra 2:2671 como conclusão. O script dispara 4032 em 5578F antes de concluir pelo comando registrado. |
| Atividades da base, falha | 0116 | início | 5:18E0 registra 2:25BC. O script dispara 4030 em 55C37 antes do registro de tentativa. |

Os relatórios 12, 13, 16–20, 22 e 23 trazem as cadeias completas citadas nesta tabela. Os 441 registros estão contabilizados no arquivo de dados. A atribuição ao evento nativo de todas as demais entradas e a viabilidade semântica de todos os caminhos condicionais continuam abertas. Um nome de som, proximidade no arquivo ou rótulo candidato isolado não prova origem de cena.

## Seleção de roteiro por cena e contexto

`mms-native-root-sources.json` guarda as duas tabelas de 25 seletores lidas dos bytes do executável e 39 chamadas com argumento literal em 30:04DA. O primeiro grupo é alcançado pelo virtual +48 padrão 5:1360, que indexa DS:03A2 com o argumento de cena. 5:0EF1 consulta o virtual, constrói o roteiro em 5:0F4C e monta comandos de início e salto ao rótulo 3 em 0F87 e 0F9F. A segunda tabela, DS:03D4, é usada por 5:1374 após a guarda 2:0D82 em 138B. 13CA carrega o roteiro pelo argumento de cena e 13F9 constrói o comando de início. A linha com FFFF continua como valor especial do original. Os valores funcionam onde se usa o virtual padrão, respeitando substituições na tabela virtual de cada objeto.

| Argumento de cena | Roteiro padrão | Alternativo |
|---:|---|---|
| 0 | 017C | 01A6 |
| 1 | 0182 | 01AD |
| 2 | 0188 | 01B4 |
| 3 | 018B | 01A7 |
| 4 | virtual próprio | 01B2 |
| 5 | 0191 | 01B1 |
| 6 | 018A | 01A6 |
| 7 | 0190 | 01A8 |
| 8, 9 | FFFF | FFFF |
| 10 | 017C | 0119 |
| 11 | FFFF | 0118 |
| 12 | 0180 | 01B0 |
| 13 | 0189 | 01A9 |
| 14 | FFFF | FFFF |
| 15 | 0181 | 01AE |
| 16 | 0184 | 01AB |
| 17 | 0183 | 01AC |
| 18 | 017F | 011A |
| 19 | 017E | 01AF |
| 20 | 0187 | 01B3 |
| 21 | 0186 | 01AA |
| 22–24 | FFFF | FFFF |

Na cena 4 o slot próprio em 9:2273 aponta a 9:1EB5. A função lê DS:02A0 e devolve 018F no tema 0, 018E no tema 1 e 018D nos demais. O seletor é lido em 5:0F3D. Os três roteiros somam 15 instruções de áudio.

Na oficina, chamadas a 5:0CF8 montam 0144 em 8:02D8, 0143 em 0E59, 0145 em 0E6E, 014F em 0E86, 0141 em 0EBB, 0142 em 0EDB, 0146 em 0EF0, 0147 em 0F05, 0148 em 0F1A e 0153 em 0F2F. O ramo de tema lê DS:02A0 em 8:12DA. Tema 0 monta 014E, 014C e 0152. Tema 1 monta 0149, 014D e 0151. Os demais montam 014B, 0150 e 014A. 5:0D2C cria o roteiro e 0D5D anexa ao proprietário. Quando os parâmetros de seleção são zero, 0D77 constrói reinício no rótulo 2. Quando presentes, 0DB3 constrói seleção aleatória pelos dois parâmetros. 0DCF instala esse comando por 30:1263 e 0DEB habilita o roteiro por 30:0DA8. O clique pode selecionar um rótulo adicional nas áreas de 17.

Na cena 3, 11:0694 associa 0124 aos parâmetros 2,2. 11:06AF monta 0126 e 06C4 monta 0130 com parâmetros 0,0. 06DC monta 013A com parâmetros 2,3. Todos usam 5:0CF8 e o mesmo mecanismo de preparação de sequência.

O painel de recompensa seleciona seis raízes adicionais pelo ID de progresso. O construtor 3:037E chama 3:1086. A rotina cria a animação após verificar a disponibilidade do filme em 10DE–10E7. 11DC chama 12F1 com o ID em owner+6E. 12F1 compara a tabela de chaves DS:024C e devolve a raiz de DS:0258 para 30:04DA em 11E7.

| ID de progresso | Raiz de recompensa |
|---:|---|
| 5 | 012E |
| 14 | 012D |
| 22 | 012C |
| 26 | 012B |
| 29 | 012F |
| 34 | 012A |

Na cena 1, 23:1F70 usa o tipo da interação e o caso da célula para selecionar 00D9, 0133, 0134 ou 0135 entre outros roteiros. As quatro raízes que contêm áudio estão incluídas no mapa. O relatório 13 documenta a tabela de seleção, o deslocamento do objeto e os callbacks de descongelamento do agente.

O cruzamento cobre a **origem de carregamento das 98 raízes sonoras** e os **441 sites de reprodução** dentro delas. Isso é uma cobertura de duas etapas. Exemplo de cadeia integral com estado nativo identificado é memória, em que as tabelas DS:10BA, DS:10C4 e DS:10CC determinam o rótulo para acerto, erro e rodada concluída. Para as demais raízes, quando há múltiplas entradas, os rótulos em JSON estabelecem alcance estrutural. Permanecem por demonstrar os acionadores externos de todos os rótulos alternativos, incluindo a origem de 27:1753.

## Índice de chamadas externas

O arquivo mms-external-label-calls.json registra 106 instruções far com alvo explícito nas listagens NE. São 90 para o construtor de comando 30:0FBD, 14 para o agendador 30:0DA8 e duas para 30:1AE0. Cada linha guarda segmento, endereço da instrução, endereço da relocação e a janela anterior de argumentos. A janela permite conferir valores empilhados e reconstruir a origem do ponteiro de animação. O índice não inclui despacho indireto nem estabelece por si só o evento que alcança cada chamada.


## Rastreamento das entradas alternativas

O comando 30:0F50 passa a 30:263D o descritor em DS:116E. O comando de interrupção e reposicionamento 30:0FBD passa DS:1176. Os dois ponteiros relocados apontam a 30:0DA8 e 30:0E08, respectivamente. O método 30:26D9 lê o rótulo armazenado em +32 e executa o ponteiro em 270C. 30:0DA8 usa 30:1AE0 em 0DBC e agenda o objeto. 30:0E08 cancela o agendamento anterior se ativo em 0E26, busca o rótulo em 0E39 e reagenda em 0E46.

Na oficina e na cena 3, 5:0DE5 passa explicitamente o rótulo 1 a 30:0DA8 em 0DEB. 5:0DCF instala o comando de nova seleção no campo +2F. 5:0DDD configura máscara 7 no wrapper por 30:1232, cujo virtual +8 aponta a 33:16DC e grava o valor. O método +4 do wrapper 30:280C aceita evento com bit 2, percorre alvos em ordem reversa, exige acerto do virtual +24 em 2876 e executa pelo virtual +14 → 33:1750 → 32:009A. O clique usa rótulo 2 quando os dois limites são zero. Os limites de 0141, 0142, 014F e 013A são 2 e 3. Os de 0124 são 2 e 2. A escolha por limites usa 2:27CF → 2:0012 → 30:0FBD.

A ajuda padrão prepara início no rótulo 1 em 5:0F87 e reposicionamento no rótulo 3 em 0F9F. A alternativa prepara os mesmos rótulos em 5:13F9 e 140D. Na alternativa, 1479 registra o callback do slot 0 e 14A9 executa o início. Na tela de cartão TC, a confirmação válida inicia explicitamente 0120 pelo rótulo 1 em 25:136A-1385. Em CT, 11:177E inicia 01A5 no rótulo 1 ou 50. Os locais 11:0DFC e 1751 preparam o rótulo 3.

Na cabine, todos os rótulos da tabela abaixo pertencem à raiz 0111. Os valores dos rótulos estão em decimal.

| Evento | Rótulos | Evidência nativa |
|---|---|---|
| Medidor cruza 50%, 100%, 109% ou 120% | 2, 10, 11, 400, respectivamente | Instalação 12:067D-06C1 e despacho em 13:10BF. |
| Percurso em terreno de limite 5 | 15, 16, 17, 18 | 15:0621-067F dispara movimento+72. 12:0947,0957 associa seletor 2. 12:278F usa a sequência circular +7F montada em 1086-10D9. |
| Percurso em outro terreno | 19 a 26 | 15:0682-06A7 dispara movimento+76. 12:0965,0975 associa seletor 9. 12:27E5 usa a sequência circular +B3 montada em 11F1-12B5. |
| Colisão com descritor que aciona seletor 0 | 27 ou 30 | 12:2662 alterna +77/+7B, construídos em 0F13,0F2D. 14:25FE classifica os símbolos. |
| Entrada em célula com posto | 60, 80 ou 90 | 14:2B53 constrói seletor 4, que 12:283C usa para sortear +8F/+93/+97 criados em 10F5,1111,112D. |
| Entrada em célula com filme | 5, 40, 41, 42, 43 ou 44 | 14:29BC fornece ID de filme por símbolo/tema. 12:290D guarda em +125 e sorteia +9B a +AF. 12:1149-11D5 define os rótulos. |
| Retomada pelo método virtual +34 | 50 | Fixup 12:30E2 aponta para 12:2DFD. Em 2E06 chama +D3, criado em 12D1. |
| Ativação do controlador | 1 | 12:2DE6-2DF0 passa o rótulo a 30:0DA8. |

Para a rotina de memória em 27:1753, foram verificados os 13.487 destinos de relocações da imagem, as três entradas NE e as instruções de desvio/chamada direta no segmento 27. Nenhum destino aponta ao intervalo 1753–17C2. Também não há o WORD 1753 nos dados dos segmentos em ordem little endian. O corpo começa depois da instrução de retorno 1752, que fecha a rotina precedente. O corpo preservado carrega os rótulos 30 e 32 de DS:10D0, sorteia por 2:0012 e constrói 30:0FBD em 178B. Os sites de áudio contêm 4092 e 42D9. O produtor do comando permanece sem referência estática identificada. A busca não exclui um destino calculado em tempo de execução.


## Outros eventos com cadeia demonstrada

| Origem | Raiz | Rótulos e regra |
|---|---|---|
| CP, total de certificados | 0173 | 9:1944 lê o total por 6:0444. Os casos 0 a 3 entregam 6, 8, 10 e 12 por 9:1989 → 1E68. |
| Oficina, segunda parte da apresentação | 017D | 8:19F0 guarda o roteiro em +82. 8:1D01 cria o salto ao rótulo 1 e 1D15 o registra no slot 1 da animação +58. |
| Oficina, diálogo interativo | 011B | 8:13BF guarda em +64. 141F,1433,15D9 e 16A6 criam comandos para os rótulos 5, 7, 2 e 1. |
| Oficina, passos do personagem | 0154 | 8:0DB7 carrega o roteiro e 0E0F entrega ao executor. 2547 aguarda ciclo de 30.000 ms, 2563 sorteia 2 a 10, 2577 aplica o rótulo e 25A8 volta ao laço. |
| FL, ação durante movimento | 0117 | 16:32BE exige estado +42 diferente de zero e animação disponível. Em 32E2 soma 5 ao estado. 34A3 sorteia 1 a 100. Se o resultado é até 10, 34BF sorteia 1 a 4, produzindo os rótulos 6 a 9 após a soma. O ramo restante deixa +42 zero. |
| FL, efeito de preparação | 01B6 | 16:0BE1 carrega e 0C42 arma o comando final. O ramo 137A inicia rótulo 1 quando cena+D0 está zerado. |
| FM, captura no alvo | 01B5 | 17:2199 é acionado para tipo 9. 228C seleciona rótulo 1 e 229C executa. O callback de remoção da célula está no relatório 13. |
| FM, conclusão de resgate por direção | 011C–011F | 17:22C4 escolhe a raiz por direção e 244B prepara rótulo 1. |
| SB, jato por tipo de célula | 00D9, 0133–0135 | 23:1F70 escolhe bloco, 2320 configura os callbacks e 2434 posiciona. O despacho entra no rótulo 1, conforme relatório 13. |
| Base de atividades, resultado | 0112, 0116 | 5:1A71/1AA5 inicia rótulo 1 no objeto respectivo de sucesso/falha. 1829/18E0 instalam as conclusões. |
| Recompensa de progresso | 012A–012F | 3:11E7 guarda o roteiro selecionado em +AE. 09D9 cria a entrada 1 e 09F2 cria o reposicionamento para 99. |

O arquivo de dados cruza **422 de 441 sites de áudio** com ao menos um caminho estrutural a partir de um rótulo vinculado a código nativo. Há **19 sites** sem esse cruzamento. As duas métricas anteriores continuam válidas. Todas as 98 raízes sonoras têm origem de carregamento identificada, e os 441 sites têm ao menos uma origem estrutural no script. O cruzamento de um site com uma entrada nativa continua condicionado aos estados e callbacks indicados no respectivo caminho.

| Raiz | Rótulo necessário | Offset do site | Áudio |
|---|---:|---:|---|
| 0111 | 12 | 2376F | 4375 TuGoodVeryGoodIdea |
| 0111 | 13 | 23877 | 4374 TuGoodOhCapitolIdea |
| 0111 | 14 | 23B53 | 0008 TuLookoutTooClose |
| 01A4 | 30 | 5E95B | 4092 CdNiceTryItsaBigJob |
| 01A4 | 32 | 5EE77 | 42D9 CdWhyNotTryLater |
| 011B | 12 | 60FE7 | 41B9 ChWhatFarmersWear |
| 011B | 14 | 61617 | 41BA ChWhatFireWear |
| 01A5 | 14 | A593B | 4035 AxCoolTruck01 |
| 01A5 | 17 | A77FB | 4038 AxExcellent |
| 01A5 | 18 | A7933 | 403A AxGoodChoice |
| 01A5 | 19 | A7A6B | 4040 AxNiftyKeen |
| 01A5 | 20 | A7BA3 | 403C AxMagnificent01 |
| 01A5 | 21 | A7CDB | 4049 AxYoureAGenius01 |
| 01A5 | 22 | A7E13 | 403E AxMarvelousInv |
| 01A5 | 23 | A7F63 | 4039 AxExquisiteDetails |
| 01A5 | 24 | A80B3 | 4043 AxSuperbLines |
| 01A5 | 26 | A8383 | 4216 AxFinishTruckBefore |
| 01A5 | 27 | A84D3 | 4215 AxAxelDriveTruck |
| 0119 | 19 | B2F5B | 42D1 RmPickCardComeIn |


Para 0111, a listagem exaustiva de todas as 30 chamadas `30:0FBD`/`30:0DA8` no segmento 12 da cabine confirma exatamente o conjunto `{1, 2, 5, 10, 11, 15..26, 27, 30, 40..44, 50, 60, 80, 90, 400}` — nenhum pedido para 12, 13 ou 14. Para 01A4, a rotina `27:1753` (rótulos 30 e 32) também tem zero entradas nas 99 jump tables em CS e 274 vtables de `analysis/recomp-cfg.json` (`38-CFG-FUNCOES-VTABLES.md`). Para 0119, as chamadas diretas a 25:1656 fornecem 1, 2, 7, 9, 13 e 17; a chamada separada em 25:13B3 fornece 60, enquanto o site 0xB2F5B requer 19. Para 01A5, os usos de +8E de interesse são 11:0DFC,1751,177E,1893; 11:1893 recebe argumento por 11:186E, cuja única relocalização na imagem é a chamada direta 10:2746 dentro de `10:272B`. Os únicos quatro chamadores de `10:272B` (`10:29F0`, `10:2A33`, `10:2BEF`, `10:2C69`) passam os literais `{9, 25, 12, 13}` e `seg11` passa `{1, 3, 50}` — nenhum `14, 17..24, 26, 27`; o roteiro 01A5 também não contém opcode 11h. Para 011B, os rótulos 12 e 14 ficam fora de todas as chamadas em `seg08` sobre a animação +64. Esses resultados fecham os 19 sites restantes como rótulos de script não acionados pelo executável compilado; um rótulo sem produtor no binário não deve ser acrescentado como evento artificial durante a implementação.

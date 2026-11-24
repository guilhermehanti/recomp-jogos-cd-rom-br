# Big Job BR — rastreamento estático do motor

Os endereços abaixo usam o número decimal do segmento NE de BIGJOBNG.EXE e o offset hexadecimal dentro dele. Os offsets do MMS são relativos a BIGJOB.MMS no LBA 8162.

## Do registro MMS ao relógio

1. O segmento 36 contém a assinatura `MMScript File` em 36:0017. O despacho em 36:0B59 usa a tabela em 36:0D88. O tipo estrutural `C002` aponta para 36:0BF0. Em memória, os registros têm passo de 12 bytes. O seletor interno fica no primeiro word do registro e a carga começa no quarto byte, conforme os acessos em 36:049A e 36:0539.
2. O executor de animações em 30:1D43 lê o seletor por 36:049A. A tabela em 30:1F7B encaminha `000C` a 30:1F42, `000D` a 30:1D62 e `000E` a 30:1DBB. Isso distingue o tipo de registro `C002` do comando `000C`.
3. O caso `000C` copia os oito bytes de parâmetros via 36:0539 e chama 30:1F8D. Em 30:1F93–1FA2, o primeiro word é testado como inteiro com sinal e, se positivo, armazenado diretamente no campo `+0x53` de 32 bits. Em 30:1FA9–1FB5, valores menores ou iguais a zero, incluindo `FFFF`, armazenam `0x7FFFFFFF` e ligam a flag `+0x69`. Esse ramo usa um prazo sentinela. A condição que libera o avanço a partir desse estado ainda precisa ser rastreada.
4. O relógio do laço em 38:3045–304F vem de `MMSYSTEM.timeGetTime` e é gravado em `+0x12`. O executor em 30:194C–1964 usa esse valor menos o tempo acumulado em `+0x16`, gravando o resultado em `+0x61`. O cálculo de prazo em 30:1C41–1C4B soma diretamente `+0x61` e `+0x53`. O ramo de prazo acumulado em 30:1C08–1C15 também soma `+0x53` sem conversão.

**Conclusão estática**. Os quatro comandos com `0032` configuram **50 ms**. Os comandos com `FFFF` usam o ramo de espera sem prazo finito. O identificador do clique da cabine, a condição de liberação e o destino exato da transição permanecem abertos. A prévia atual usa 50 ms para todos os seis quadros, portanto ainda difere do fluxo do motor nas extremidades.

## Áudio brasileiro

| Banco | LBA | Entradas | Tamanho | Tags por entrada |
|---|---:|---:|---:|---|
| BIGJOB.SND | 182928 | 33 | 3.202.935 bytes | 33 × 0x0012 |
| BIGVOX.SND | 206494 | 929 | 95.038.318 bytes | 927 × 0x0012 e 2 × 0x0011 |

Ambos começam com `MMFW Sounds`, têm contagem BE16 em 0x66 e tabela de offsets BE32 a partir de 0x68. O último offset de cada tabela coincide com o tamanho do arquivo. As duas entradas `0x0011` de BIGVOX.SND são 819 e 870, com índice iniciado em zero.

As entradas observadas começam com a tag BE16 e outro campo BE32, seguido por bytes de amostra. O campo BE32 é consistente com duração inteira em milissegundos, cruzando o comprimento de dados com a taxa, com discrepância compatível com arredondamento. Por exemplo, BIGJOB.SND[5] tem 66.324 bytes após o cabeçalho e campo 2.980. BIGVOX.SND[819] tem 17.975 bytes após o cabeçalho e campo 1.615. A fórmula exata do arredondamento fica pendente.

O seletor em 36:23A7–23E5 usa os dois bits baixos da tag e devolve 11.025, 22.050 ou 44.100 Hz para os códigos 1, 2 ou 3. A taxa deduzida dos tamanhos e durações declaradas é aproximadamente 11.128 ou 22.255 bytes/s. Logo, os tempos do cabeçalho e a taxa nominal de reprodução devem ser tratados separadamente. Em 39:011C–0154 há evidência de saída PCM mono de 8 bits a 22.050 Hz. Os fixups identificam `waveOutOpen` em 39:0155 e `waveOutWrite` em 39:0338. Ainda falta verificar o caminho de amostras até esse dispositivo para os dois tipos de entrada e associar cada som ao evento que o seleciona.

## Direção e fronteiras pendentes

- BIGJOBHD.PIC contém o painel em 279, bordas em 281 e 286, horizontes da cidade, fazenda e incêndio em 319–327, chave em 328–330, rádio em 331–335, velocímetro em 336–346 e volante em 347–357. Esses índices são dos recursos visuais. O mapa de coordenadas e dos eventos de cada controle ainda precisa vir do motor.
- BIGJOB.DAT aparece três vezes no segmento 14. A função 14:02DE passa esses caminhos em sequência às chamadas nos offsets 14:0335, 14:0375 e 14:03B5, parando ao obter sucesso. O arquivo da ISO tem 130.392 bytes e começa por `4D 4D 00 6B 00 C7 00 0B`. Ainda não há vínculo comprovado entre seu conteúdo e a pista. A leitura deve ser seguida a partir das chamadas de 14:0335 e 14:03C5.
- Para fechar a entrada da cabine, localizar o objeto interativo e seguir seu callback até a escolha do comando de animação. Cruzar o identificador com o bloco da cortina no MMS e seguir a saída para determinar a cena exata.
- Para fechar a atividade, mapear o callback de atualização da cena, a leitura de controles, o estado do veículo, a geração da pista e o vínculo numérico entre eventos e amostras. Para vídeo, o ponto de partida continua sendo a cadeia de MMMovieQuickTime no segmento 33.

Nenhum executável do jogo foi iniciado. A prévia gráfica não foi alterada nesta investigação.

## Continuação — espera, reinício e aplicação de estado

| Evidência em BIGJOBNG.EXE | Comportamento observado |
|---|---|
| 30:1933–193C → 37:19AB | O executor repassa o prazo de 32 bits do objeto de animação, campo `+0x5D`, à infraestrutura de espera. |
| 37:19B8–19CA → 37:1AB5 | A infraestrutura copia esse prazo para `+0x32`, zera `+0x36` e entra na rotina de espera. |
| 37:1B58–1BBB | O teste de avanço retorna zero quando `+0x36` é nulo e o prazo em `+0x32` é `0x7FFFFFFF`. O ramo de comparação com o relógio só é alcançado para um prazo finito. Existe um caminho separado para `+0x36` não nulo, inicializado pela entrada alternativa 37:19DE. |
| 37:17C6 | Um caminho separado zera os dois campos da espera e liga a flag global `0x123C`. O fixup em 30:1ABA aponta a essa rotina. Esse vínculo identifica um cancelamento explícito disponível no executor, sem atribuí-lo ainda ao clique da cabine. |
| 30:22DE–2338 | A reinicialização toma o tempo corrente efetivo, grava-o nos campos `+0x59`, `+0x5D` e `+0x61`, e zera a compensação `+0x65`. Há chamadas em 30:1862 e 30:1AC5. |
| `000E`: 30:1DBB → 36:0611 → 34:0036, depois 30:2142 | Há resolução de um valor a partir do registro. O executor resolve um objeto, obtém seu estado por chamada virtual, compara seis bytes desse estado com a entrada e só chama a atualização quando há diferença. O formato completo do valor resolvido ainda precisa ser decodificado para mapear cada registro da cabine ao recurso concreto. |

O prazo sentinela de `FFFF` é reconhecido pelo teste de avanço. Portanto a lógica fiel precisa ligar a saída dessa espera ao evento ou ao caminho de cancelamento real. A localização do controle da cabine e a associação de seus sons continuam abertas.

## Índice do MMS e referências à cortina

Foram verificadas as seguintes entradas do índice no próprio MMS. O offset aponta para um par BE16 de tamanho e contagem, seguido por registros de 12 bytes. Em cada um dos três blocos, tamanho = contagem × 12, e o fim coincide com o início do bloco seguinte.

| Entrada no índice | ID | Início | Registros | Fim exclusivo |
|---|---:|---:|---:|---:|
| 0x0DAE | 0x0190 | 0x4DA4B | 231 | 0x4E523 |
| 0x0EF2 | 0x013B | 0x61C77 | 23 | 0x61D8F |
| 0x0EFE | 0x013C | 0x61D8F | 74 | 0x6210B |

- **0x013B** contém os seis estados alinhados da cortina. Os quatro registros estruturais de abertura em 0x61C7B–0x61C9F precedem seus comandos temporais. O bloco termina com `C000 0011` em 0x61D83.
- **0x013C** também referencia o objeto `00CA` em `(210,119)`, nos registros em 0x61E17 e 0x61E5F. Alterna esses estados com outros objetos e comandos de 100 ms. Essa segunda animação deve ser analisada separadamente da sequência de seis quadros.
- Em **0x0190**, o registro em 0x4E007 contém `C005 0010 013C 41B0`. O caso `0010` do executor em 30:1E34 encaminha a referência por 36:05C3 e 30:224B. Em 30:224B o motor consulta a tabela de objetos por 30:0D87 e passa o alvo e a carga da referência a outra chamada far. Esse é um vínculo concreto entre os blocos, sem identificação ainda do evento de entrada em 0x0190.
- O cabeçalho do MMS contém `C001 0058 0112 013B` em 0x42A. O operando 0x013B coincide também com uma coordenada horizontal plausível. O campo precisa ser interpretado pelo carregador da tabela antes de ser usado como referência à animação.

## Código nativo da cabine e áudio por identidade

O segmento 12 contém uma cadeia de montagem da cabine. Em 12:138B, 12:13E3 e 12:143B ele empilha, respectivamente, os índices PIC 279 (`DSbackgroundDownMid`), 281 (`DSbackgroundUpLeft`) e 286 (`DSbackgroundUpRight`). Todas as três chamadas de recurso têm fixups para 37:011E. Os objetos retornados são enviados à mesma rotina em 35:3B22. Isso vincula o segmento 12 à cabine por três índices, ordem e destinos de relocação, além dos nomes no PIC.

Há chamadas do segmento 14 para 12:2441, 12:2579 e 12:2615 e do segmento 15 para 12:3078, 12:308E, 12:321F e 12:34BF. Esses são pontos para reconstruir o fluxo em torno do módulo da cabine. O vínculo entre uma função específica de 14 e os registros do DAT ainda exige seguir os objetos e argumentos entre essas chamadas.

As tabelas de nomes dos dois SND começam em `108 + 10 × contagem` e usam campos de 32 bytes. Os índices abaixo são de base zero e vêm dessas tabelas:

| Banco | Índices | Identidade |
|---|---|---|
| BIGJOB.SND | 0 | DShorn |
| BIGJOB.SND | 1–3 | DSradioLoop01–03 |
| BIGJOB.SND | 4–10 | Avisos sobre estrada, colisões e direção, pelos nomes internos |
| BIGJOB.SND | 11–13 | Avisos de combustível |
| BIGJOB.SND | 14–21 | Comentários de desempenho e aceleração |
| BIGJOB.SND | 22–28 | Instruções de marcha |
| BIGJOB.SND | 29–32 | Instruções de direção, incluindo `TuToSteerPointMouse` no índice 32 |
| BIGVOX.SND | 45–47 | ALtruckBig, ALtruckLoop, ALtruckStartUp |
| BIGVOX.SND | 878–882 | DScabScreen, DSgasPump, DSgasStation, DSturboLoop, DSturboStart |

A classificação acima deriva dos nomes internos. Os gatilhos e a política de repetição de cada som ainda precisam ser obtidos do código. `TuToSteerPointMouse` dá evidência adicional para investigar o mouse como entrada original do volante.

## BIGJOB.DAT — layout cruzado com o leitor original

O arquivo brasileiro começa com três palavras BE16: assinatura `4D4D`, limite inferior 107 e limite superior 199. A rotina 14:03EA–0437 lê os limites e calcula a quantidade inclusiva `199 - 107 + 1 = 93`. Em 14:044B–0485, aloca dez blocos de `93 × 0x8C` bytes. Em 14:0492–04BC, carrega cada bloco por 39:10EE, pedindo `93 × 0x46` palavras. Em 14:04D1–04E8, lê outra tabela de 93 palavras para o campo `+0xC52` do objeto.

| Região | Primeiro offset | Tamanho |
|---|---:|---:|
| Cabeçalho | 0x00006 − 6 | 6 bytes |
| Dez tabelas de 93 registros de 140 bytes | 0x00006 | 130.200 bytes |
| Tabela final de 93 palavras | 0x1FC9E | 186 bytes |

A leitura estática conferiu os limites das dez tabelas e o fim exato do arquivo, em 130.392 bytes. Em 14:1587–15F8, o motor inicia a iteração no limite inferior e usa valores da tabela final em cálculos de deslocamento. Em 14:15FA–1695, combina valores de registro com campos do objeto e aplica limites ou ajustes de coordenadas. Isso liga o DAT a cálculos espaciais do segmento 14. Para uma tradução fiel, a semântica das 70 palavras de cada registro deve vir dos consumidores, e não da posição dos campos no arquivo.

## Encadeamento de reprodução e estados da cabine

- Em 36:28F7–2936, a rotina de solicitação de áudio encaminha a referência para 36:1710. Em 36:1735, a máscara `0x3FFF` seleciona a entrada e 36:19AD usa os dois bits superiores para consultar a tabela de quatro bancos. A numeração dos bancos deve ser resolvida pela montagem da tabela em execução ou por suas inicializações estáticas.
- Em 12:2672, o código da cabine chama o caminho de interrupção/consulta do som corrente antes de emitir `0x0001400E` em 12:268D. O índice decodificado é 14 no banco lógico 1, que corresponde a `ALcrash01` se o banco lógico 1 estiver montado com BIGVOX.SND. O mesmo trecho alterna um estado binário no byte `+0x72` e atualiza objetos associados. A rotina de detecção de colisão que entra nesse trecho ainda deve ser identificada.
- Em 15:23F3, outro chamador emite `0x0001402D`, índice 45 do banco lógico 1, correspondente a `ALtruckBig` sob o mesmo mapeamento. Em 15:2ECB e 15:300D, os valores `0x00014387` e `0x00014388` resolvem os índices 903 e 904, `FLloaderForward` e `FLloaderReverse` em BIGVOX.SND. Logo, o segmento 15 também contém rotinas de outras atividades. O segmento inteiro deve ser classificado por função antes de ser atribuído à cabine.
- Em 12:353A–35CC, um despachante consulta o estado `+0x2A`, aplica uma tabela de cinco casos e encaminha para rotinas do segmento 15. Em 15:0743–0778, um ajuste discreto move `+0x62` em direção a `+0x60`, com limite superior `+0x64`. O laço que o chama mede tempo decorrido com `timeGetTime` em 15:071B–0732. Esses campos ainda precisam ser nomeados pelos controles que os escrevem.

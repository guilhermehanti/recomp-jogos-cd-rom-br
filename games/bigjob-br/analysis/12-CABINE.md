# Cabine: cadeia estática da cena 2

Análise somente estática de BIGJOBNG.EXE, segmentos 12–14, relocações NE expandidas e registros de BIGJOB.MMS em mapa-jogo-v3.json. Nenhum executável original foi iniciado e nenhuma compilação foi realizada. Endereços `segmento:hex` são relativos ao segmento NE.

## Correção essencial do formato MMS

A análise anterior que rejeita o callback 4 em 0x23AFF está incorreta. Os seis words de mapa-jogo-v3.json estão em ordem **big endian do arquivo**. O leitor 36:01CD chama 37:0456 em 36:0299. Depois da leitura de `n*12` bytes, 37:04B8–04C3 passa `n*3` DWORDs a **39:13AB**. Essa rotina troca bytes 0/3 e 1/2 de cada DWORD (39:13C0–13E8). Portanto, se o JSON mostra `[w0,w1,w2,w3,w4,w5]`, os words em memória são `[w1,w0,w3,w2,w5,w4]`.

Consequências verificáveis:

- 36:049A retorna o word em memória +0, isto é, **w1**: o seletor/opcode.
- 36:047C retorna memória +2, isto é, **w0**: o tipo.
- 36:04F7 retorna memória +4, isto é, **w3**: o primeiro argumento de 16 bits.
- 36:0515 retorna memória DWORD +4, isto é, `(w2<<16)|w3`.
- O registro em 0x23AFF `[C000,0011,C006,0004,01A6,00E7]` realmente executa opcode 0011 com índice 4. Os dados altos C006 são irrelevantes à leitura word 36:04F7.

Essa correção se aplica também às antigas buscas globais de opcodes e à leitura de campos de registros por funções 36:0448/049A/04F7. Não confundir a ordem de WORDs do JSON com a ordem de memória usada pelo programa.

## Inicialização e entrada

2:07FF–0826 instancia a cena 2 com 12:0000, tamanho 013B. O construtor instala tabela 12:30AE; slot +0C aponta a 12:04A6. O slot +4 usa 35:3BD3, que chama +0C, ligando a criação de controles à ativação da cena.

12:0010 passa 013B ao alocador; não é ID de animação. A animação da cabine é criada em 12:0EE7–0EF1 com ID lógico **00010111** e guardada em +6E. A raiz MMS 0111 aponta ao bloco 0x23237. O controlador do mundo é +64 (construído via segmento14); o objeto de movimento é +68 (segmento15). 12:04B6 chama 12:12EB, que monta fundo e indicadores; 12:0BC4 chama 12:15C8, que anexa as áreas de comando.

### Teclado / comandos de controle

12:0C00 é slot +18 da tabela 12:30AE. A tabela 12:0D47 contém `31,32,33,52,72` (ASCII 1,2,3,R,r). As três primeiras entradas constroem 12:30FA com pares `(5,0)`, `(15,1)`, `(25,2)` e direção 0; R/r usa `(-5,3)`. O executor 12:3167 guarda o primeiro valor em movimento+5A quando +30 é zero, ou guarda +30 em movimento+60 quando diferente de zero. O argumento final não negativo atualiza o indicador por 13:15AE.

12:0D5B é slot +1C. Aceita eventos 0x65–0x68, não devem ser descritos automaticamente como teclas ASCII e–h: são códigos internos recebidos pelo método virtual. 0x65 aplica direção -1; 0x66 direção +1; 0x68 direção 0; 0x67, condicionado pelo global DS:008A, aplica par `(75,2)`. Todos usam o mesmo comando 12:30FA e execução 32:009A. A conversão de uma tecla física para esses eventos pertence à aplicação/base, fora deste trecho.

A área explícita criada em 12:0B5D–0BA0 tem words de retângulo `0122 0198 014E 01BB`, comando 12:361E e identificador de área 0065. O executor 12:366D chama 12:2EF7, que alterna a flag +EB quando +ED==0: inicia/pausa movimento, muda o indicador PIC 0148/014A e chama auxiliares 15:1194/1179. Isso comprova um controle de alternância de movimento; nome visual exige imagem correspondente.

## Todos os seletores da classe de comandos da cabine

O construtor 12:3328 instala 12:375B e guarda cabine em +26, seletor em +2A e argumento em +2C. Executor 12:3385 usa a tabela word 12:3454.

| Seletor | Ramo | Destino | Efeito confirmado |
|---:|---|---|---|
| 0 | 33A0 | 2662 | Para som anterior de +F3, solicita lógico 0001400E; executa animação alternada +77/+7B; alterna byte +72; interrompe movimento, +ED=1. |
| 1 | 33B0 | 2716 | Restaura flags mundo+14 e movimento+68; troca indicador para PIC014A; +EB=1,+ED=0. |
| 2 | 33C0 | 278F | Se animação estiver ociosa por 30:0F08, executa +7F[index], incrementa byte+F1 e volta a0 após4. |
| 3 | 33CF | 2A3A | Troca indicador014A; +EB=1; reativa flags; zera combustível acumulado em objeto+73/+0A e redesenha; +ED=0. |
| 4 | 33DE | 283C | Seleção pseudoaleatória0..2 por 2:0012; executa +8F[index]; interrompe movimento; +ED=1,+EB=0. |
| 5 | 33ED | 290D | Guarda argumento em +125; seleciona animação aleatória0..5 do vetor+9B; interrompe movimento; +ED=1,+EB=0. |
| 7 | 3403 | 29E6 | Usa +125 em 4:1929 e executa o comando de vídeo retornado por32:009A. |
| 6 | 3412 | 2E34 | Traduz +125 pela tabela43:0000,21 entradas de8 bytes; chama6:026C com o valor traduzido e contexto1. |
| 8 | 344F | nenhum | Entrada da tabela é diretamente epílogo: nenhum efeito. |
| 9 | 3421 | 27E5 | Se animação ociosa, executa vetor+ B3[byte+F2], incrementa circularmente em8. |
| 10 | 3430 | 2AD8 | Para movimento; +ED=1; executa sequência de animação de rótulo400 por30:0FBD. |
| 11 | 343F | 2B7E | Limpa acumulador do indicador de combustível e solicita oficina, cena0, argumentos FFFF e9. |

Fábricas: 12:2441->0;248F->1;24DD->2;252B->9;2579->4;25C7->6;2615(argumento)->5. Não confundir o seletor da classe com índice do vetor de callbacks da animação.

## Vetor de callbacks e retorno realmente presente

12:0EB2 configura o objeto de animação +6E:

| Entrada do vetor | Registro nativo | Comando |
|---:|---|---|
| 0 | 12:0F58 | seletor1 |
| 1 | 12:0FAC | seletor7 |
| 2 | 12:0FCC | seletor1 |
| 3 | 12:1020 | seletor3 |
| 4 | 12:1074 | seletor11 |

O interpretador 30:1D2E faz leitura do opcode; 0011 passa por30:1E70 e36:04F7, usa30:11C4 para buscar o vetor+27 e32:009A para executar. Assim **0x23AFF aciona a entrada4 configurada acima**, levando a12:343F->12:2B7E->2:19DE->2:0697(cena0). Esse é um retorno original demonstrado, não uma hipótese dependente de bloco01B4.

O callback está dentro da sequência de rótulo402 (registro de início em0x23A4B), com espera/salto0012 para rótulo1 no registro0x23B0B. O rótulo400 inicia em0x2364F e salta para401 em0x2371B;401 começa0x2395B. Em0x23A33 há0013 com argumento DWORD `(402<<16)|0`:30:2299 testa objeto de áudio0 e, quando inativo, reposiciona para402;0x23A3F volta a401. Isso confirma espera pelo fim do som antes do callback4. A análise anterior da quarta palavra errou justamente o argumento que resolve esta cadeia.

O comando10 que inicia400 é construído12:0629–0637 e passado ao medidor de combustível13:0E88 em12:06C1. O medidor calcula `percent=(acumulado+abs(delta))*100/total` em13:10BF–10EC, e nos limites50,100,109,120 executa campos+14,+18,+1C,+20. As ligações de12:067D–06C1 atribuem:50->rótulo2;100->rótulo10;109->rótulo11;**120->seletor10**. Assim a condição de120% do acumulador inicia400->401->402->callback4->oficina. O desenho e strings internas identificam o objeto como medidor de combustível (`Half a tank`, `Empty tank`, `Really empty`, `We just ran out of gas`).

## Vídeos escolhidos pelo mundo

14:29BC recebe um byte de símbolo, soma7F modulo256 e usa o resultado como índice de42 entradas. Para símbolos81–AA, índice0..41. Global de tema DS:02A0 escolhe tabela44:00A6 para0,44:01F6 para1,DS:09A8 para os demais. Cada entrada tem8 bytes: word rótulo de animação, word0, DWORD índice de filme. As42 entradas são seis repetições de sete filmes; rótulo aumenta de463 a468 a cada grupo de7.

| Global02A0 | Filmes nas colunas0..6, repetidos nas seis linhas |
|---|---|
| 0 | 14,6,52,47,38,39,1 |
| 1 | 10,28,50,53,37,33,30 |
| demais | 3,12,16,42,26,45,35 |

14:2A89 passa lowword do DWORD à fábrica12:2615 (seletor5),14:2AA8 combina rótulo e comando pelo construtor14:33FF. O seletor5 guarda ID de filme em+125; seletor7 chama4:1929 com esse ID em12:29F7–2A02. O wrapper de vídeo+129 é construído em12:082F–0862 com o comando6; a ligação completa do wrapper ao executor de vídeo consta da análise de mídia. Os nomes/offsets de58 filmes devem ser obtidos da tabela MOV; o número da área clicável não é índice de filme.

## Mundo, áudio e limites restantes

14:0563–057D seleciona raiz MMS24(global1),22(global0),23(demais). Lê listas19,1A,1B,1C em05AA,05D7,0604,0631;14:1092 percorre filhos, lendo identificador1D por36:0448 e ponteiros por36:06B6. Isso vincula a família de grades8009 à cabine, além da simples coincidência de segmento.

14:25FE–269F consulta uma tabela de oito descritores de símbolo em DS:083C; quando encontra, cria14:33FF com o comando0 da cabine (fábrica12:2441), que executa o som lógico0001400E. Símbolos57/58 possuem ramos próprios que também constroem comando0 com parâmetros por variante. Já14:2ABD trata AB..AE e associa rótulos1D5,1D6,1FD,1FE ao mesmo comando0. Assim há gatilhos de colisão/interação nos descritores do mundo; traduzir cada byte em obstáculo visual exige resolver a criação e a tabela de descritores e o teste espacial efetivo, não apenas ouvir o som.

O elo espacial entre o movimento, os aliases da grade e os descritores foi verificado na seção seguinte e nos dados driving/native-driving-tables.json.

## Fechamento espacial: movimento, aliases e execução de eventos

O elo antes pendente está resolvido por `15:0589 -> 14:1D89 -> 14:1DBD -> 13:0662 -> 14:37F2 -> 32:009A`. A grade não contém necessariamente o símbolo original MMS: `14:0C64-0CA4` registra uma cópia de célula em `mundo+52[alias]` e liga `celula+0B` ao descritor `mundo+852[simboloOriginal]`. Portanto conservar a associação alias/célula/descritor é necessário; procurar diretamente bytes 81-AA na grade transformada é incorreto. `14:2551` cria os 256 descritores possíveis por `25FE`.

Convenções: `H=DWORD DS:0768` é a altura construída, `W=DWORD DS:076C` é a largura (61 inicial); `G` é o buffer linear em `mundo+4E`. Uma célula é `G[linha*W+coluna]`; seu objeto é `mundo+52[byte]`. As coordenadas inteiras atuais são `mundo+22=coluna`, `+24=linha`. As coordenadas de precisão centesimal são `+1A=coluna100`, `+1E=linha100`; `+16/+18` guardam a fração decimal e `+22/+24` a divisão por100. DS:0768 começa zero e é somado durante a montagem em `14:21C4`, portanto não é uma dimensão fixa do executável.

### Passo do controlador

`15:01AA` é o laço. Só atualiza quando movimento+68 não é zero. O valor efetivo de velocidade é +62, alvo +60 (15 inicial); direção lateral +5A. Define `q=trunc(velocidade/5)`, `s=(-1 se velocidade<0, senão1)`, `n=abs(q)`; quando tempo desde +5C é menor que100ms, usa `n=trunc(n*elapsed/100)`. Se n=0 não desloca nem testa eventos; se q=0 ainda aproxima velocidade pelo método743. Quando n>0:

1. Soma `s*n` ao medidor de combustível (que usa módulo); soma `10*s*n` à linha100, com wrap H*100. Soma `10*direcao*n` à coluna100, com wrap W*100. A direção lateral independe do sinal longitudinal.
2. Atualiza coordenadas inteiras/fração, redesenha o mundo por slot virtual+10 e chama `14:1D89(mundo,q,direcao)`.
3. Consulta velocidade de terreno por `14:229D` e atualiza os acumuladores de reações; aproxima velocidade em exatamente1 do mínimo entre alvo e limite de terreno (`15:0743-077C`). Atualiza indicadores e timestamp.

O contador de amostragem DS:212A acumula no máximo300 passos. Após15 amostras, `mundo+12` recebe1 se média de elapsed é maior que200, senão0 (`15:0528-0574`): é uma adaptação de desenho, não condição de sucesso.

### Ordem precisa de colisão/interação

`14:1D89` só testa q positivo: q>=2 usa distância2, q==1 usa distância1, q<=0 retorna0. Assim marcha à ré não dispara esse teste. Seja `k=min(q,2)`, `r=wrap(linha+k,H)`, `c=wrap(coluna+k*direcao,W)`. O teste de `14:1DBD` examina, na ordem:

- Se fração lateral `mundo+16<=5` e direção==0: c-2.
- Se fração lateral >5 e direção==0: c+2.
- Sempre: c-1, c, c+1.

Cada coluna auxiliar recebe wrap W. O primeiro objeto cujo `+0B` não é nulo executa imediatamente seu descritor por `13:0662` e retorna1; os posteriores não são testados. Não há teste de distância de sprite ou transparência de pixels. O descritor chama o comando armazenado em+6 por `14:37F2`.

Depois de qualquer acerto, `14:20E6` soma k-1 à linha inteira e `(k-1)*direcao` à coluna inteira, com wrap. `15:0595-0602` marca mundo+10=1, põe movimento+6A=-5,+6E=0, acrescenta200 à linha100 e debita20 extras de combustível. Se linha100 ultrapassa H*100 aplica resto e atualiza fração da linha. Preservar essa ordem e os dois sistemas de coordenadas evita deslocamentos diferentes do original.

### Consulta de terreno e reações

`14:229D` percorre máscara DS:0820 de sete linhas, em ordem, a partir da posição inteira: pares `(largura,offsetColuna)` = `(1,0),(3,-1),(5,-2),(5,-2),(5,-2),(5,-2),(5,-2)`. Em cada linha consulta células da esquerda para a direita. Coluna inicial negativa torna-se W-1; linha >=H torna-se0. O laço interno incrementa coluna diretamente, sem wrap por célula. `13:068C` retorna apenas `descritor+0A`, sem executar comando. Se encontra valor diferente de zero retorna limite6. Se não encontra, retorna `celula(G[linha*W+coluna-1])+4`. Não substituir esta consulta por uma colisão: são funções diferentes.

No controlador, limite==5 acumula n em+6A e zera+6E; qualquer outro limite zera+6A e acumula n em+6E. Quando `25<+6A<200`, executa comando+72 e fixa +6A=205. Quando +6E>1000, executa comando+76 e zera+6E (`15:0621-06A7`). São gatilhos por distância efetiva, não timers.

## Classificação completa dos descritores por símbolo

`14:25FE` aplica a primeira regra correspondente. O arquivo `driving/native-driving-tables.json` conserva os valores binários little endian das tabelas nativas (não são registros MMS), incluindo todos os12 destinos de cena e42 destinos de filme por tema e as duas listas que escolhem classes de célula. `14:089B` procura primeiro37 bytes DS:07CA (classe13:09AB), depois40 bytes DS:07EF (classe13:09B7). `13:0000` inicializa o limite+4 em75; `13:0212` e os55 registros de3 bytes de44:0000 ajustam propriedades de símbolos durante a montagem.

| Símbolo original (hex) | Fábrica / comando |
|---|---|
| 4E,4F,50,51,52,53,59,5A | Rótulos488,489,495,496,507,508,517,519; comando cabine0. |
| 57 /58 | Arrays DS:087C/0882, indexados por mundo+DF8/+DFA; comando0. |
| AB-AE | 14:2ABD: rótulos469,470,509,510; comando0. |
| 81-AA | 14:29BC:42 entradas por tema, filme e rótulo já descritos; flag descritor+0A=1. |
| 01-0C | 14:28B4:12 entradas DS:0888/08E8/0948 por tema0/1/demais; comando2:1BDF recebe lowword do DWORD de destino; flag+0A=1. Não são índices MOV. |
| 47,48,4A,4B,4C,4D | 14:2B53: tabela DS:0AF8, comando cabine4; flag+0A=1. |
| 61 /62 /63 /64 | Fábricas2CFE/2DB3/2E66/2FA8; comando cabine0 e rótulos condicionados por tema e variante. |
| 28 /29 | Fábricas2BEC/2C75; tema0 usa rótulo453/454, tema1 usa449/450, tema2 indexa DS:0B28/0B30 por+DFC; comando0. |
| Demais | Descritor nulo: este caminho não executa interação. |

As variantes são sorteadas uma vez em `14:23FB`, antes da criação dos descritores, não a cada colisão: DF8/DFA usam intervalo0..2; DFC começa0 e na tema2 usa0..3. DF0/DF2/DF4/DF6 têm intervalos específicos por tema. As fábricas63/64 ainda constroem comandos compostos de18 bytes: não achatar suas listas a um único comando.

**Escopo após fechamento:** estão demonstrados o teste espacial, a ordem dos eventos, o despacho de todos os símbolos e o mecanismo de escolha dos filmes/cenas. Para equivalência integral de desenho/montagem ainda se deve conservar o algoritmo de aliases e as listas animadas de63/64; o presente texto não reivindica que a reconstrução executável existente já os implemente. Não houve execução original nem compilação nesta análise.

Nota de nomenclatura: DS:02A0 é o seletor de tema0/1/2, gravado por3:2D70 a partir dos cartazes da cena3. Não confundir com uma dificuldade independente. Os nomes `level0/level1` no JSON auxiliar são chaves históricas e representam o valor bruto desse tema.

### Variantes e descritores animados63/64

A classe `14:3865` não é uma sequência de comandos. Guarda contagem em+0C e cópia dos WORDs de imagem em+0E. Seu método de desenho `14:39A6` escolhe `imagens[DS:212A % contagem]`, grava em+4 e chama o desenho34BE. Seu acionamento mantém o único comando0 herdado. O índice usa o contador de passos, não sorteio por colisão.

| Símbolo | Tema2 | Tema0 | Tema1/demais |
|---|---|---|---|
|61, índice DF0 | DS:0B38,2 imagens | DS:0B3C,2 | DS:0B40,2 |
|62, índice DF2 | DS:0B44,1 | DS:0B46,2 | DS:0B4A,2 |
|63, índice DF4 | DS:0B4E,3 | DS:0B54,4 | DS:0B5C,3; se DF4==2, animado com3 imagens DS:0B62 |
|64, índice DF6 | DS:0B68,2 | DS:0B6C,3 | DS:0B72,1; se DF6==0, animado com20 imagens DS:0B74 |

`14:23FB` sorteia DF0/DF2/DF4/DF6 nos intervalos inclusivos: tema1 =0..1,0..1,0..2,0; tema2 =0..1,0,0..2,0..1; tema0/demais =0..1,0..1,0..3,0..2. Todos os WORDs dessas listas constam no JSON auxiliar. Isso fecha as seleções variáveis sem inventar comandos adicionais.

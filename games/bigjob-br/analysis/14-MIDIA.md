# Mídia: seleção nativa de filmes e sons

Análise exclusivamente estática de BIGJOBNG.EXE e dos extents da ISO brasileira. Os endereços `segmento:offset` usam segmento decimal e offset hexadecimal. Índices de filme e som são baseados em zero. O resultado fecha a associação numérica dos **58 filmes** aos quatro grupos de seleção abaixo; nomes descritivos de conteúdo e a ligação de cada contexto do objeto comum a uma atividade continuam separados da prova numérica.

## Bancos efetivamente carregados

O carregamento dos bancos não usa os descritores MMS E2/E3. A tabela da aplicação 2:190A tem slot +48 em 2:1952 ->31:0F43. Esse método chama USER ordinal176 (LoadString), separa a string por vírgulas e devolve o item solicitado. Assim 31:0D17 solicita RT_STRING 02BF (703), e 31:0EE6 solicita 02C0 (704). A leitura dos recursos NE fornece:

| Recurso | Valor exato | Consumidor |
|---|---|---|
| 701 | BIGJOB.MMS | Script |
| 702 | BIGJOBCD.PIC,BIGJOBHD.PIC | Bancos de imagens |
| 703 | BIGJOB.SND,BIGVOX.SND | 31:0CC3 ->36:1BB1; registro virtual +20 em31:0D05 |
| 704 | BIGJOB.MOV | 31:0E92 ->33:25DE; registro virtual +10 em31:0ED4 ->33:249F |

Os cabeçalhos de som em +100 têm palavra 0000 no BIGJOB.SND e 4000 no BIGVOX.SND. Em36:19AD, `banco = id >>14`, com tabela de quatro ponteiros em DS:1216. Em36:1735 o índice é `id &3FFF`. A combinação inversa é explícita em36:19FB-1A01. O registro em36:1A1A usa o slot +28 do provedor e o grava nessa tabela. Portanto banco0 é BIGJOB.SND e banco1 é BIGVOX.SND, confirmados pelo cabeçalho, não apenas pela ordem da lista.

**Correção do relatório11:** em `0001400E`, o banco1 vem dos bits14-15 da palavra baixa400E. A palavra alta0001 não identifica banco. 36:28F7 passa somente a palavra baixa ao construtor36:1710; os demais argumentos seguem separadamente ao executor36:2937.

## Como o ordinal vira um filme concreto

33:22E0 constrói o identificador de filme: mascara a palavra baixa com3FFF e resolve o provedor por33:2588, que usa os dois bits superiores para indexar DS:119E.33:23FD chama o método +0C do provedor. Para o objeto QuickTime, esse slot chega a33:2A12. Ali o índice é multiplicado por4, o DWORD da tabela +10 é passado como posição, e1:07DE abre o filme a partir dessa posição no arquivo do provedor.

O cabeçalho original de BIGJOB.MOV tem um DWORD big-endian58 seguido de58 DWORDs big-endian. **Todos os58 valores são exatamente os offsets `moov`**, na ordem de `mapa-jogo-v3.json.movies`; não são os offsets `mdat`. O primeiro é00208800 (2131968), o último15495573 (357127539). A verificação comparou as58 entradas uma a uma. Os58 descritores têm vídeo `cvid` e áudio `raw `.

4:1710 recebe retângulo, handle da janela e objetos associados. Ele copia oito bytes para +0E, grava o handle em+16 e zera o player+0A. **Esses oito bytes são geometria, não o ID do filme**:33:2D8F-2DAE usa-os como posição e dimensões ao criar a janela. O ID é fornecido depois a4:1929 ->4:13F6; o comando guarda o wrapper em+26 e o ID em+2A. Seu slot+8 (tabela4:24E3, fixup24EB) é4:14A1, que chama4:19FE.4:1AEB constrói a referência de filme pelo ID com33:22E0;4:1AFB chama33:2FBA. O player busca o movie via33:23FD em2FEB, instala o controlador e inicia a reprodução. O callback de término é configurado em4:1A79 ->33:2F11.

## Quatro grupos de seleção: cobertura58/58

### Cabine, cena2:21 filmes

O wrapper é criado em12:0862 e guardado em+129.12:29F7 lê o ID em+125 e12:2A02 cria o comando4:1929. O ID foi gravado em12:291C pelo comando da cabine seletor5, construído em12:2615.14:2A97 passa o DWORD+4 de uma tabela dependente da tema (global 02A0) a essa fábrica. A seleção de coluna vem do símbolo da estrada; a seleção de linha escolhe seis rótulos de animação, mas repete os mesmos sete filmes. A verificação detalhada desse fluxo está no relatório da cabine.

| Global02A0 | Tabela | Colunas0,1,2,3,4,5,6 |
|---|---|---|
| 0 |44:00A6 |14,6,52,47,38,39,1 |
| 1 |44:01F6 |10,28,50,53,37,33,30 |
| outro |49:09A8 |3,12,16,42,26,45,35 |

### Cena4: três filmes

9:100D cria4:1710, guardado no objeto+114.9:19DD-19F3 exige que+6E,+6C,+6A estejam zerados.9:19F5-1A0F escolhe20 para global02A0=0,18 para1,8 nos demais casos.9:1A19 chama4:1929;9:1A2B executa o comando por32:009A. O construtor de cena4 é9:0060; a cena23 usa outro construtor9:22C4 e não deve herdar esta associação apenas por estar no mesmo segmento.

### Jukebox, cena12: quatro filmes

26:344E cria a classe4:1C15 e guarda em+76; ela deriva do wrapper4:1710. As chamadas26:34AC,355A,3608,36BD constroem comandos por4:22A9 com IDs2,9,19,22. Os comandos passam ao construtor de controles33:10F8, usando pares de imagens4DBA/4DBB,4DBC/4DBD,4DBE/4DBF,4DC0/4DC1.4:22A9 ->4:21E0 guarda ID+2A e wrapper+26; o executor4:228B chama4:1DC3. Este constrói4:22F9, que guarda o ID em+46.4:2353 prepara a interface, espera37:1971 retornar sucesso e em4:239D chama4:1929 com o ID+46. Essa segunda etapa demonstra que2/9/19/22 são filmes, não seletores de cena. O construtor26:2FFA é o destino da cena12.

### Objeto comum do segmento3:30 filmes

3:0074 guarda o argumento BP+0E em+6E.3:0970-098A lê esse contexto, chama3:12DD e fornece o retorno a4:1929.3:12DD é exatamente `DS:0204[contexto]` em WORDs. Os36 valores da região0204..024B são:

`24,36,44,17,49,FFFF,25,48,46,56,15,7,55,27,FFFF,57,0,11,21,5,29,31,FFFF,41,4,34,FFFF,51,43,FFFF,40,32,23,54,FFFF,13`.

Existem30 IDs válidos e seis sentinelasFFFF. Esses30 IDs, os21 da cabine, três da cena4 e quatro do jukebox são disjuntos e cobrem exatamente0..57. Isso fecha a cobertura do banco, mas a relação contexto->atividade requer seguir os chamadores do objeto comum. Não se atribui um nome de atividade por semelhança visual ou prefixo de áudio.

## Índice verificável dos58 filmes

Os offsets são relativos ao início de BIGJOB.MOV. O grupo é o consumidor numérico demonstrado acima; não pretende ser um título editorial do vídeo.

| ID | mdat | moov / entrada do cabeçalho | Fim | Seleção |
|---:|---:|---:|---:|---|
|0|0xEC|0x208800|0x20933C|Objeto comum seg.3, contexto 16|
|1|0x20933C|0x9C99EC|0x9CB003|Cena 2, tema (global 02A0) 0, coluna 6|
|2|0x9CB003|0x1E29DDD|0x1E2E5B1|Cena 12, botão 0|
|3|0x1E2E5B1|0x2646B89|0x2648224|Cena 2, tema (global 02A0) outra, coluna 0|
|4|0x2648224|0x2883F77|0x2884B5B|Objeto comum seg.3, contexto 24|
|5|0x2884B5B|0x2ACFF52|0x2AD0B5A|Objeto comum seg.3, contexto 19|
|6|0x2AD0B5A|0x327FD76|0x32813F9|Cena 2, tema (global 02A0) 0, coluna 1|
|7|0x32813F9|0x3451BC0|0x3452678|Objeto comum seg.3, contexto 11|
|8|0x3452678|0x45146F5|0x45182E5|Cena 4, tema (global 02A0) outra|
|9|0x45182E5|0x596A359|0x596EB4D|Cena 12, botão 1|
|10|0x596EB4D|0x5FFC5C3|0x5FFDA36|Cena 2, tema (global 02A0) 1, coluna 0|
|11|0x5FFDA36|0x61FED71|0x61FF8F5|Objeto comum seg.3, contexto 17|
|12|0x61FF8F5|0x68FB09A|0x68FC5D5|Cena 2, tema (global 02A0) outra, coluna 1|
|13|0x68FC5D5|0x6AC3C68|0x6AC474C|Objeto comum seg.3, contexto 35|
|14|0x6AC474C|0x72BD28B|0x72BE9B6|Cena 2, tema (global 02A0) 0, coluna 0|
|15|0x72BE9B6|0x74E7EA5|0x74E8AA9|Objeto comum seg.3, contexto 10|
|16|0x74E8AA9|0x7BD3746|0x7BD4BDD|Cena 2, tema (global 02A0) outra, coluna 2|
|17|0x7BD4BDD|0x7E04B11|0x7E0574D|Objeto comum seg.3, contexto 3|
|18|0x7E0574D|0x9176D44|0x917B174|Cena 4, tema (global 02A0) 1|
|19|0x917B174|0xA4E5B59|0xA4E9EE5|Cena 12, botão 2|
|20|0xA4E9EE5|0xB547B74|0xB54B6BC|Cena 4, tema (global 02A0) 0|
|21|0xB54B6BC|0xB73B0FF|0xB73BC33|Objeto comum seg.3, contexto 18|
|22|0xB73BC33|0xCBF42F2|0xCBF8C56|Cena 12, botão 3|
|23|0xCBF8C56|0xCE17D93|0xCE1894F|Objeto comum seg.3, contexto 32|
|24|0xCE1894F|0xCFF3D2A|0xCFF485A|Objeto comum seg.3, contexto 0|
|25|0xCFF485A|0xD198171|0xD198BC9|Objeto comum seg.3, contexto 6|
|26|0xD198BC9|0xD82D480|0xD82E86F|Cena 2, tema (global 02A0) outra, coluna 4|
|27|0xD82E86F|0xDA34BE1|0xDA3578D|Objeto comum seg.3, contexto 13|
|28|0xDA3578D|0xE05D9F7|0xE05ED5A|Cena 2, tema (global 02A0) 1, coluna 1|
|29|0xE05ED5A|0xE24CDF6|0xE24D98E|Objeto comum seg.3, contexto 20|
|30|0xE24D98E|0xE8CCE7C|0xE8CE2CB|Cena 2, tema (global 02A0) 1, coluna 6|
|31|0xE8CE2CB|0xEACCAD7|0xEACD6AB|Objeto comum seg.3, contexto 21|
|32|0xEACD6AB|0xECDDB99|0xECDE72D|Objeto comum seg.3, contexto 31|
|33|0xECDE72D|0xF4BB700|0xF4BCDF3|Cena 2, tema (global 02A0) 1, coluna 5|
|34|0xF4BCDF3|0xF67039D|0xF670E09|Objeto comum seg.3, contexto 25|
|35|0xF670E09|0xFCB9C23|0xFCBAF32|Cena 2, tema (global 02A0) outra, coluna 6|
|36|0xFCBAF32|0xFE716CF|0xFE720FB|Objeto comum seg.3, contexto 1|
|37|0xFE720FB|0x1050DF5F|0x1050F392|Cena 2, tema (global 02A0) 1, coluna 4|
|38|0x1050F392|0x10C99792|0x10C9ADD9|Cena 2, tema (global 02A0) 0, coluna 4|
|39|0x10C9ADD9|0x11305858|0x11306C13|Cena 2, tema (global 02A0) 0, coluna 5|
|40|0x11306C13|0x114A1C3A|0x114A2676|Objeto comum seg.3, contexto 30|
|41|0x114A2676|0x116D5246|0x116D5E32|Objeto comum seg.3, contexto 23|
|42|0x116D5E32|0x11DC0BAB|0x11DC20DE|Cena 2, tema (global 02A0) outra, coluna 3|
|43|0x11DC20DE|0x11FF609E|0x11FF6C86|Objeto comum seg.3, contexto 28|
|44|0x11FF6C86|0x1219F23C|0x1219FC58|Objeto comum seg.3, contexto 2|
|45|0x1219FC58|0x128B0CCD|0x128B21F8|Cena 2, tema (global 02A0) outra, coluna 5|
|46|0x128B21F8|0x12A8BB90|0x12A8C640|Objeto comum seg.3, contexto 8|
|47|0x12A8C640|0x13201FB2|0x132035D5|Cena 2, tema (global 02A0) 0, coluna 3|
|48|0x132035D5|0x13421819|0x1342238D|Objeto comum seg.3, contexto 7|
|49|0x1342238D|0x135C645B|0x135C6EA3|Objeto comum seg.3, contexto 4|
|50|0x135C6EA3|0x13BC983F|0x13BCAC52|Cena 2, tema (global 02A0) 1, coluna 2|
|51|0x13BCAC52|0x13E0948F|0x13E0A117|Objeto comum seg.3, contexto 27|
|52|0x13E0A117|0x145760E1|0x1457774C|Cena 2, tema (global 02A0) 0, coluna 2|
|53|0x1457774C|0x14D3E467|0x14D3FC32|Cena 2, tema (global 02A0) 1, coluna 3|
|54|0x14D3FC32|0x14EFD4FF|0x14EFDF4B|Objeto comum seg.3, contexto 33|
|55|0x14EFDF4B|0x150EB554|0x150EC07C|Objeto comum seg.3, contexto 12|
|56|0x150EC07C|0x152AA51B|0x152AAFE7|Objeto comum seg.3, contexto 9|
|57|0x152AAFE7|0x15495573|0x1549608B|Objeto comum seg.3, contexto 15|

## Premio: contexto de progresso e evento que inicia o filme

O contexto +6E do objeto comum e o ID de progresso recebido pela gerencia sao a mesma palavra: 6:04E6 empilha BP+0A em 6:0515 e chama 3:0003 em 6:0520; o construtor copia BP+0E para +6E em 3:0071-0074. O argumento de tempo e 0x2710 (10000 ms), empilhado em 6:0512. Portanto os contextos da tabela 49:0204 podem ser ligados aos construtores de atividade cuja palavra +64 foi demonstrada, sem inferir nomes a partir do video.

| Cena | Chamada de 5:14FF | ID de progresso +64 | Filme de premio |
|---:|---|---:|---:|
| 7 | 6:1AF4; DWORD 001F0007 | 31 | 32 |
| 13 | 25:33B9; DWORD 000C000D | 12 | 55 |
| 15 | 21:0116; DWORD 0020000F | 32 | 23 |
| 16 | 18:071B; DWORD 00130010 | 19 | 5 |
| 17 | 19:0AC6; DWORD 00120011 | 18 | 21 |
| 18 | 27:252F; DWORD 000F0012 | 15 | 57 |
| 19 | 29:0585; DWORD 00040013 | 4 | 49 |
| 20 | 16:08BE; DWORD 00060014 | 6 | 25 |
| 21 | 17:002D; DWORD 00170015 | 23 | 41 |

Os IDs acima sao de progresso, nao seletores de cena. O modulo VM/cena 5 chama 5:0059 em 27:0029, em vez de 5:14FF; nao recebe automaticamente uma linha desta tabela. Os outros21 contextos foram fechados pela traducao filme/progresso da cabine em43:0000: ver [27-RECOMPENSAS.md](27-RECOMPENSAS.md). A uniao21+9 coincide com todas as30 entradas validas da tabela.

A reproducao nao decorre apenas da criacao do premio. O receptor 3:079E subtrai 1 do evento em 3:07AB e indexa a tabela WORD em 3:0B27: eventos 1..4 vao a 07BA,0802,086B,0A41. **Evento 3** entra em 3:086B; apos os testes de contexto/estado, 3:0964 exige wrapper +AA nao nulo; 3:0970-097A passa +6E a 3:12DD, e 3:098A cria o comando 4:1929. 3:099A executa o comando por 32:009A. O controle criado na montagem do premio recebe comando 2:1CAD com seletor 3 em 3:05D5-05E0. Esta cadeia identifica um disparador concreto do filme; nao significa que todos os premios iniciem video automaticamente ao aparecer.

## Catalogo reproduzivel de midia e alcance dos sons

`python tools/catalogue_media.py` gera `analysis/media-catalogue.json` usando o mapa original, o executavel apenas como bytes e o fluxo MMS corrigido. Verifica a tabela de 36 WORDs diretamente no segmento de dados 49, exige 58 filmes e exatamente um grupo de selecao para cada ID. O arquivo contem:

- 58 filmes, offsets e seletores numericos; 36 contextos de premio, incluindo seis FFFF.
- 962 entradas dos dois bancos de som, com ID empacotado, banco, indice, nome, offset e tamanho.
- 441 disparos de som nos 122 scripts raiz, com raiz, bloco, registro, offset, palavras em disco/runtime e ultimo label textual anterior. Esse ultimo label nao e uma prova de alcance: saltos podem entrar no meio do bloco.
- 85 chamadas nativas a 36:28F7/2937, com segmento, site e janela literal dos argumentos. Argumentos dinamicos e valores formados por varios pushes permanecem visiveis, sem serem convertidos em IDs presumidos.
- Nove pares cena/progresso e seus filmes comprovados pela tabela acima.

O caminho de audio MMS esta demonstrado no relatorio 16: opcode runtime 0010 resolve W3 por 36:05C3 ->36:1710; 30:224B entrega o objeto ao executor 36:2937. O catalogo liga cada disparo ao recurso concreto, mas **nao demonstra que todo disparo seja alcancavel em cada atividade**. A associacao completa evento/UI -> estado/label -> som ainda requer seguir o owner e os saltos de cada controlador. Em particular, a presenca de audio num segmento compartilhado nao autoriza atribui-lo a todas as cenas desse segmento. Esta limitacao e registrada tambem no JSON.

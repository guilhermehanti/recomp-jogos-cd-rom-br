# Big Job BR — mapa estático do jogo inteiro

O arquivo [mapa-jogo-v3.json](mapa-jogo-v3.json) reúne o índice da mídia, do MMS e dos executáveis Win16 da edição brasileira. O gerador `python tools/map_game.py analysis/novo-mapa.json` abre a ISO em leitura, verifica os hashes dos executáveis e cria um relatório novo. A validação conferiu contagens, limites, os IDs da cortina e a cobertura do arquivo de script.

| Subsistema | Evidência catalogada | Trabalho para preservar o comportamento |
|---|---|---|
| Executável NG | 49 segmentos, 13.487 registros de fixup, 8.156 locais candidatos a chamada far | Resolver chamadas indiretas, tabelas virtuais, callbacks e contratos Win16 a partir dos destinos relocados. Verificar regras de memória segmentada, x87 e o laço de mensagens. |
| Executável com WinG | 49 segmentos, 13.496 fixups, 8.170 candidatos | Usar como referência comparativa. O NG elimina o import estático de WinG. |
| Script | 206 blocos estruturados, 53.509 registros e 205 referências para blocos com cabeçalhos válidos | Identificar o significado dos comandos por classe. Associar cada atividade a seu ponto de entrada, eventos, transições, retorno e chamadas nativas. |
| Tabelas MMS com dados de comprimento variável | 11 regiões, 86 blocos, 2.497 registros, 2.495 strings e 107 referências indexadas | Associar as tabelas aos consumidores e interpretar as regras codificadas nos símbolos. |
| Imagens | 4.119 entradas no PIC de CD e 625 no PIC de HD, com nome, intervalo e cabeçalho de cada entrada | Validar imagens e máscaras por tipo, paleta e camada. Cruzar IDs com chamadas de desenho e estado. |
| Áudio | 33 entradas em BIGJOB.SND e 929 em BIGVOX.SND | Resolver banco lógico por evento, taxa nominal, loop, prioridades e interrupção. Confirmar vozes e efeitos por cena. |
| Filmes | 58 índices, offsets e descritores Cinepak | Ligar IDs às cenas e reproduzir áudio, vídeo, recorte, eventos e encerramento da reprodução. |
| Dados espaciais | Dez tabelas DAT de 93 registros de 140 bytes e uma tabela final de 93 palavras | Reconstruir a interpretação dos campos pelos consumidores em 14:1587 em diante. |
| Persistência | Referências a BIGJOB.SAV e BIGJOB.INI no segmento 6 | Mapear campos, versão, valores padrão e vínculo com o progresso. |

## Como usar o mapa

- Os endereços das chamadas no JSON são relativos ao segmento. Os offsets MMS, PIC, SND e MOV são relativos ao arquivo de origem. Candidatos a chamada far exigem confirmar o limite da instrução no fluxo real. A lista de fixups completa permanece disponível para refazer essa análise.
- No MMS, `C002:000C` aparece 11.584 vezes, `C002:000D` 21.887 vezes, `C006:000E` 14.065 vezes e `C005:0010` 441 vezes nas regiões estruturadas. As contagens incluem todos os blocos alcançados e servem para priorizar o interpretador compartilhado. Os papéis desses comandos foram inicialmente investigados no fluxo da cortina, descrito no [rastreamento do motor](09-RASTREAMENTO-MOTOR.md).
- Os prefixos do banco permitem localizar famílias de recursos em escala. No PIC de HD há 258 entradas DS, 203 CH e 89 JY. No PIC de CD há famílias como CH (905), CP (512), VM (459), SB (425), CT (192), CS (170), DD (153), TC (148), CG (133), FM (133), FL (120) e HW (110). Esses prefixos agrupam recursos. A quantidade de famílias não determina a quantidade de atividades jogáveis.

## Critério para considerar o rastreamento completo

Para cada atividade, demonstrar a cadeia entre o evento de entrada, os manipuladores de mouse e teclado, as regras e estados nativos, os IDs de recursos, os gatilhos de áudio e vídeo, a conclusão e o retorno à oficina. Para o jogo inteiro, validar também a abertura e o salvamento. O índice estrutural cobre os bancos e os dois executáveis do título e orienta essa reconstrução. O mapa comportamental completo permanece em aberto.

## Decodificação das 11 regiões

Cada região é uma sequência de blocos. Cada bloco tem tamanho e contagem BE16, uma tabela de registros de 12 bytes e uma área final de dados. Para o tipo `C003`, a quarta palavra localiza uma string terminada por zero em relação ao início dessa área. O código em 36:0C37 encaminha o tipo `C003` à rotina em 36:0ED9. A rotina procura o terminador, aloca a string e guarda seu ponteiro no registro em memória. As duas exceções são registros `C001` nos descritores de filmes. Eles seguem o formato de registro fixo.

| Intervalo relativo ao MMS | Tipo das referências | Blocos | Conteúdo observado |
|---|---|---:|---|
| 0x133E–0x92E6 | 8009 | 11 | Linhas de 61 caracteres, mais terminador |
| 0x9302–0xCA8A | 8009 | 2 | Mesmo layout |
| 0xCAA6–0x11DEE | 8009 | 2 | Mesmo layout |
| 0x11E0A–0x17437 | 8009 | 2 | Mesmo layout |
| 0x174F3–0x1F577 | 8009 | 11 | Mesmo layout |
| 0x1F7AF–0x1F83D | 800E | 1 | Seis linhas de dez símbolos, incluindo H, F, X, 1 e 2 |
| 0x1F8BD–0x2090D | 8010 | 9 | Vinte linhas de dez símbolos por bloco, com os seletores DD e DE alternados |
| 0x20971–0x217F1 | 8010 | 8 | Mesmo layout |
| 0x21855–0x22723 | 8010 e 8008 | 10 | Oito grades e dois descritores com `cinepak.mov` e `indeo.mov` |
| 0x22787–0x23237 | 8012 | 8 | Treze linhas de treze símbolos por bloco, incluindo números e separadores |
| 0x2D3D7–0x2EB63 | 8016 | 22 | Dez linhas de catorze símbolos por bloco, incluindo w, o e pontos |

Com os 206 blocos previamente indexados, as 86 tabelas formam 292 intervalos contíguos a partir do byte 18 e até o fim de BIGJOB.MMS. A verificação releu a ISO e comparou cada palavra dos 2.497 registros e cada string com o mapa. A cobertura de bytes estabelece a estrutura das regiões. Os significados das letras nas grades e os eventos que escolhem cada grade exigem identificar os leitores específicos.

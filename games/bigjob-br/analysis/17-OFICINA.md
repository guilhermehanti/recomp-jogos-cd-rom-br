# Oficina — eventos e comandos registrados

Os retângulos abaixo não abrangem todos os controles filhos nativos. O receptor de mouse da bola, sua física, cesta, sons e série de acertos estão em [32-OFICINA-BASQUETE.md](32-OFICINA-BASQUETE.md). Ele opera dentro da cena0 e não é uma troca de cena.

A cena 0 é construída por 8:010D e preenchida por 8:024D. Os endereços nesta página vêm de BIGJOBNG.EXE e das cadeias de relocação NE expandidas em analysis/disassembly/references.json. A enumeração abaixo complementa as oito áreas já descritas no relatório 11.

## Quatro áreas adicionais

| Criação do comando | Criação da área | Retângulo, quatro palavras na ordem original | Animação controlada |
|---|---|---|---|
| 8:0F86 → 2:2772 | 8:100E → 32:3735; anexada em 8:102C | 0, 330, 119, 463 | MMS 0141, bloco 0x671D7 |
| 8:1068 → 2:2772 | 8:10F0 → 32:3735; anexada em 8:110E | 120, 409, 132, 453 | MMS 0141, bloco 0x671D7 |
| 8:114A → 2:2772 | 8:11D2 → 32:3735; anexada em 8:11F0 | 394, 361, 524, 478 | MMS 014F, bloco 0x7FD87 |
| 8:122C → 2:2772 | 8:12B4 → 32:3735; anexada em 8:12D2 | 496, 339, 620, 417 | MMS 014F, bloco 0x7FD87 |

Os dois objetos de animação são criados em 8:0EBB e 8:0E86 por 5:0CF8. Seus ponteiros são guardados em BP-0134 e BP-0130, respectivamente, e reutilizados nas quatro chamadas da tabela.

O argumento empilhado 00030002 é formado por duas palavras. 2:2772 guarda 0002 em +2A e 0003 em +2C do comando. Sua execução 2:27CF passa ambas para 2:0012, que calcula um valor aleatório inclusivo entre elas, usando o resto de uma divisão pelo intervalo mais um. 2:27F7 cria o comando de salto 30:0FBD com o resultado e o ponteiro da animação. 2:2807 executa-o por 32:009A. As quatro áreas portanto solicitam os rótulos 2 ou 3 da respectiva animação. A chamada não contém um destino de cena.

## Encaminhamento comum de clique

32:3735 cria a ação 33:1903. A tabela da ação usa o receptor 33:1956. Esse receptor testa o bit 2 do evento, testa o ponto recebido no slot +24 da área e, em caso de acerto, chama o slot +14 da ação. O comando armazenado chega a 32:009A, que executa o slot +8 imediatamente quando o estado global em 1236 e a palavra +24 do comando estão zerados. Os demais estados encaminham a operação pelo caminho de cópia e fila.

A cadeia distingue os quatro comandos de animação dos comandos 2:1AC3 que solicitam troca de cena. O valor alto do argumento da área não basta para identificar um som ou filme. A identidade da mídia exige seguir o script selecionado e o banco efetivamente registrado.


## Oito áreas de navegação

| Objeto / descritor | Construção da área | Argumento bruto da área | Comando ligado à área | Destino |
|---|---|---|---|---|
| Cabine, seletor MMS 0066 (X 233–378, Y 122–290) | 8:0D47 → 5:2F8D → 8:0D62 → 32:3735 | 000B:006E | 8:0D35 cria comando sobre animação 013B. 8:0CDE instala na entrada 0 o comando de transição construído em 8:03CF–03E0. | 2, direção |
| Faixa esquerda, seletor 0067 (X 0–23, Y 77–320) | 8:04BA → 5:2F8D → 8:04D3 → 32:3735 | 0009:001A | 8:045C–046D cria 2:1AC3. 8:04AF passa seu ponteiro ao objeto da área. | 3 |
| Painel à esquerda, seletor 0068 (X 31–128, Y 54–208) | 8:05C9 → 5:2F8D → 8:05E4 → 32:3735 | 0009:0001 | 8:0564–0575 cria 2:1AC3. 8:05BD transfere o comando. | 4 |
| Área superior direita, seletor 0069 (X 433–497, Y 96–163) | 8:0683 → 5:2F8D → 8:069E → 32:3735 | 000B:0001 | 8:0635 instancia 2:1BDF, classe cujo método 2:1C7C grava contexto e solicita o seletor 14. | 14, resolução por cache |
| Jukebox, seletor 006C (X 429–507, Y 178–323) | 8:0796 → 5:2F8D → 8:07B1 → 32:3735 | 000B:02C6 | 8:0731–0742 cria 2:1AC3. 8:078A transfere o comando. | 12 |
| Retângulo explícito (302, 73, 352, 130) | 8:08A1–08B3, 8:08E0 → 32:3735 | 000B:01F4 | 8:0844–0855 cria 2:1AC3. 8:089D transfere o comando. | 23 |
| Retângulo explícito (236, 392, 388, 433) | 8:0A9B–0AAD, 8:0ADA → 32:3735 | 0008:0546 | 8:0A91 cria comando sobre 013D. 8:0A31 instala na entrada 0 a transição construída em 8:0342–0353. | 1, segmento 23 e candidato das grades 8010 |
| Retângulo explícito (558, 50, 600, 360) | 8:0BF9–0C0B, 8:0C38 → 32:3735 | 000A:04B0 | 8:0BEF cria comando sobre 013C. 8:0B8F instala na entrada 0 a transição construída em 8:0973–0984. | 6 |

Os argumentos brutos da área não identificam diretamente mídia. Os comandos e os registros MMS determinam as ações. O seletor14 é a tela de dificuldade para a cena5, conforme18-NAVEGACAO.md.

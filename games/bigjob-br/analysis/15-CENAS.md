# Cenas auxiliares e atividades: identidade por caminho virtual

Esta página registra construtores e tabelas virtuais. As cadeias de comportamento e as correções posteriores estão consolidadas em11-RASTREAMENTO-ATIVIDADES.md e detalhadas nos relatórios12–24.

Análise estática de BIGJOBNG.EXE BR; segmentos em decimal e offsets em hexadecimal. Nenhum executável original foi executado e nenhuma compilação foi realizada. A identidade abaixo deriva do construtor despachado por 2:0697, da tabela virtual instalada e do slot de construção de conteúdo +0C. Nomes de atividade são descrições dos recursos observados, não traduções oficiais.

## Identidade e conteúdo de cada seletor

| Cena | Construtor | Tabela virtual | Construção +0C | Conteúdo comprovado |
|---:|---|---|---|---|
| 3 — Seleção de cenários por cartazes (CS) | 11:0113 | 11:0C46 | 11:0278 | 11:028F → 5:0217, 47F7 = CSbackground |
| 4 — Painel CP | 9:0060 | 9:222F | 9:01E7 | 9:0206, 467E = CPbackground |
| 5 — Módulo VM | 27:0000 | 27:1BC4 | 27:047E | 27:04D1, 4E92 = VMbackground |
| 6 — Criação/impressão CT | 11:0C92 | 11:1A46 | 11:0E78 | 11:0EBB, 486C = CTbackground |
| 8 — Tela de imagem parametrizada | 5:3048 | 5:335A | 5:30E1 | 5:3104, índice vindo de +64 |
| 9 — Tela LG | 20:0A16 | 20:10BA | 20:0AEE | 20:0B77, 020D = LGbackground no HD |
| 10 — Módulo TC | 25:0000 | 25:3340 | 25:02DD | 25:0362, 4D59 = TCbackground |
| 11 — Tela JY | 20:114A | 20:13AB | 20:11CE | 20:1210, 020B = JYbackground no HD |
| 12 — Jukebox VJ | 26:2FFA | 26:3811 | 26:325B | 26:329E, 4DB8 = VJbackground |
| 13 — Módulo DD | 25:338C | 25:3870 | 25:33E2 | 25:3424, 48D4 = DDbackground |
| 15 — Guindaste/vigas SG | 21:00E7 | 21:276B | 21:0564 | 21:1389, 4CEE = SGempty; fundos variáveis |
| 16 — Mecanismo de trigo GW | 18:06E9 | 18:3E06 | 18:0B2E | 18:0BA3, 4A8B = GWbackground |
| 17 — Mangueira/incêndio HW | 19:0A94 | 19:252E | 19:0C4A | 19:0C8C, 4ABB = HWbackground |
| 18 — Bola de demolição WB | 27:2502 | 27:3C34 | 27:25E0 | 27:2623, 4F99 = WBbackground |
| 19 — Canhão de água WC | 29:0558 | 29:18E9 | 29:0616 | 29:0643, 4FDE = WCbackground |
| 20 — Módulo FL | 16:088F | 16:2A86 | 16:0AE2 | 16:0B41, 49B2 = FLbackground |
| 22 — Menu CP | 15:11E0 | 15:1BE0 | 15:1289 | 15:13D2, 46A5 = CPmenuTemp |
| 23 — Créditos CR | 9:22C4 | 9:2785 | 9:23CA | 9:240C, 4757 = CRbg |
| 24 — Cena-base vazia | 5:0059 | 5:2665 | 1:3EE2 | Nenhum carregamento próprio; +0C é stub 1:3EE2 |

O slot +4 de todas as tabelas acima resolve para 35:3BD3. Esse método chama +0C na própria instância; portanto a associação construtor → conteúdo foi feita por relocação de tabela, e não pela mera presença dos recursos no mesmo segmento. O slot +8 resolve para 35:3BF8.

## Métodos de cena resolvidos

A tabela preserva os slots concretos para continuar o rastreamento de eventos. Os rótulos semânticos de +18/+20/+24 ainda não foram demonstrados aqui; não devem ser todos tratados como eventos de mouse. A entrada de mouse global passa pelo hit-test +14 e pelo +4 do objeto selecionado, conforme 11-RASTREAMENTO-ATIVIDADES.md.

| Cena | +10 | +18 | +20 | +24 | +28 |
|---:|---|---|---|---|---|
| 3 | 11:090A | 11:0853 | 5:0982 | 5:098D | 11:0884 |
| 4 | 9:1221 | 9:12D3 | 9:05A4 | 5:098D | 9:1272 |
| 5 | 27:0591 | 5:0305 | 27:06C7 | 27:0781 | 27:0641 |
| 6 | 11:1794 | 5:0305 | 11:0D36 | 11:0D75 | 5:09AE |
| 8 | 5:327D | 5:3294 | 5:0982 | 5:098D | 5:09AE |
| 9 | 35:3C20 | 5:0305 | 5:0982 | 5:098D | 5:09AE |
| 10 | 35:3C20 | 25:0294 | 5:0982 | 5:098D | 5:09AE |
| 11 | 35:3C20 | 5:0305 | 5:0982 | 5:098D | 20:1359 |
| 12 | 26:378F | 5:0305 | 26:37DB | 5:098D | 5:09AE |
| 13 | 25:36F7 | 25:3799 | 25:3805 | 25:3821 | 5:1A2F |
| 15 | 21:0616 | 21:031A | 5:0982 | 5:098D | 5:1A2F |
| 16 | 18:1199 | 18:14FE | 18:172F | 18:17BC | 5:1A2F |
| 17 | 19:0FB8 | 5:1B9D | 19:1FA7 | 19:20A9 | 5:1A2F |
| 18 | 27:2CC5 | 27:3907 | 27:2DCF | 27:2E1F | 5:1A2F |
| 19 | 29:1671 | 29:088E | 29:0B09 | 29:0BB3 | 5:1A2F |
| 20 | 16:1A8C | 16:0EDE | 16:0FF6 | 16:11D3 | 5:1A2F |
| 22 | 35:3C20 | 15:1264 | 5:0982 | 5:098D | 5:09AE |
| 23 | 35:3C20 | 5:0305 | 5:0982 | 5:098D | 5:09AE |
| 24 | 35:3C20 | 5:0305 | 5:0982 | 5:098D | 5:09AE |

## Comportamento dos módulos

As regras e os caminhos de mídia/saída estão nos relatórios12-CABINE,13-GRADES,14-MIDIA,18-NAVEGACAO,19-ATIVIDADES-A,20-ATIVIDADES-B,21-BOTOES-JUKEBOX,22-TC-PERFIL,23-IMPRESSAO e24-HW-CELULAS. A tabela acima fornece as instâncias concretas a que esses caminhos pertencem.

## Entrada por caracteres e parâmetros da base de atividades

O slot +18 compara códigos de caracteres, demonstrando caminhos de teclado distintos dos cliques dos objetos filhos. A tradução anterior de teclas físicas para esses códigos deve acompanhar o despachante global. A evidência abaixo preserva o código literal; em especial `4` não é automaticamente sinônimo da tecla física de seta esquerda.

| Cena | Método +18 | Código recebido | Efeito imediato comprovado |
|---:|---|---|---|
| 3 | 11:0853 | s/S | Retorna 1 quando a global 008A habilita o ramo; demais códigos seguem 5:0305 |
| 8 | 5:3294 | K ou # | Constrói transição para cena 22 (32D1) |
| 8 | 5:3294 | c/C | Constrói transição para oficina/cena 0 (3321) |
| 13 | 25:3799 | 4; 6 | Grava 1 nas globais 0F16 ou 0F14, respectivamente |
| 15 | 21:031A | Tabela 21:053C | Tabela contém E,F,H,L,W,e,f,h,l,w; ramos condicionam controlador a estados 1/2/3 |
| 16 | 18:14FE | 4; 6 | Cria comandos por 18:3A5C ou 18:3B2A e executa por 32:009A |
| 16 | 18:14FE | a/A; d/D | Cria comandos por 18:38C0 ou 18:398E e executa por 32:009A |
| 16 | 18:14FE | espaço; 2 | Executa 11:2295 em 18:170F |
| 18 | 27:3907 | 4; 6; 5; espaço | Instancia comando da tabela 27:3BF0 com seletores 0,1,3,2, respectivamente |
| 19 | 29:088E | 4; 6; 8; 2; espaço | Fábricas 29:02EB,03B6,0220,0155,0481; os comandos são executados por 32:009A |
| 20 | 16:0EDE | 4; 6 | Fábricas 16:04F9 e 05C4; comandos executados em 0F3B e 0FB1 |
| 22 | 15:1264 | # | Retorna 1 diretamente; demais códigos seguem 5:0305 |

A base comum 5:14FF é chamada com três palavras de configuração. O construtor guarda `[BP+0A]` em +66, `[BP+0C]` em +64 e `[BP+0E]` em +6C. Os construtores de atividade empilham primeiro o argumento externo `[BP+0A]`, depois um DWORD constante e o ponteiro `this`. Isso fixa os valores sem depender de nomes de recursos.

| Cena | Chamada da base | DWORD constante | +66 | +64 | +6C |
|---:|---|---|---:|---:|---|
| 13 | 25:33B9 | 000C000D | 13 | 12 | Argumento externo |
| 15 | 21:0116 | 0020000F | 15 | 32 | Argumento externo |
| 16 | 18:071B | 00130010 | 16 | 19 | Argumento externo |
| 17 | 19:0AC6 | 00120011 | 17 | 18 | Argumento externo |
| 18 | 27:252F | 000F0012 | 18 | 15 | Argumento externo |
| 19 | 29:0585 | 00040013 | 19 | 4 | Argumento externo |
| 20 | 16:08BE | 00060014 | 20 | 6 | Argumento externo |

O campo +64 da tabela é o identificador de progresso usado em6:026C. O campo +66 é o seletor da cena. O retorno manual usa os grupos da tabela3:36A6 e os controles3:35A4, detalhados em18-NAVEGACAO.md.

A base 5:1B9D também intercepta w/l quando a global 008A está habilitada, encaminhando 1/0 ao slot +4C da cena; fora disso delega ao teclado genérico 5:0305. Isso permite forçar resultados por um caminho condicionado globalmente; não demonstra que as teclas façam parte dos controles normais de vitória/derrota.

## Regra nativa adicional: deslocamento da bola WB

O executor +8 da classe de comando 27:3BF0 resolve para 27:3C88, que passa `owner=+26` e `seletor=+2A` a 28:1457. A cadeia é portanto teclado da cena 18 → comando 27:3BF0 → 28:1457, sem depender de prefixos de recursos.

28:1457 rejeita todos os comandos se `+A8 != 0` ou se estado `+AA == 12`. O seletor deve estar entre 0 e 3. O seletor 0, ramo 1489, exige `+A6 != 0`, copia a posição de +CE para +D2/+D6, subtrai 37 da coordenada em +D2, define estado +AA=3 e direção +A6=0. O seletor 1, ramo 14F7, exige `+A6 != 1`, copia a mesma posição, soma 37 à coordenada +D2, define +AA=4 e +A6=1. Os ramos comparam a posição atual às globais 2132/2134 antes de solicitar áudio 000C84031 (isto é, low word 4031 e parâmetro alto 200) em 14E0/1547. Isso comprova passos de 37 e bloqueios de estado do deslocamento horizontal.

O seletor 2 (espaço), ramo 1568, se estiver em estado 3 ou 4, grava a direção -1/+1 em +E0 e muda para estado 5. Quando o estado é 2, o caminho 15AE calcula `(posiçãoX - limiteEsquerdo) / 37` para indexar a coluna. O restante do ramo deve ser seguido para concluir colisão/dano. O seletor 3 corresponde ao código `5` e seu ramo separado; não foi rotulado como tiro ou parada sem observar seus efeitos.

## Regra nativa adicional: comandos do canhão WC

Na cena 19, as fábricas direcionais do teclado chegam ao controlador 29:1DC2. A fábrica `29:0155`, usada pelo código `2`, instala a vtable 29:1A51; seu executor 29:01AD chama 29:1DC2 com seletor 3. A tabela em 29:1EB8 contém os alvos 1DDB,1DFA,1E4E,1E17,1E2E para seletores 0..4.

Os seletores 0 e 1 rejeitam estado +48=7; caso permitido, definem +48=3/4 e zeram +44. Os seletores 3 e 4 também rejeitam +48=7; caso permitido, definem +4A=5/6 e zeram +46. O seletor 2 só define +48=7 quando tanto +48 quanto +4A estão em {1,2}. Os caminhos rejeitados retornam 1 e solicitam o som low-word 401B com parâmetro 1, em 29:1EAB. Isso demonstra intertravamento de movimento/ação e áudio de recusa; a semântica final das coordenadas deve acompanhar o atualizador do controlador.

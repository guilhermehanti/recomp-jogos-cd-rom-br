# HW — entrada por células e conectividade

O módulo HW, cena 17, usa uma malha de 10 colunas por 6 linhas. Em 19:1062 o laço percorre 60 posições, incrementa X em 64 e, a cada dez células, reinicia X e soma 64 a Y (19:1124–115B e 11DF–11EE). Cada ponteiro é armazenado em scene+80+4*índice. O tipo inicial passado ao construtor 19:2CCE é 1; o identificador visual é 4056.

## Clique até a troca de peça

Cada iteração constrói um comando da tabela 19:24EA com o ponteiro da cena em +26, o índice em +2A e a flag 1 em +2C. O construtor da célula 19:2CCE recebe esse comando e cria a ação 19:3628 em 19:2E16. A ação instala 19:36CD, cujo slot +4 é 19:367C e +14 é 33:1750. O receptor exige o bit de pressionamento 2 e ponto dentro da área, testado pelo slot +24 do visual. Em caso de acerto executa o comando por 33:1750 → 32:009A.

O slot +8 do comando 19:24EA é 19:2582. Ele exige +2C não zero e chama 19:16CC com scene+26 e índice+2A. A troca só é aceita quando o controlador scene+18E está no estado +42=3, a célula clicada está com +12=0 e o tipo é 1 ou está entre 5 e 10. Não é uma rotação livre de qualquer célula.

19:16CC obtém os comandos dos dois objetos visuais, o da célula clicada e o de scene+170. Configura no controlador de troca o índice em +52, o estado 0 em +42 e os dois comandos em +4C/+48. Incrementa scene+1B0. Quando esse contador iguala +1B4, solicita o rótulo 2 da animação scene+1B6. A atualização do controlador é, portanto, necessária para concluir a apresentação da troca.

## Tabela completa de entradas e saídas

As direções são 0 esquerda, 1 direita, 2 cima e 3 baixo. 19:3141 usa a tabela literal de 10 entradas em 19:31AD. 19:31C1 usa a de nove entradas em 19:3226. Só os tipos 5–10 são atravessáveis no teste 19:311E. O tipo 1 é aceito pelo teste isolado de entrada, mas é barrado pelo teste de atravessabilidade que o precede.

| Tipo | Entradas aceitas por 3141 | Saída correspondente por 31C1 |
|---:|---|---|
| 0 | Fora da tabela de entradas; tratado como destino final pelo controlador da cena | Resultado da cena |
| 1 | Qualquer direção | Sem saída definida por esta tabela |
| 2 | Nenhuma | Força entrada 0 e saída 1 |
| 3 | Nenhuma | Sem atribuição própria de saída |
| 4 | Nenhuma | Sem atribuição própria de saída |
| 5 | 0, 2 | 0→2; 2→0 |
| 6 | 0, 3 | 0→3; 3→0 |
| 7 | 1, 2 | 1→2; 2→1 |
| 8 | 1, 3 | 1→3; 3→1 |
| 9 | 2, 3 | 2→3; 3→2 |
| 10 | 0, 1 | 0→1; 1→0 |
| Outros | Nenhuma | Sem atribuição própria |

A função de saída grava a entrada em cell+14 e a saída em cell+16. Seus ramos padrão preservam DX, por isso a reconstrução deve invocá-la depois das mesmas guardas do original. A tabela não autoriza aplicar uma saída padrão a tipos inválidos.

O avanço do fluxo, os limites, o destino tipo 0 e os resultados estão descritos em 20-ATIVIDADES-B.md. A saída manual usa o botão comum para a cabine 2, cujo controle de saída retorna à oficina 0, conforme 18-NAVEGACAO.md.

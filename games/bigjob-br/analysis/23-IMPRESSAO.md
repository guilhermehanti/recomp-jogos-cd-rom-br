# CT: controles, montagem e impressao

Analise exclusivamente estatica do original NE, sem executar jogo, impressora ou compilacao. Segmentos decimais, offsets hexadecimais. Fontes: `disassembly/seg10.asm`, `seg11.asm`, `seg02.asm`, `seg34.asm`, `references.json` e bytes de `build/BIGJOBNG.EXE` segundo os segmentos de `mapa-jogo-v3.json`.

## Cena e entrada

A cena6 usa construtor11:0C92, vtable11:1A46 e montagem11:0E78. O construtor zera +8E (roteiro), +64, globais DS:2120/2124. O owner possui motor em +66, construido em11:12AE ->10:23C7 com janela/owner +4, composicao10:0B60 e a propria cena. O motor guarda esses argumentos em +418,+41E,+442, e inicializa modo+422=0, movimento+424=0, ciclo+426=0 e ponteiro+428=0 (10:2450-249C).

O retorno e uma area retangular (18,309,47,440), criada11:1203 ->32:3735; o comando construido11:116F-1186 tem vtable2:1AC3 e campos +26=0,+28=FFFF,+2A=4. Portanto esta saida leva a cena0; nao concede premio de atividade.

## Todos os seletores da interface

11:1B21 cria quatro controles, posicionados em11:1D1C-1DC0. A selecao inicial +34=2 esta em11:1B5E; o motor tambem inicia em modo0. 11:1EEE associa comando ao vetor+24+4*i. 11:1F12 troca a imagem do seletor anterior, aplica a imagem do novo, executa seu comando por32:009A (1FE4-1FEC) e grava +34. Se o indice nao mudou, retorna sem executar novamente (1F1B-1F25).

| Indice UI | Posicao | Recurso normal DS:064A | Recurso selecionado DS:0652 | Campo da cena | Modo escrito em motor+422 |
|---:|---|---|---|---|---:|
| 0 | 66,400 | 4862 | 4860 | +72 | 2 |
| 1 | 108,400 | 488E | 488D | +6E | 1 |
| 2 (inicial) | 181,402 | 487C | 487B | +6A | 0 |
| 3 | 101,426 | 4879 | 4878 | +76 | 3 |

A ordem acima e comprovada por11:1491-14E0; nao se deve copiar a ordem dos comandos construidos como ordem visual. Os quatro comandos sao10:30D0, executor10:3126 ->10:2981; este somente escreve o seletor em +422. Os WORDs das tabelas foram lidos diretamente no segmento49 do executavel, sem executar codigo. Nomes editoriais dos quatro formatos nao sao necessarios para preservar esta correspondencia.

O botao de acao fica em(300,420), fabrica2:30A9 chamada11:1543, prioridade/parametro200, comando cena+7A construido11:1430 ->10:3199. Seu executor10:3239 chama10:2997. O motor recebe tambem o ponteiro desse botao em10:2753 (11:1582), usado em +428 para controlar seu estado.

Outro botao fica em(460,425), recursos4054/4055, fabrica33:10F8 em11:1660. Seu comando11:193A executa11:1989, que chama owner slot+30; na vtable11:1A46 esse slot aponta11:18CB, que chama10:2CEC. Este e o caminho de impressao do documento, separado do comando que inicia o ciclo animado.

## Recursos e roteiro

A montagem11:0E78-110F adiciona os recursos486C,4870,4871,486E,486F,486D,487A por37:011E. Coordenadas literais incluem(0,0),(611,0),(0,321),(617,249),(0,458),(46,386). A familia de imagens dos seletores usa as duas tabelas listadas acima. O motor cria sete elementos de tamanho7E em+A6, com flag de atividade em+122+7E*i (10:242E-2445,24AD-24FE).

O roteiro MMS01A5 e construido11:16EB-16F6 e guardado em+8E.11:186E recebe label, constroi salto por30:0FBD e executa32:009A;10:272B encaminha os labels a esse owner. Assim os labels9 e25 sao ramos de rejeicao do ciclo, e12/13 acompanham suas fases. A introducao consulta2:0D82 com cena6: se positiva, escolhe label3 e bloqueia via5:0B18; senao usa parametro32 em30:0DA8 (11:1727-1789).11:0D36 remove o roteiro ativo e limpa+64/+1E;11:0D75 permite reativa-lo.

## Ciclo do motor e condicoes

10:2997 nao imprime no sistema operacional. Se +426 for1/2, limita-se a reativar o controle+428 quando seu+4=1. Senao consulta a composicao+41E:10:1D61 devolve+4; zero escolhe label9.10:1D4E devolve+392; zero escolhe label25. Somente ambos positivos escrevem+424=1 e+426=1 (10:2A5B-2A65).

10:2A71 e a atualizacao periodica, com espera/temporizacao37:1971 de50. +424 governa movimento:0 incrementa+41C de10 ate50 e passa a3;1 decrementa de10 ate abaixo10 e passa a2;3 conserva atualizacao. +426 governa fases:

| Estado+426 | Operacao | Proximo estado |
|---:|---|---:|
| 1 |10:1C5E avanca composicao ate450; atualiza+42C |2; label12 e10:13F3 |
| 2 |10:1CA2 atualiza ate exceder670 |3; chamadas10:04E2,137D,0CE8,1E40 e label13; movimento+424=0 |
| 3 |24:2981 sobre+42C com passo5/alvo355 |0 quando concluido |

O label13 pertence ao MMS. A rotina10:28CA procura o primeiro dos sete elementos livres quando10:27B3>80, aplica o modo+422 por10:05EC e o ativa. Esta e a ligacao do formato selecionado ao efeito visual do ciclo.

## Documento e backend de impressao

10:2CEC rejeita se2:11E7 indicar backend ocupado, ou se ciclo+426=1/2; toca40A1 nesse caso e retorna.2:11E7 consulta o singleton DS:00BE, retornando ocupado quando estado+42 e diferente de0/6. Caso livre,2:1152 cria esse singleton pela fabrica34:2902 (tamanho472), vtable34:2DD1.

10:2D60-2FE1 prepara esse objeto por34:1FBF e define retangulo(0,0,640,480) por34:1F9E.34:2950 tenta sequencialmente34:2087,20CB,2118,216B; qualquer falha desvia ao retorno sem superficie valida.34:2118 monta estrutura de10 bytes e chama importacao `GDI!377` em34:2140; resultado negativo encaminha erro3 a34:2055. As referencias importadas permanecem numericas, sem depender de tabela externa de nomes de API.

Na superficie resultante,10:2EDB chama10:0EB1 com composicao+41E e origem(0,0).10:2F20 obtem o texto do jogador por6:0258 do gerenciador49:0432; cria elemento de texto por38:15E7 na posicao(391,437), aplica tamanho10 e parametros11/14 (2F61-2FA3), e o adiciona a superficie.10:2FD9 finaliza por34:2A21 ->37:1678. O backend posteriormente usa34:2A38, estado6 e callback+44E; nao e correto tratar a submissao como impressao fisica comprovadamente concluida.

A remontagem deve manter separados selecao de modo, ciclo visual e documento enviado ao backend. Os enderecos e estados acima fecham a cadeia de entrada/controles/recursos ate a submissao estatica; configuracao de driver, resultado de spool e aparencia do papel nao foram verificados por execucao.


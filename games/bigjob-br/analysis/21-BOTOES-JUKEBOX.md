# Entrada dos botões e jukebox

## Contrato comum de botão

O construtor 33:10F8 cria o objeto visual por 38:1F3E e a ação por 33:1773. Esta instala 33:223D. O slot +4 aponta a 33:17CC, o slot +14 a 33:1750 e o slot +18 a 33:18D2.

Em 33:17CC, +0E registra se o botão está capturado. Se estiver solto, o evento com bit 2 e ponto dentro da área chama +18 com 1 e marca a captura. Enquanto capturado, eventos de movimento atualizam +18 segundo o teste de área. O evento com bit 4 encerra a captura. Se o ponto ainda estiver dentro, chama +14, que executa o comando registrado por 32:009A. Em seguida chama +18 com zero e limpa a captura. Portanto arrastar para fora e soltar não executa a ação. A palavra retornada é o estado de captura, consumido pelo despachante de mouse 31:15E5.

Este contrato é diferente do objeto de área 33:1903/1956, que executa no pressionamento. A reconstrução deve preservar o tipo usado em cada controle.

## Cena 12 — jukebox VJ

A oficina seleciona a cena 12 pelo comando de 8:0731. O inicializador 26:2FFA instala 26:3811. O método +0C é 26:325B, que cria quatro botões pelo contrato acima.

| Imagens pressionada/solta | Posição inicial das imagens | Comando | Filme no banco BIGJOB.MOV |
|---|---|---|---:|
| VJbjBD/BU, 4DBA/4DBB | 128, 196 | 26:34AC → 4:22A9; botão 26:34EC | 2 |
| VJconstBD/BU, 4DBC/4DBD | 196, 196 | 26:355A → 4:22A9; botão 26:359A | 9 |
| VJfarmBD/BU, 4DBE/4DBF | 289, 196 | 26:3608 → 4:22A9; botão 26:364C | 19 |
| VJfireBD/BU, 4DC0/4DC1 | 357, 196 | 26:36BD → 4:22A9; botão 26:3703 | 22 |

Os valores da segunda coluna são os pares de palavras empilhados como posição. O tamanho e o teste da área derivam da imagem carregada pelo objeto visual.

A ação executada cria o comando 4:21E0 por 4:22A9, guardando o filme em +2A e o wrapper em +26. O executor 4:228B chama 4:1DC3. O estado 4:22F9 recebe o filme em +46. Depois de preparar a interface, 4:2353 chama 4:1929 em 4:239D, que alcança o player 33:2FBA. O banco e os offsets concretos dos quatro filmes estão em 14-MIDIA.md.

A montagem carrega MMS 0122/0123 por 26:32F6/3360. O som 428B é iniciado por 5:0A1E em 26:3782. O tratamento da reprodução usa a versão escurecida VJbackgroundDim, carregada em 26:30CD, e restaura VJbackground em 26:31C3. O método de saída +20, 26:37DB, atua sobre o wrapper +76. O caminho global de saída de cena e os controles de navegação estão em 18-NAVEGACAO.md.

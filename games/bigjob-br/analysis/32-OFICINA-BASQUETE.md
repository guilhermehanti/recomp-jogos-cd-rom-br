# Oficina: bola, cesta, clique e regras nativas

Esta interação ocorre dentro da oficina, cena0. Não constitui uma13ª cena de atividade nem precisa de uma troca de cena para retornar. Os12 retângulos de17 cobrem navegação/ações registradas por área; não eram uma enumeração de todos os controles nativos filhos. O basquete acrescenta um receptor próprio de mouse.

## Objeto e entrada

8:15A5 chama8:25B1, armazena o controlador em `scene.+7E` e o inicia por37:1678 em15C6. O objeto tem0x12A bytes e vtable8:47D2, cujo slot+14 em47E6 aponta4118. Constrói dois players em+42/+74, visual em+AA, região interativa em+B2, ação em+B6 e aro em+AE. A posição inicial é(452,140), copiada para+D2/+D4.

8:28AD constrói a ação8:458F, entregando controlador e região+B2. A ação guarda esses ponteiros em+0A/+0E e instala vtable4782. Seu slot+4 é45EA. Em45F6 exige bit2 do evento; em461A testa a posição pelo slot+24 da região. Se está dentro,4638 chama34B4 com as duas coordenadas e retorna1, consumindo o evento. Não depende do contrato pressionar/soltar dos botões comuns.

34B4 rejeita novo impulso enquanto+F4 está ativo. Calcula o centro da região por posição e dimensões, com divisão inteira, e usa as diferenças até o clique: componente horizontal = `(centroX-cliqueX)*E0`, vertical = `(centroY-cliqueY)*DE`. Guarda módulos em+CE/+D0 e sinais em+D6/+D8. Limita cada módulo a300; com+F2 ativo o limite cai para60. A multiplicação é WORD e deve preservar os efeitos de overflow do original; não substituir sem comparação por física contínua em ponto flutuante.

## Tipos e parâmetros

A tabela CS:34AC é exatamente `[31F5,2ED6,3076,333C]`. Assim os blocos de código não aparecem na ordem dos tipos.2EAF grava tipo em+128 e configura os parâmetros:

| Tipo | Bloco | Gravidade +DA | Atrito +DC | Impulso Y +DE | Impulso X +E0 | Restituição vertical +E2/+E4 |
|---:|---|---:|---:|---:|---:|---|
|0, bola normal|31F5|9|3|25|10|2/3|
|1, Terra|2ED6|10|3|25|10|1/2|
|2, Lua|3076|3|3|18|8|3/5|
|3, olho|333C|9|3|25|10|2/3|

Tipos1/2/3 solicitam respectivamente4329/432D/432A na configuração, no player+74. O tipo0 não passa por esses três disparos. A seleção não é inferida só pelos nomes dos sons: as quatro entradas da tabela foram conferidas no executável original. A troca normal é consequência da série de acertos, descrita abaixo.

## Atualização e colisões

4118 atualiza os visuais no owner, limpa flags de colisão+EA/+EC/+EE/+F0, avança `X += trunc(CE/10)*D6` e `Y += trunc(D0/10)*D8`, depois atualiza geometria por3631. Chama3E66 para cesta e3A83 para limites. Espera30 pelo agendador37:1971 e repete enquanto a tarefa permanece ativa. O30 é argumento do agendador, não uma taxa de quadros garantida pelo host.

3A83 limita Y a10..305, marcando teto(+EE) ou piso(+EA). X mínimo166; máximo resulta de495 menos a largura do retângulo+120. Marca+F0 nas paredes.39CC reduz velocidade vertical pelo quociente E2/E4 e inverte D8; ao zerar a vertical fora da colisão de aro também reduz horizontal em2.3A3D divide CE por2, trunca e inverte D6.40C6 aplica gravidadeDA ao módulo vertical, invertendo o sinal quando cruza zero. No piso o atritoDC também reduzCE, com mínimo0.

O aro usa retângulos/pontos+100,+104,+108,+10C; a bola usa o retângulo+120. O corpo3E66 testa a passagem acima/abaixo de+102 e diferencia aproximação+F2, bloqueio+F4 e colisão descendente. Só com D8>0 considera cesta. Interseção com+10C e+F2 ativo chama3BD8; os demais testes com+104/+100 chamam3C7F para resolver os dois lados do aro.3C7F ajusta posição/velocidade e marca+EC; contém correções aleatórias e limites específicos. Todas as comparações e sua ordem, inclusive bordas inclusivas/exclusivas diferentes, estão preservadas no bloco de fonte em `workshop-ball.json`, sem substituição por um teste genérico de distância.

## Cesta, série e falha

3BD8 corrige a posição quando necessário, marca+F4=1, limpa+FA, incrementa+FC (série), zera+FE e marca+F6 para animação do aro. Restaura imagem pelo seletor5FB, limpa+F2, escolheCE entre10 e25 inclusivos e D6=1; chama3B34 para áudio.

No primeiro piso após a trajetória, se+FA está ativo,441E limpa esse sinal, zera a sérieFC e incrementaFE. Se o tipo era especial, volta ao tipo0 por4440 e solicita410B no player+74. Os limiares de série são exatos:FC==3 muda para tipo1;FC==6 para2;FC==9 para3, desde que o tipo ainda difira. Não são pontuações aproximadas baseadas na aparência.

4050 anima o aro usando DS:0562 e índice global05AC:18 quadros, depois zera índice e+F6.3830 anima a bola usando índice global05AA e as tabelas DS049E/04AE/0526/04EA:8 quadros para tipo0 e30 para os outros. Preservar esses índices globais. As tabelas completas foram acrescentadas ao JSON reproduzível.

## Áudio concreto

| Evento e condição | Site | Recurso |
|---|---|---|
|Cesta: som do aro|3B55|referência+CA, construída com432C|
|Primeira cesta, E8 ainda ativo|3B7B|4311, depois E8=0|
|Demais cestas, sorteio0..100 <25|3BCB|DS0586, índice inclusivo0..17|
|Piso/obstáculo EA ativo e D0>20|437A ou43E3|BA/BE por tipo1 e alternânciaF8; C2 para colisãoEC|
|Parede F0 ativa e CE>20|4521|referênciaC6|
|Perda de série com tipo especial|4459|410B, após voltar ao tipo0|

O sorteio de fala contém **18** recursos: a última entrada é4311 emDS05A8. O catálogo anterior lia17. A rotina2:0012 inclui os dois limites; a chamada00110000 significa0..17, não0..16. Após o sorteio inicial menor que25, o código escolhe a entrada por um segundo sorteio. Não arredondar isso para uma probabilidade exata de25%, pois o algoritmo usa resto de divisão.

Os sonsBA/BE/C2/C6 por tipo estão em26 e são configurados por2EAF. Os sites437A,43E3 e4521 usam o player+42 e cancelam seu áudio anterior antes de tocar; não disparar todos os impactos simultaneamente em players novos.3B55/3B7B/3BCB usam+74.

## Vídeo, progresso e saída

O caminho mapeado de cesta não seleciona um filme nem chama o sistema de30 recompensas. Os30 IDs válidos já pertencem aos contextos de27. O brinquedo permanece no owner da oficina; navegar usa os comandos normais da cena0. Não inventar um botão de retorno ou um filme de vitória para esse brinquedo. As teclas1/2/3/4/~ em464A são condicionadas à global de diagnóstico008A e não substituem o clique normal.

`python tools/trace_workshop_ball.py` confere hash do executável, tabela de tipos,18 sons, referências virtuais e guarda os corpos nativos usados nesta análise. Não executa código original. A equivalência temporal/visual será validada quando a implementação integrada existir.

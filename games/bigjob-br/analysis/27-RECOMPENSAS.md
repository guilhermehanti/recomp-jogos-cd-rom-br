# Fechamento dos contextos de recompensa

## Som de concessão e repetição

`6:026C` solicita a fala4169 (`BIGVOX[361]`, `TuEarnedTrCard01`) somente quando o ponteiro proprietário em BP+0E é não nulo e a flag `gerente.+12+ID` ainda é zero. Em6:0289 empilha ID4169/parâmetro1;6:0294 constrói2:23D8 e6:02A1 executa o comando. Depois6:02B6 monta a recompensa por6:04E6. Se a flag já estava marcada ou o proprietário é nulo, segue2:1E41, não repete essa solicitação. No fim grava BP+0C na flag; o teste inicial não deve ser substituído por um teste do nome do filme.

O executor2:242C lê ID/argumento de comando+26/+28 e chama5:0A1E sobre a cena corrente da aplicação. O DWORD em `app.+4DA+4*app.+566` é esse receptor de cena, não um ID de som. A distinção corrigiu uma atribuição anterior do inventário de áudio. O comando chega a36:28F7 pelo player `scene.+22`. Assim a fala de concessão está ligada aos30 contextos pela rotina comum de progresso, sem inventar um gatilho em cada script.

A tabela de recompensas DS49:0204 tem36 entradas,30 filmes válidos e6 sentinelasFFFF. A leitura de43:0000 fornece21 registros de8bytes que traduzem o filme visto na cabine em identificador de progresso. Os9 construtores de atividades fornecem os outros9 identificadores. A união é exatamente o conjunto das30 entradas válidas, sem duplicação.

## Cadeia de conclusão da cabine

O comando da classe12:3328 com seletor5 grava o filme em scene+125. O seletor7, tabela3454[7]=3403, chama29E6 e solicita a reprodução por4:1929. A fábrica12:25C7 constrói o seletor6, tabela3454[6]=3412, e fornece esse comando ao wrapper de vídeo em12:082F–0862. A execução do seletor6 chama12:2E34. Essa rotina usa12:2EA6 para procurar o filme em43:0000 e entrega o identificador encontrado a6:026C, com modo1 e a cena proprietária.

As linhas6 e7 da tabela de seletores no relatório12 estavam invertidas. A tabela binária3454 e as instruções da fábrica25C7 confirmam6 como registro de progresso e7 como solicitação de reprodução. A tabela do relatório foi corrigida.

## Tradução dos21 filmes da cabine

| Filme visto | ID de progresso | Filme de recompensa | Registro original |
|---:|---:|---:|---|
|14|10|15|43:0000|
|6|11|7|43:0008|
|52|9|56|43:0010|
|47|7|48|43:0018|
|38|16|0|43:0020|
|39|30|40|43:0028|
|1|0|24|43:0030|
|10|17|11|43:0038|
|28|20|29|43:0040|
|50|27|51|43:0048|
|53|33|54|43:0050|
|37|2|44|43:0058|
|33|25|34|43:0060|
|30|21|31|43:0068|
|3|24|4|43:0070|
|12|35|13|43:0078|
|16|3|17|43:0080|
|42|28|43|43:0088|
|26|13|27|43:0090|
|45|8|46|43:0098|
|35|1|36|43:00A0|

## Identificadores fornecidos pelas atividades

| Cena | ID de progresso | Filme de recompensa | Construção da base |
|---:|---:|---:|---|
|7|31|32|6:1AF4|
|13|12|55|25:33B9|
|15|32|23|21:0116|
|16|19|5|18:071B|
|17|18|21|19:0AC6|
|18|15|57|27:252F|
|19|4|49|29:0585|
|20|6|25|16:08BE|
|21|23|41|17:002D|

## Gravação e revisão de recompensas

A fábrica de conclusão2:2671 é chamada pela base5:14FF em5:180F. Seu executor2:26CE alcança6:026C. A outra chamada direta a6:026C é a da cabine12:2E6C. A busca das referências expandidas e das chamadas locais identifica esses dois caminhos de concessão. O painel9:171C também chama6:04E6 diretamente para apresentar uma recompensa selecionada; essa apresentação não implica nova concessão por6:026C.

A antiga anotação de que VM/TC/SB tinham contextos de progresso sem atribuição foi retirada do catálogo. Os30 contextos usados pela tabela de filmes estão explicados pelos21 eventos da cabine e9 conclusões da base. Essa cobertura não deve gerar novos prêmios para os outros módulos.

A reprodução é verificável por tools/catalogue_media.py. O script lê as duas tabelas do executável original e exige que a união dos identificadores seja exatamente igual às entradas nãoFFFF. Os dados estão em media-catalogue.json, campos cabin_completion_progress e activities_with_proven_progress_id. Nenhum código do jogo foi executado nesta verificação.

# Evidência de entrada das chamadas nativas de áudio

Situação atual: 81 sites com gatilho local descrito, 3 interfaces e 1 sem produtor direto identificado. Pendências de classificação: 0.

A tabela cobre os 85 pontos catalogados. A entrada é a âncora local de prólogo. Referências NE podem ser ponteiros armazenados para despacho posterior. A janela lista saltos locais anteriores ao site e seus destinos, sem assumir que todo salto mostrado domina a chamada. A comprovação do evento da interface exige seguir o controlador proprietário.

| Site | Situação | Entrada | Entradas diretas observadas | Saltos locais na janela anterior |
|---|---|---|---|---|
| 2:3540 | Gatilho local descrito | 2:34EE | 2:2DEF (NE), 6:29BE (NE) | 34FF jne 35DD, 3508 jne 3569 |
| 2:35A2 | Gatilho local descrito | 2:34EE | 2:2DEF (NE), 6:29BE (NE) | 3560 je 35D1 |
| 3:07E8 | Gatilho local descrito | 3:079E | 2:1D16 (NE) | 07AF ja 0B22, 07C1 je 07F3 |
| 3:1DA8 | Gatilho local descrito | 3:1D5A | 2:1DE0 (NE) | 1D6F ja 1F21, 1D81 je 1DB3 |
| 3:2708 | Gatilho local descrito | 3:26E5 | 3:2B57 (jmp), 3:2B1A (NE) | — |
| 4:2846 | Gatilho local descrito | 4:2806 | 4:2619 (NE) | 2817 jne 284E |
| 5:0A3A | Interface | 5:0A1E | 5:2046 (callw), 2:2464 (NE), 9:25DC (NE), 17:1B3E (NE), 25:0F56 (NE), 25:0CE4 (NE), 25:0C9B (NE), 25:10D7 (NE), 26:31B4 (NE), 26:3783 (NE) | — |
| 5:202B | Gatilho local descrito | 5:1F20 | 5:24D2 (NE) | 1FD7 je 1FF8 |
| 6:28A6 | Gatilho local descrito | 6:1C7E | 6:3792 (NE) | 2837 je 2882, 2849 je 2855, 2867 je 2878 |
| 6:2C4F | Gatilho local descrito | 6:2B81 | 6:29CC (NE), 6:383A (NE), 7:0C88 (NE) | — |
| 6:3728 | Gatilho local descrito | 6:36FF | 7:048A (NE) | — |
| 7:0EA0 | Gatilho local descrito | 7:0D9A | 7:0D3D (callw) | 0E6C ja 0E82 |
| 7:0F75 | Gatilho local descrito | 7:0F3A | 6:2BF0 (NE) | — |
| 8:2EE9 | Gatilho local descrito | 8:2EAF | 8:2AA6 (callw), 8:4440 (callw), 8:44A9 (callw), 8:4777 (callw) | 2ECB ja 34A7 |
| 8:3089 | Gatilho local descrito | 8:2EAF | 8:2AA6 (callw), 8:4440 (callw), 8:44A9 (callw), 8:4777 (callw) | 301B je 302E, 3061 je 306D |
| 8:334F | Gatilho local descrito | 8:2EAF | 8:2AA6 (callw), 8:4440 (callw), 8:44A9 (callw), 8:4777 (callw) | 32D9 je 32EC, 331F je 3333 |
| 8:3B55 | Gatilho local descrito | 8:3B34 | 8:3C74 (callw) | — |
| 8:3B7B | Gatilho local descrito | 8:3B34 | 8:3C74 (callw) | 3B66 je 3B8F |
| 8:3BCB | Gatilho local descrito | 8:3B34 | 8:3C74 (callw) | 3B66 je 3B8F, 3BA0 jge 3BD3 |
| 8:437A | Gatilho local descrito | 8:4118 | 8:47E6 (NE) | 4326 je 43A6, 432E jle 43A6, 434B jne 4396, 4353 je 4362 |
| 8:43E3 | Gatilho local descrito | 8:4118 | 8:47E6 (NE) | 43AF je 43EB, 43B7 jle 43EB |
| 8:4459 | Gatilho local descrito | 8:4118 | 8:47E6 (NE) | 43F4 jne 43FE, 43FC je 4409, 4412 je 44CD, 441C je 4461, 4437 je 4461 |
| 8:4521 | Gatilho local descrito | 8:4118 | 8:47E6 (NE) | 44C2 jge 44E4, 44D6 je 44E4, 44ED je 4534, 44F5 jle 4534 |
| 9:1AFC | Gatilho local descrito | 9:145E | 9:14C0 (callw), 9:14D8 (callw), 9:1736 (callw), 9:174E (callw), 9:19BF (callw), 9:19D7 (callw), 9:1F45 (callw) | 1A97 je 1AAF |
| 10:13D7 | Sem produtor direto identificado | 10:139A | — | — |
| 10:1430 | Gatilho local descrito | 10:13F3 | 10:2BFF (callw) | — |
| 10:15E2 | Gatilho local descrito | 10:15A5 | 10:306A (callw) | — |
| 10:2AAC | Gatilho local descrito | 10:2A71 | 10:3388 (NE) | — |
| 10:2C9B | Gatilho local descrito | 10:2A71 | 10:3388 (NE) | 2C20 je 2CD2 |
| 10:2D46 | Gatilho local descrito | 10:2CEC | 11:18DA (NE) | 2CFD jne 2D12, 2D08 je 2D12, 2D10 jne 2D60 |
| 11:00AE | Gatilho local descrito | 11:002E | 11:0BF6 (NE) | 004A je 010E, 0057 je 0039 |
| 12:268D | Gatilho local descrito | 12:2662 | 12:33AA (callw) | — |
| 15:0B90 | Gatilho local descrito | 15:0B33 | 15:11C3 (NE) | 0B46 ja 0CAC, 0B59 je 0B67, 0B63 je 0CAC |
| 15:0C00 | Gatilho local descrito | 15:0B33 | 15:11C3 (NE) | 0BB9 je 0BC0, 0BD0 jne 0BDB, 0BD7 jne 0CAC |
| 15:0C8B | Gatilho local descrito | 15:0B33 | 15:11C3 (NE) | 0C18 je 0C1F, 0C23 je 0CAC, 0C4E je 0C55, 0C65 je 0C9C |
| 15:23F3 | Gatilho local descrito | 15:23DC | 16:3952 (NE) | — |
| 15:2D7B | Gatilho local descrito | 15:2D54 | 16:32B4 (NE) | — |
| 15:2ECB | Gatilho local descrito | 15:2D9C | 16:31FC (NE), 16:31DB (NE) | 2E51 jne 2E5C, 2E58 jne 2E71, 2E64 jne 2E71 |
| 15:300D | Gatilho local descrito | 15:2EDB | 16:3267 (NE), 16:3247 (NE) | 2F93 jne 2F9E, 2F9A jne 2F62, 2FA6 jne 2FB3 |
| 16:16CE | Gatilho local descrito | 16:1683 | 16:3574 (callw) | — |
| 16:3374 | Gatilho local descrito | 16:316C | 16:3AFD (NE) | 330D je 3323, 3339 ja 3462, 335D je 337C |
| 16:33D3 | Gatilho local descrito | 16:316C | 16:3AFD (NE) | 335D je 337C, 33B3 jne 33B8, 33BC je 33DB |
| 17:1389 | Gatilho local descrito | 17:122E | 17:1223 (callw), 17:26DC (callw) | 1338 jne 1343, 133E je 1343, 134A ja 16FB |
| 17:1428 | Gatilho local descrito | 17:122E | 17:1223 (callw), 17:26DC (callw) | 13C8 je 16FB |
| 17:1479 | Gatilho local descrito | 17:122E | 17:1223 (callw), 17:26DC (callw) | 1441 jne 16FB |
| 17:15CB | Gatilho local descrito | 17:122E | 17:1223 (callw), 17:26DC (callw) | 1595 jne 15DB |
| 17:163A | Gatilho local descrito | 17:122E | 17:1223 (callw), 17:26DC (callw) | 1604 je 1654 |
| 17:2C2A | Gatilho local descrito | 17:2BE3 | 17:3C1C (NE) | 2BF4 je 2C43 |
| 17:2C77 | Gatilho local descrito | 17:2BE3 | 17:3C1C (NE) | — |
| 17:2F6F | Gatilho local descrito | 17:2D9C | 17:3C10 (NE) | 2F37 jne 2FBD |
| 18:185B | Gatilho local descrito | 18:17BC | 18:3E2A (NE) | 17DB je 17E4, 17E2 jne 1806, 17F5 jne 1806, 17FF jne 1806, 180A je 1848, 1832 je 1848 |
| 18:198E | Gatilho local descrito | 18:1868 | 18:3E56 (NE) | 1921 jl 1901, 192E jl 18F6, 1979 jl 1963 |
| 18:1BF5 | Gatilho local descrito | 18:1BCB | 18:3E5A (NE) | 1BE2 jne 1E4A |
| 18:1E79 | Gatilho local descrito | 18:1E4F | 18:3E5E (NE) | 1E66 jne 20AD |
| 18:20EB | Gatilho local descrito | 18:20B2 | 18:3E62 (NE) | — |
| 18:21E1 | Gatilho local descrito | 18:21A8 | 18:3E66 (NE) | — |
| 18:273F | Gatilho local descrito | 18:22A4 | 18:3E6A (NE) | 26EC je 29E6 |
| 18:2A5C | Gatilho local descrito | 18:2A2A | 19:0893 (NE), 19:0872 (NE) | 2A37 je 2A64 |
| 18:2B0C | Gatilho local descrito | 18:2AE3 | 19:097C (NE) | — |
| 18:2E52 | Gatilho local descrito | 18:2DBD | 18:3522 (callw), 18:35E8 (callw) | 2DD6 je 2E5A, 2E30 jg 2E00 |
| 18:3120 | Gatilho local descrito | 18:308F | 18:3C56 (callw) | 30A5 jle 3128, 30DF jl 30BF, 30EC jl 30B4 |
| 19:0348 | Gatilho local descrito | 19:01E3 | 19:0A8F (NE) | 0304 jl 030B, 030F jne 0321, 0325 jne 0337 |
| 20:03B4 | Gatilho local descrito | 20:0329 | 20:0915 (NE) | 038E jne 03BC, 0399 jne 03BC, 03A3 je 03BC |
| 20:0504 | Gatilho local descrito | 20:04D1 | 19:1ADC (NE) | — |
| 20:0806 | Gatilho local descrito | 20:0696 | 20:0674 (je), 20:08FD (NE) | 07BB jne 07CC, 07C5 jne 07CC, 07D0 je 083C |
| 21:1D2F | Gatilho local descrito | 21:1C91 | 21:0483 (callw), 21:04FD (callw), 21:1BF0 (callw), 21:2DDD (callw) | 1CCE je 1CD7, 1CE0 jne 1CE7, 1D0D jne 1D1A |
| 21:29F2 | Gatilho local descrito | 21:2964 | 21:2F7C (NE) | 2973 je 297A, 2985 je 298C, 29AF je 29C1 |
| 23:0DF3 | Gatilho local descrito | 23:030F | 23:37ED (NE) | 0D7A je 0DC5, 0D8C je 0D98, 0DAA je 0DBB |
| 23:1585 | Gatilho local descrito | 23:11EF | 23:1191 (callw), 23:38A8 (callw) | 152B jl 15BF, 1533 jg 15BF |
| 23:1DD1 | Gatilho local descrito | 23:1BFD | 23:0BA0 (callw), 23:0C12 (callw), 23:130A (callw), 23:1389 (callw), 23:160F (callw), 23:1638 (callw), 23:1749 (callw), 23:252F (callw), 23:25B8 (callw), 23:263F (callw), 23:26B6 (callw), 23:282D (callw), 23:2917 (callw), 23:2A2C (callw), 23:2AEE (callw), 23:3167 (callw), 23:364D (callw) | 1D76 je 1D9F, 1D82 je 1D92, 1D89 je 1D92, 1D90 jne 1D97, 1D9B jne 1EEB |
| 25:0B91 | Gatilho local descrito | 25:02DD | 25:334C (NE) | 0B68 je 0B7E |
| 26:051B | Gatilho local descrito | 26:04D4 | 26:1B69 (callw), 26:2193 (callw), 26:2558 (callw) | — |
| 26:1862 | Gatilho local descrito | 26:181F | 26:1801 (callw) | 1831 je 1840, 1836 je 1884, 183B je 18A3 |
| 28:0631 | Gatilho local descrito | 28:03D9 | 28:0034 (callw), 28:0121 (callw), 28:0332 (callw), 28:0372 (callw) | 05BE jl 05CE, 05F6 jne 0601 |
| 28:0D71 | Gatilho local descrito | 28:0CFE | 28:069E (callw), 28:06DD (callw), 28:071C (callw), 28:07B1 (callw), 28:0898 (callw), 28:097F (callw), 28:0A15 (callw), 28:1909 (callw), 28:1931 (callw), 28:198E (callw), 27:3B84 (NE) | 0D11 je 0D83, 0D19 je 0D27, 0D1E je 0D79, 0D23 je 0D27, 0D31 ja 0D51 |
| 28:13B9 | Gatilho local descrito | 28:125B | 28:2474 (NE) | 1368 jne 1374 |
| 28:14E0 | Gatilho local descrito | 28:1457 | 27:3C9B (NE) | 1469 jne 1631, 1473 jne 1478, 147E ja 1631, 1492 je 1631, 14CB jle 14E8 |
| 28:1547 | Gatilho local descrito | 28:1457 | 27:3C9B (NE) | 14CB jle 14E8, 1500 je 1631, 1532 jge 154F |
| 28:160C | Gatilho local descrito | 28:1457 | 27:3C9B (NE) | 15A9 je 15AE, 15DB je 1616 |
| 28:33B2 | Gatilho local descrito | 28:3367 | 29:1C75 (NE) | 3389 je 33F0, 3393 jle 33DB |
| 29:0C59 | Gatilho local descrito | 29:0BB3 | 29:190D (NE) | 0BEB je 0C01, 0C0B je 0C46, 0C30 je 0C46 |
| 29:0D7A | Gatilho local descrito | 29:0C66 | 29:1939 (NE) | 0D08 je 0D67, 0D0F je 0D67, 0D16 je 0D67, 0D3E je 0D54 |
| 29:1EAB | Gatilho local descrito | 29:1DC2 | 29:0354 (NE), 29:0289 (NE), 29:01BE (NE), 29:04F4 (NE), 29:041F (NE) | 1E36 je 1E49, 1E56 je 1E5F, 1E5D jne 1E7B, 1E67 je 1E70, 1E6E jne 1E7B, 1E80 je 1EB3 |
| 30:228C | Interface | 30:224B | 30:1E57 (NE) | 2275 je 227F |
| 36:291E | Interface | 36:28F7 | 3:07E9 (NE), 3:1DA9 (NE), 4:2847 (NE), 5:0A3B (NE), 6:28A7 (NE), 6:2C50 (NE), 6:3729 (NE), 8:2EEA (NE), 8:3350 (NE), 8:308A (NE), 8:3BCC (NE), 8:3B7C (NE), 8:445A (NE), 9:1AFD (NE), 11:00AF (NE), 12:268E (NE), 15:0B91 (NE), 15:0C8C (NE), 15:0C01 (NE), 15:23F4 (NE), 15:300E (NE), 15:2ECC (NE), 15:2D7C (NE), 16:16CF (NE), 16:33D4 (NE), 16:3375 (NE), 18:1BF6 (NE), 18:198F (NE), 18:185C (NE), 18:1E7A (NE), 18:21E2 (NE), 18:20EC (NE), 18:2740 (NE), 18:2B0D (NE), 18:2A5D (NE), 18:2E53 (NE), 18:3121 (NE), 19:0349 (NE), 20:03B5 (NE), 20:0505 (NE), 20:0807 (NE), 21:29F3 (NE), 23:0DF4 (NE), 25:0B92 (NE), 28:0632 (NE), 28:0D72 (NE), 28:13BA (NE), 28:160D (NE), 28:1548 (NE), 28:14E1 (NE), 28:33B3 (NE), 29:0D7B (NE), 29:0C5A (NE), 29:1EAC (NE) | — |

## Vínculos demonstrados

| Sites | Evento e evidência |
|---|---|
| 5:0A3A | Wrapper 5:0A1E usa o player em cena+22. Em 6:026C, a primeira concessão com proprietário e flag zerada cria o comando 4169 pelo construtor 2:23D8. O executor 2:242C usa o ID de comando+26. Os demais produtores estão discriminados no relatório 26. |
| 4:2846 | Atualização cooperativa 4:2528/25E3/2806 aguarda jogador de áudio ocioso e seleciona item aleatório da tabela do construtor. Três construtores: oficina 8:1670, painel 9:04E8 e cartazes 11:0826. |
| 8:2EE9, 8:3089, 8:334F | Configuração dos tipos 1/2/3 da bola em 8:2EAF. A resolução da cesta em 8:4118 muda para os tipos especiais após 3/6/9 acertos consecutivos. Cadeia de clique e tabelas em 32. |
| 8:3B55, 8:3B7B, 8:3BCB | Após a passagem pela cesta, 8:3BD8 chama 3B34. Som do aro sempre, 4311 no primeiro acerto e, nos demais, fala sorteada após rand inclusivo 0..100 <25. |
| 8:437A, 8:43E3, 8:4459, 8:4521 | Atualização da bola 8:4118. Contato vertical com velocidade acima de 20 usa som de piso ou do aro. Parede exige velocidade horizontal acima de 20. A perda da série usando bola especial muda para tipo 0 e solicita 410B. |
| 11:00AE | Seleção de cartaz 11:0921 alimenta +42/+78 da tarefa 11:002E. A tarefa toca 4387 e aguarda seu fim antes de despachar troca de tema. |
| 18:185B, 18:198E | Retomada de controladores da cena GW reativa 4393, conforme caminhos de pausa/retomada em 19. |
| 18:1BF5, 18:1E79 | Comandos de giro GW alcançam 1BCB/1E4F por despacho virtual. O argumento de variante e os testes de posições provêm do objeto de cena e do par em queda. |
| 18:20EB, 18:21E1 | Teclas 4/6 ou controles correspondentes chegam aos comandos 3A5C/3B2A, slots +5C/+60, e alcançam os ramos de deslocamento do par em 20B2 e seguintes. |
| 18:3120 | Caminho de limpeza/plunger 308F solicita 400F. A ligação entre plunger e temporizador depende do estado do par e está descrita no relatório 19. |
| 21:1D2F | A jogada normal cancela áudio +D8, depois escolhe a voz em +10A/+114 conforme o turno, nas instruções 1CED–1D34. |
| 21:29F2 | O controlador da jogada automática 28D0 ativa a rotina de animação 2964, que solicita 4031. |
| 26:051B | A colisão passa por 26:04D4, subtrai dano do HP +16 e solicita 436D antes do teste de morte. |
| 29:1EAB | Espaço chega pelo seletor 2 a 29:1DC2. O ramo de recusa com eixos fora do estado permitido solicita 401B. |
| 30:228C | O interpretador fornece objeto de som para 30:224B, resolve player via 30:0D87 e escolhe argumento 1/7FFF pelo campo +51. O evento concreto é determinado pelo script que invocou o interpretador. |
| 36:291E | A rotina 36:28F7 resolve o ID fornecido pelo chamador via 36:1710 e repassa objeto de som ao executor 36:2937. |
| 17:1389, 17:15CB, 17:163A | A entrada de movimento FM vem da tecla 8/2/4/6 ou comando de direção. 17:11EC exige ator livre e chama 122E. O despacho considera tipo da célula. A tentativa de empurrar a partir de 156C toca 4390 quando o destino extrapola a grade ou já está ocupado. |
| 17:1428, 17:1479 | O mesmo despacho de movimento chega ao tipo 3 (139A) ou tipo 9 (1439). Com flag +32 zerada, solicita respectivamente 4267 e 4266. |
| 17:2F6F | A interação com machado inicia em 17:2D03, marca +22 e guarda alvo. O tick 2D9C toca 400E na fase 2, antes de chamar 3A01 no alvo. |
| 28:14E0, 28:1547, 28:160C | WB: entrada de caracteres 27:3907 constrói 27:3BF0 e seu executor 27:3C88 chama 28:1457. Os seletores 0/1/2 correspondem a `4`, `6` e espaço. Os comandos dos botões WBswingBD/BU chegam ao mesmo controlador (ver relatório 20). Executor 28:1457 ignora +A8 ativo ou estado +AA==12. Seletores 0/1 mudam a direção somente se +A6 difere, e o som é condicionado pelo limite lateral. Seletor 2, com +AA==2 e bloco presente na coluna, muda para 10 e solicita o mesmo 4031. |
| 28:33B2 | Durante estado 7 do jato WC, controlador 28:3367 espera vencer temporizador +C. Com reserva +24 positiva, atualiza o efeito, toca 43A0 e aplica efeito ao alvo antes de reduzir a reserva. |
| 29:0C59 | Retomada da cena WC pelo slot +24 reinicia os dois controladores e solicita 4018 no caminho 0BB3. |
| 2:3540, 2:35A2 | Método 2:34EE do controle genérico, referenciado nas tabelas 2:2DEF e 6:29BE. Exige +0A=0 em 34FA. A flag +04 zero seleciona 3540, e seu valor não zero seleciona 35A2. Ambos cancelam pelo player +20, criam objeto pelo ID +1E, tocam e podem despachar comandos distintos em +18/+14. |
| 3:07E8, 3:1DA8 | Despachos por tabela CS:0B27 e CS:1F58, respectivamente. Os ramos 07BA e 1D7A consultam a configuração em 6:16E7 e tocam apenas quando ela retorna valor não zero. O relatório 25 vincula 6:16E7 a DisablePrint. |
| 10:2AAC | Inicialização da tarefa periódica da montagem CT em 10:2A71, antes do laço 2AC3. Cria e solicita o som inicial 4361. |
| 10:1430 | Clique de ação CT alcança 10:2997 pelo comando 10:3199. Com os dois testes da composição satisfeitos, entra em ciclo 1. No tick 2B64, se 1C5E concluir avanço, faz label 12 e passa em 10:13F3. |
| 10:2C9B | Mesmo tick CT, no ciclo 2, apenas após 1CA2 retornar não zero. Atualiza composição, passa label 13 e reinicia a solicitação de 4361. |
| 10:2D46 | Clique de impressão CT segue 11:193A → 11:1989 → slot +30 → 11:18CB → 10:2CEC. O som 40A1 acompanha recusa por backend ocupado ou ciclo 1/2. |
| 23:1585 | Executor 23:11EF chega a 1527. Aceita os seletores 19 a 24 e escolhe de DS:0DDC pelo seletor menos 19. A tabela 13 associa esses seletores ao comando de buzina do objeto visual. |
| 20:03B4 | O atualizador 20:0329 no ramo 0389 exige flag +5C zero, player +18 zero e recurso +4E diferente de FFFF. Os produtores 01D1/02C1 e 04F0 escolhem respectivamente fogo e jato. |
| 29:0D7A | Atualização 29:0C66 testa estados 1/2/3 do controlador, prepara comando de interrupção se necessário e converge em 0D67 para solicitar a repetição de áudio 4018. O método está na referência de tabela 29:1939. |
| 6:28A6 | Montagem da cena CG em 6:1C7E, slot da tabela 3792, solicita 4304 com argumento 7FFF. |
| 6:2C4F | Seletor de chegada de animal em 6:2B81. O percurso ao ultrapassar a coluna limite chama o ramo 2C25, que atualiza +382 e sorteia 1 de 4 IDs em DS:043E. |
| 6:3728 | Desvio de animal para o canteiro em 7:0469 → 6:36B9. A referência 7:048A aponta para a rotina 36FF que solicita 41A6 nesse fluxo. |
| 7:0EA0 | Atualização 7:0D0D chama 0D9A em 0D3D no fluxo de gerar animal. O tipo da entidade seleciona o som, com 4300 no caso padrão. |
| 7:0F75 | Espaço da cena CG em 6:296A cria seletor 9 de 2B81, que entra em 7:0F3A. A rotina cancela o som anterior, toca buzina e manda parar os animais ativos. |
| 12:268D | Comando 0 do controlador de cabine usa a entrada 12:33A0 que chama 2662. Cancela áudio anterior em +F3, solicita 400E, alterna animação +77/+7B e interrompe movimento. |
| 16:3374 | Tick 16:316C do controlador FL. No ramo 3350, a retirada 1C8F deve retornar não zero para solicitar 4389. |
| 16:33D3 | Mesmo tick, outra retirada. 33B3 testa retorno não zero de 33A9 e o teste 33B8 exige o índice local não zero antes da segunda solicitação de 4389. |
| 18:273F | Atualização de gravidade 18:22A4 do par GW. O ramo 26EC exige descritor de célula em BP−4E não nulo, ajusta o visual pela chamada 0528 e solicita o som. |
| 18:2A5C | Callback 18:2A2A, referenciado por 19:0872 e 0893. O parâmetro WORD BP+0C deve ser diferente de zero e a rotina cancela o som corrente antes de solicitar o novo. |
| 18:2B0C | Entrada 18:2AE3, referenciada por 19:097C. Solicitação antecede as operações de remoção/arranjo de bloco. |
| 18:2E52 | A correspondência de tipos de pedido em 18:3538 arma a animação do caminhão por 2DBD. O som é solicitado ao final da preparação deste método. Há também uma chamada local em 35E8. |
| 20:0504 | No término de um caminho HW, o teste em 19:1A91–1AE3 exige destino válido e tipo terminal zero. 19:1ADB chama 20:04D1 no controlador +18A, que grava 43A0 e solicita áudio. |
| 23:0DF3 | Montagem SB pelo slot +0C de 23:37E1, entrada 030F. A montagem atualiza C8E a partir do tema atual e seleciona o som via DS:0C92. |
| 28:0D71 | Método 28:0CFE compara estado atual com argumento; coincidência sai em 0D83. Solicitação em 0D71 pertence à mudança para estado 2/4. Escolha de som conforme tipo do bloco, com tabela em DS:10D4. |
| 3:2708 | Entrada 3:26E5 toca 4156 incondicionalmente no início do método, a partir da referência de som embutida +A0. A base de atividade cria esse painel de falha em 5:198F → 3:24DE, que instala 3:2B0A. O método de montagem é o slot +10 em 2B1A e a interface secundária em 2B57 encaminha também a 26E5. A base chama a apresentação de falha por 5:1B5E, conforme relatório 18. |
| 5:202B | A cena de dificuldade 14 constrói objeto em 5:1E47 e instala vtable 5:24C6. Seu slot +0C aponta para o método 5:1F20, que chega a 200A após resolver o visual adequado pelo seletor em CS:23DD. A referência em +9A construída com 4137 é entregue ao player +68 na abertura. |
| 9:1AFC | O painel constrói em 09:0999 comando de seletor 9 via 1EDA e registra a área (42,99,53,111) em 0A10 por 32:3735. O executor 1F2E (ref. 21F3, slot +8 de 21EB) passa comando+26/+28 ao despachante 145E. A entrada 9 da tabela CS:1B91 aponta 1AD7, que cancela player +22 e toca 4300. O prólogo real 145E foi absorvido pela última instrução ilusória da tabela na listagem linear. A âncora inicial 12D3 estava errada para este site. |
| 10:15E2 | A montagem CT cria callbacks para os sete elementos da composição em 10:2549 → 2FE6. A classe instala vtable 3330 (ref. em 301A), com slot +8 → 3047. Esse executor exige motor+426 igual a zero. Com o motor livre, passa os dois ponteiros de +2A/+26 a 15A5 em 306A, que usa o player +46E. |
| 15:0B90 | A cabine constrói o controlador por 12:02EB → 15:09D4. O slot +14 em 15:11C3 da vtable 11AF entra em 0B33 e despacha o estado +42 pela tabela CS:0CC9. A entrada 0 da tabela aponta 0B51. 12:030E chama 15:1001 na montagem inicial e 15:104E grava +42=0. Som é selecionado por +76, desde que não haja o mesmo recurso +78 já ativo em +5C. Cancela o anterior, solicita com parâmetro 4E20 e atualiza o estado. |
| 15:0C00 | O estado 1 do controlador aponta o ramo 0BA6 pela tabela CS:0CC9. O setter 15:1081 grava +42=1 quando seu argumento é não zero em 10F2–10FB. O seletor 12:308E(objeto+8C) alterna 4371/402E. Se o som selecionado já é o atual e +5C não é zero, suprime a chamada. |
| 15:0C8B | O estado 3 aponta 0C3B na tabela CS:0CC9. A cabine passa o controlador +12D a 15:1194 em 12:2F7A. Seu teste +42!=0 escreve 3 em 15:11A4. O ramo seleciona 4372/402F via 12:308E(objeto+8C). O teste em 0C62–0C65 compara com o recurso atual e pula a chamada para evitar repetição. |
| 15:23F3 | Entrada 15:23DC, referenciada no método de 16:3952, toca 402D com parâmetro 1. |
| 15:2D7B | Entrada 15:2D54 referenciada por 16:32B4. Cancela o player da entidade em 2D62 e solicita 4303. |
| 15:2ECB, 15:300D | FL: `4` e `6` chegam a 16:0EDE (slot +18 da tabela 2A86, relocação 2A9E), constroem os comandos 04F9/05C4 e os passam a 32:009A. Seus executores 16:0551/061C enviam seletores 0/1 a 16:35F4. A tabela CS:36CE resolve 0→360B e 1→3633. Se +56∈{1,2} e +42=0, escrevem +54=3/4 em 362A/3650. O tick 16:316C lê +54 e encaminha 3 a 15:2D9C em 31DA/31FB ou 4 a 15:2EDB em 3246/3266. Controladores em 15:2D9C/2EDB. Chegam respectivamente após atualização por 15:273B em 2EB4/2FF6. Pedem sons diferentes para deslocamento de ida e volta, condicionados aos ramos de estado +48. |
| 16:16CE | 16:3574 chama o método de cena FL 1683. A rotina resolve receptor pelo objeto em +B0 e pede recurso 0005 com argumento 1 pelo player em receptor+10. |
| 17:2C2A, 17:2C77 | Método do ator FM em 2BE3, registrado por referência 17:3C1C. Campo +20 ativo seleciona 438C, zero seleciona 438F. Ambos cancelam previamente o player em ator+30. |
| 19:0348 | Método 19:01E3 do controlador de movimento, referência na tabela 19:0A8F. Depois de atualizar as posições de objeto por 15:1DB5 nas condições de BP−6 e BP−8, solicita o som. |
| 25:0B91 | Montagem de conteúdo da cena TC por 25:02DD, referência 25:334C. Solicita o loop 42AA com parâmetro 4E20 após construir/aplicar os objetos e o script 0137. |
| 26:1862 | Atualização DD 26:17F0 lê +FE do objeto em 17F3. Se zero, chama 181F em 1801. O estado do argumento zero usa ramo 1840 e solicita a fanfarra, enquanto estados 1 e 2 seguem ramos separados. |
| 28:13B9 | Atualização WB 28:125B. O ramo 138B trava ação em +A8, configura espera 25 e invoca 2059 antes de pedir 4031 com parâmetro 200. |
| 20:0806 | A atualização 20:0696 só toca 401B se as duas coordenadas +48/+4A igualarem +4C/+4E, após o passo 085E. Chama então 19:1EEE, escreve +42=2 e restaura o destino a partir de DS:0C3C. |
| 23:1DD1 | Auxiliar SB 23:1BFD. Após resolver a célula, se o objeto de animação +3A8 está ausente ou em estado fora de 1, 2, 3, prossegue em 1D9F. A solicitação cancela som anterior e pede 4013 para o player em +396. |
| 28:0631 | Atualizador de bloco WB 28:03D9. Após 0CC9 em 05DE, copia +2E para +1C. Se o estado anterior é 4, grava 2 e pula o site. Nos outros estados desse ramo, marca +26=1/+2C=1, cancela o player em +22 e emite 4011. |

### Corpo 10:139A sem produtor direto identificado

O corpo 10:139A cancela a reprodução em motor+46E, constrói o recurso 4367 com argumento 1 e o solicita em 13D7. A entrada começa após o retorno explícito em 1399, portanto não recebe a continuação ordinária de 137D. As listagens não têm salto ou chamada near ao ponto e o inventário expandido NE não tem relocação para 10:139A. Busca da palavra little-endian 139A em todos os segmentos do executável retorna somente 17:1759, na tabela de tipos de atividade FM do próprio segmento 17. O cruzamento adicional com as 99 jump tables em CS e 274 vtables em `analysis/recomp-cfg.json` (`38-CFG-FUNCOES-VTABLES.md`) também retorna zero entradas apontando para `10:139A`, fechando a classificação como corpo órfão/não alcançável estaticamente.

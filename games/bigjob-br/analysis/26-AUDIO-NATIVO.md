# Audio nativo: auditoria das 85 chamadas

Analise estatica exclusivamente; nenhum original executado e nenhuma compilacao. `native-audio-closure.json` preserva todas as 85 chamadas, janela original e janela ampliada, ancora de funcao, fonte dos argumentos, IDs concretos e nomes do banco.

Contagens: {"resolved_symbolic_interface": 5, "resolved_literal": 61, "resolved_selection": 19}. Resolucao de recurso nao e prova de alcance UI. A coluna owner mantem a funcao concreta: nao atribui segmentos compartilhados inteiros a cenas. Os registros individuais preservam mencoes ja existentes nos relatorios e as provas ainda necessarias.

36:28F7 recebe player farptr em BP+6 e ID WORD em BP+A; a palavra seguinte e parametro. 36:2937 recebe referencia de som farptr em BP+A. Nao interpretar ponteiros como IDs. O banco vem de `id>>14`; indice `id&3FFF`.

## Todas as chamadas

| Site | Funcao local | Status | Recursos ou fonte |
|---|---|---|---|
|2:3540|2:34EE|resolved_symbolic_interface|0000 = BIGJOB.SND[0] DShorn; 4346 = BIGVOX.SND[838] CLlever01|
|2:35A2|2:34EE|resolved_symbolic_interface|0000 = BIGJOB.SND[0] DShorn; 4346 = BIGVOX.SND[838] CLlever01|
|3:07E8|3:079E|resolved_literal|41D9 = BIGVOX.SND[473] ChCantPrintParents|
|3:1DA8|3:1D5A|resolved_literal|41D9 = BIGVOX.SND[473] ChCantPrintParents|
|3:2708|3:26E5|resolved_selection|4156 = BIGVOX.SND[342] TuWantPlayAgain01|
|4:2846|4:2806|resolved_selection|4312 = BIGVOX.SND[786] CHbird01; 4352 = BIGVOX.SND[850] CPsound02; 4351 = BIGVOX.SND[849] CPsound01; 4353 = BIGVOX.SND[851] CPsound03; 4354 = BIGVOX.SND[852] CPsound04; 4355 = BIGVOX.SND[853] CPsound05; 4356 = BIGVOX.SND[854] CPsound06; 4357 = BIGVOX.SND[855] CPsound07; 4358 = BIGVOX.SND[856] CPsound08; 4359 = BIGVOX.SND[857] CPsound09; 435A = BIGVOX.SND[858] CPsound10|
|5:0A3A|5:0A1E|resolved_symbolic_interface|0000 = BIGJOB.SND[0] DShorn; 4169 = BIGVOX.SND[361] TuEarnedTrCard01; 428E = BIGVOX.SND[654] BIGJOBINSTRTHEME; 4018 = BIGVOX.SND[24] ALfire03; 400D = BIGVOX.SND[13] ALclick; 439C = BIGVOX.SND[924] TCcabinetDoorClose; 439D = BIGVOX.SND[925] TCcabinetDoorOpen; 428B = BIGVOX.SND[651] 60RandB|
|5:202B|5:1F20|resolved_selection|4137 = BIGVOX.SND[311] TuWantTryEasyHard|
|6:28A6|6:1C7E|resolved_literal|4304 = BIGVOX.SND[772] CGloop|
|6:2C4F|6:2B81|resolved_selection|41A8 = BIGVOX.SND[424] TuThatsAnotherOne; 41A9 = BIGVOX.SND[425] TuThoseHappyCows; 41A5 = BIGVOX.SND[421] TuCowMadeItToBarn; 41A4 = BIGVOX.SND[420] TuAnotherCowAtBarn|
|6:3728|6:36FF|resolved_literal|41A6 = BIGVOX.SND[422] TuCowsInFlowers|
|7:0EA0|7:0D9A|resolved_selection|42FE = BIGVOX.SND[766] CGbull; 42FF = BIGVOX.SND[767] CGcalf; 4300 = BIGVOX.SND[768] CGcow|
|7:0F75|7:0F3A|resolved_literal|4303 = BIGVOX.SND[771] CGhorn|
|8:2EE9|8:2EAF|resolved_literal|4329 = BIGVOX.SND[809] CHearthBall|
|8:3089|8:2EAF|resolved_literal|432D = BIGVOX.SND[813] CHmoonBall|
|8:334F|8:2EAF|resolved_literal|432A = BIGVOX.SND[810] CHeyeBall|
|8:3B55|8:3B34|resolved_selection|432C = BIGVOX.SND[812] CHhoop|
|8:3B7B|8:3B34|resolved_literal|4311 = BIGVOX.SND[785] CHbasketCheer|
|8:3BCB|8:3B34|resolved_selection|406B = BIGVOX.SND[107] CdExcellent01; 406C = BIGVOX.SND[108] CdExcellent02; 4067 = BIGVOX.SND[103] CdCool01; 4068 = BIGVOX.SND[104] CdCool02; 4080 = BIGVOX.SND[128] CdWayCool; 40E1 = BIGVOX.SND[225] RmBodacious; 40E2 = BIGVOX.SND[226] RmBullsEye; 40E4 = BIGVOX.SND[228] RmCool; 40E6 = BIGVOX.SND[230] RmExcellent; 40E9 = BIGVOX.SND[233] RmHighFive; 40EC = BIGVOX.SND[236] RmOnaRoll; 40F0 = BIGVOX.SND[240] RmScore; 40F6 = BIGVOX.SND[246] RmTurboCharged; 40F7 = BIGVOX.SND[247] RmWayCoolMan; 40F8 = BIGVOX.SND[248] RmWayToGo; 40F9 = BIGVOX.SND[249] RmWellDone; 40FE = BIGVOX.SND[254] RmYoureGoodAtThis; 4311 = BIGVOX.SND[785] CHbasketCheer|
|8:437A|8:4118|resolved_selection|4317 = BIGVOX.SND[791] CHbounceFloorEarth01; 431A = BIGVOX.SND[794] CHbounceFloorMoon; 4316 = BIGVOX.SND[790] CHbounceFloor; 4319 = BIGVOX.SND[793] CHbounceFloorEye; 4318 = BIGVOX.SND[792] CHbounceFloorEarth02|
|8:43E3|8:4118|resolved_selection|431B = BIGVOX.SND[795] CHbounceHoop; 431E = BIGVOX.SND[798] CHbounceHoopMoon; 431D = BIGVOX.SND[797] CHbounceHoopEye|
|8:4459|8:4118|resolved_literal|410B = BIGVOX.SND[267] RmBummer01|
|8:4521|8:4118|resolved_selection|4320 = BIGVOX.SND[800] CHbounceWallEarth; 4322 = BIGVOX.SND[802] CHbounceWallMoon; 431F = BIGVOX.SND[799] CHbounceWall; 4321 = BIGVOX.SND[801] CHbounceWallEye|
|9:1AFC|9:12D3|resolved_literal|4300 = BIGVOX.SND[768] CGcow|
|10:13D7|10:139A|resolved_literal|4367 = BIGVOX.SND[871] CTvehicleStartUp|
|10:1430|10:13F3|resolved_literal|4362 = BIGVOX.SND[866] CTdriveOut|
|10:15E2|10:15A5|resolved_literal|4365 = BIGVOX.SND[869] CTpartInPlace|
|10:2AAC|10:2A71|resolved_literal|4361 = BIGVOX.SND[865] CTconveyorBeltLoop|
|10:2C9B|10:2A71|resolved_literal|4361 = BIGVOX.SND[865] CTconveyorBeltLoop|
|10:2D46|10:2CEC|resolved_literal|40A1 = BIGVOX.SND[161] CdTryAgainLater01|
|11:00AE|11:002E|resolved_literal|4387 = BIGVOX.SND[903] FLloaderForward|
|12:268D|12:2662|resolved_literal|400E = BIGVOX.SND[14] ALcrash01|
|15:0B90|15:0B33|resolved_selection|0001 = BIGJOB.SND[1] DSradioLoop01; 0002 = BIGJOB.SND[2] DSradioLoop02; 0003 = BIGJOB.SND[3] DSradioLoop03|
|15:0C00|15:0B33|resolved_selection|4371 = BIGVOX.SND[881] DSturboLoop; 402E = BIGVOX.SND[46] ALtruckLoop|
|15:0C8B|15:0B33|resolved_selection|4372 = BIGVOX.SND[882] DSturboStart; 402F = BIGVOX.SND[47] ALtruckStartUp|
|15:23F3|15:23DC|resolved_literal|402D = BIGVOX.SND[45] ALtruckBig|
|15:2D7B|15:2D54|resolved_literal|4303 = BIGVOX.SND[771] CGhorn|
|15:2ECB|15:2D9C|resolved_literal|4387 = BIGVOX.SND[903] FLloaderForward|
|15:300D|15:2EDB|resolved_literal|4388 = BIGVOX.SND[904] FLloaderReverse|
|16:16CE|16:1683|resolved_literal|0005 = BIGJOB.SND[5] TuDontHitAnything|
|16:3374|16:316C|resolved_literal|4389 = BIGVOX.SND[905] FLscoopDirt|
|16:33D3|16:316C|resolved_literal|4389 = BIGVOX.SND[905] FLscoopDirt|
|17:1389|17:122E|resolved_literal|4390 = BIGVOX.SND[912] TuOoomph|
|17:1428|17:122E|resolved_literal|4267 = BIGVOX.SND[615] TuRescuePerson|
|17:1479|17:122E|resolved_literal|4266 = BIGVOX.SND[614] TuPersonOutSafely|
|17:15CB|17:122E|resolved_literal|4390 = BIGVOX.SND[912] TuOoomph|
|17:163A|17:122E|resolved_literal|4390 = BIGVOX.SND[912] TuOoomph|
|17:2C2A|17:2BE3|resolved_literal|438C = BIGVOX.SND[908] FMpush01|
|17:2C77|17:2BE3|resolved_literal|438F = BIGVOX.SND[911] FMturonWalk|
|17:2F6F|17:2D9C|resolved_literal|400E = BIGVOX.SND[14] ALcrash01|
|18:185B|18:17BC|resolved_literal|4393 = BIGVOX.SND[915] GWmusicLoop|
|18:198E|18:1868|resolved_literal|4393 = BIGVOX.SND[915] GWmusicLoop|
|18:1BF5|18:1BCB|resolved_literal|4394 = BIGVOX.SND[916] GWrotatingBags|
|18:1E79|18:1E4F|resolved_literal|4394 = BIGVOX.SND[916] GWrotatingBags|
|18:20EB|18:20B2|resolved_literal|4392 = BIGVOX.SND[914] GWbagsSideways|
|18:21E1|18:21A8|resolved_literal|4392 = BIGVOX.SND[914] GWbagsSideways|
|18:273F|18:22A4|resolved_literal|4391 = BIGVOX.SND[913] GWbagsHitBottom|
|18:2A5C|18:2A2A|resolved_literal|400F = BIGVOX.SND[15] ALexplosion01|
|18:2B0C|18:2AE3|resolved_literal|4395 = BIGVOX.SND[917] GWrowGone|
|18:2E52|18:2DBD|resolved_literal|4396 = BIGVOX.SND[918] GWtruckDrivesOff|
|18:3120|18:308F|resolved_literal|400F = BIGVOX.SND[15] ALexplosion01|
|19:0348|19:01E3|resolved_literal|432E = BIGVOX.SND[814] CHmouseSteps|
|20:03B4|20:0329|resolved_selection|4018 = BIGVOX.SND[24] ALfire03; 43A0 = BIGVOX.SND[928] WChoseSpray|
|20:0504|20:04D1|resolved_selection|43A0 = BIGVOX.SND[928] WChoseSpray|
|20:0806|20:0696|resolved_literal|401B = BIGVOX.SND[27] ALhitEgg|
|21:1D2F|21:1C91|resolved_selection|401C = BIGVOX.SND[28] ALhitMetal01; 401D = BIGVOX.SND[29] ALhitMetal02|
|21:29F2|21:2964|resolved_literal|4031 = BIGVOX.SND[49] ALwinch|
|23:0DF3|23:030F|resolved_selection|439B = BIGVOX.SND[923] SBloopFire; 439A = BIGVOX.SND[922] SBloopFarm; 4399 = BIGVOX.SND[921] SBloopConstruction|
|23:1585|23:11EF|resolved_selection|4023 = BIGVOX.SND[35] ALhornShort; 4024 = BIGVOX.SND[36] ALhornShort01; 4025 = BIGVOX.SND[37] ALhornShort02; 4026 = BIGVOX.SND[38] ALhornShort03; 4027 = BIGVOX.SND[39] ALhornShort04|
|23:1DD1|23:1BFD|resolved_literal|4013 = BIGVOX.SND[19] ALexplosion05|
|25:0B91|25:02DD|resolved_literal|42AA = BIGVOX.SND[682] TCloop01d|
|26:051B|26:04D4|resolved_literal|436D = BIGVOX.SND[877] DDtractorCollideShort|
|26:1862|26:181F|resolved_literal|436A = BIGVOX.SND[874] DDfanfareStart|
|28:0631|28:03D9|resolved_literal|4011 = BIGVOX.SND[17] ALexplosion03|
|28:0D71|28:0CFE|resolved_selection|4015 = BIGVOX.SND[21] ALexplosionGlass; 4010 = BIGVOX.SND[16] ALexplosion02; 4012 = BIGVOX.SND[18] ALexplosion04; 4014 = BIGVOX.SND[20] ALexplosion06|
|28:13B9|28:125B|resolved_literal|4031 = BIGVOX.SND[49] ALwinch|
|28:14E0|28:1457|resolved_literal|4031 = BIGVOX.SND[49] ALwinch|
|28:1547|28:1457|resolved_literal|4031 = BIGVOX.SND[49] ALwinch|
|28:160C|28:1457|resolved_literal|4031 = BIGVOX.SND[49] ALwinch|
|28:33B2|28:3367|resolved_literal|43A0 = BIGVOX.SND[928] WChoseSpray|
|29:0C59|29:0BB3|resolved_literal|4018 = BIGVOX.SND[24] ALfire03|
|29:0D7A|29:0C66|resolved_literal|4018 = BIGVOX.SND[24] ALfire03|
|29:1EAB|29:1DC2|resolved_literal|401B = BIGVOX.SND[27] ALhitEgg|
|30:228C|30:224B|resolved_symbolic_interface|MMS sound object supplied BP+0A; player resolved by30:0D87; +51 selects parameter1/7FFF; IDs in441 MMS trigger records|
|36:291E|36:28F7|resolved_symbolic_interface|wrapper36:28F7 constructs36:1710 using BP+0A WORD; player BP+06; forwards parameters BP+0C/+0E/+10|

## Provas de selecao adicionais

- 2:3540: 36:1710(this.+1E); field initially0 at2:2E89; setter2:3454 copies BP+0A to+1E and BP+0C to+1C; concrete caller2:326A packs00034346 (ID4346, otherword3); player this.+20; generic setter remains parameterized.
- 2:35A2: 36:1710(this.+1E); field initially0 at2:2E89; setter2:3454 copies BP+0A to+1E and BP+0C to+1C; concrete caller2:326A packs00034346 (ID4346, otherword3); player this.+20; generic setter remains parameterized.
- 3:2708: embedded sound this.+A0 constructed3:2554-2561 with4156; player this.+6E.
- 4:2846: ctor4:2627 copies BP+A table to+3A and BP+E count to+3E; scheduler4:2528->25E3->2806 random modulo count; callers8:1670 workshop DS048E count8 [4312,4352,4351,4352,4353,4354,4355,4356];9:04E8 CPscene4 DS05B2 count10 [4351..435A];11:0826 CSscene3 DS0636 count10 [4351..435A].
- 5:0A3A: forwarded packed ID BP+0A; remaining parameters BP+0C/+0E/+10; player this.+22; caller2:2463 uses command WORD+26 as ID and WORD+28 as parameter; app211A->+4DA[+566] is the scene receiver, NOT the sound ID. Command ctor2:23D8 copies BP+A/+C; caller6:0294 supplies4169 on first successful progress,12:1CEE supplies0000 horn.5:3254 forwards generic scene+68 unlessFFFF; its direct ctor caller2:095B suppliesFFFF, disabling that branch. Other direct callers9:25DB ID428E credits,17:1B3D ID4018 FM,25:0C9A ID400D TC,25:0CE3/0F55/10D6 IDs439C/439D from36:17DE(embedded+13A/+130), initialized25:003F-0061,26:31B3/3782 ID428B jukebox; all caller windows persisted separately.
- 5:202B: embedded sound this.+9A constructed5:1E94-1EA1 with4137; player this.+68.
- 6:2C4F: DS:043E[random()%4], selection 6:2C33-2C42.
- 7:0EA0: DX switch over selected object WORD0; default4300; 7:0E54-0E7F.
- 8:3B55: sound reference this.+CA; constructors 8:31BD-31DC/3483-34A2 use432C; player this.+74.
- 8:3BCB: DS:0586[2:0012(00110000)] at8:3BA2-3BB4; helper2:0012 is inclusive0..17, so18 entries including4311 atDS05A8. Reached on basket score after first-score flagE8 clears and random inclusive0..100 is less than25; 8:3B8F-3BA0.
- 8:437A: this.+F8 chooses +BA versus +BE; BA constructors8:2F4B,30EB,324F,33B1; BE8:2F91; player this.+42.
- 8:43E3: reference this.+C2; constructor8:2FD7/3131/3295/33F7; player this.+42.
- 8:4521: reference this.+C6; constructor8:301D/3177/32DB/343D; player this.+42.
- 15:0B90: WORD this.+76 initialized3 at15:0A36; overwritten15:0EA3/0F17 by15:0F20: +7A=0/1/2 maps3/2/1, otherwiseFFFF; play guarded by +78 and +5C; nonstandard+7A preservesFFFF hazard.
- 15:0C00: 12:308E(this.+8C) nonzero chooses4371, zero402E; 15:0BA6-0BC3.
- 15:0C8B: 12:308E(this.+8C) nonzero chooses4372, zero402F; 15:0C3B-0C58.
- 20:03B4: WORD this.+4E !=FFFF, this.+5C=0 and player.+18=0; writers20:01D1/02C1=4018,04F0=43A0; parameter500; player this.+4A.
- 20:0504: immediate store this.+4E=43A0 at20:04F0, then push WORD +4E at04FB.
- 21:1D2F: sound object this.+10A or +114 selected at21:1D0F-1D24; initialized 21:0176-0198 by36:1710.
- 23:0DF3: DS:0C92[DS:0C8E]; selector writes23:2741/2749/2751=0/1/2; parameter7FFF; player this.+39A.
- 23:1585: DS:0DDC[event-19], event19..24 checked23:1527-153D; constructed by36:1710.
- 28:0D71: DS:10D4/10D6/10D8/10DA selected by block.+2A; state argument2/4 only; switch28:0D07-0D49; branch default reachesD51 without assigning local (preserved original hazard).
- 30:228C: MMS sound object supplied BP+0A; player resolved by30:0D87; +51 selects parameter1/7FFF; IDs in441 MMS trigger records.
- 36:291E: wrapper36:28F7 constructs36:1710 using BP+0A WORD; player BP+06; forwards parameters BP+0C/+0E/+10.

## Limites que impedem declarar fechamento total

Vínculos confirmados para regressão de compatibilidade:

| Origem | Condição | Chamada |
|---|---|---|
| CS, escolha de cartaz | 11:0921 registra o botão em +42 e o tema em +78. A tarefa 11:002E aguarda +42, troca a imagem e toca o som. Aguarda o player ficar ocioso antes de despachar a troca de tema. | 11:00AE, 4387 |
| WC, disparo | Espaço chega pelo seletor 2 a 29:1DC2. Se o controlador chega ao ramo de recusa do disparo com os eixos fora do estado permitido, solicita o som. | 29:1EAB, 401B |
| DD, dano | A colisão passa pela rotina 26:04D4, que subtrai dano do HP +16. O som precede o teste de morte. | 26:051B, 436D |
| Concessão de recompensa | 6:026C exige proprietário e flag de progresso ainda zerada. 2:242C encaminha ID e parâmetro à cena corrente e 5:0A1E encaminha ao player. | 5:0A3A, 4169 |

Os registros `source_traced_remaining_producers` identificam campos, tabelas ou wrappers, mas ainda requerem enumeracao dos produtores e associacao ao estado da atividade. Mesmo os literais fecham apenas recurso/argumento: o caminho de owner/evento precisa ser demonstrado antes de implementar disparos por atividade. O JSON conserva essa lacuna em cada chamada. `resolved_symbolic_interface` fecha a origem de parametro/objeto e conserva o contrato reutilizavel; IDs listados nesses registros sao concretos conhecidos, sem afirmar que um contrato generico limita seus futuros argumentos a esse conjunto. A ancora de funcao e obtida do prologo local e nao equivale a um nome de classe ou cena.

Reproducao: `python tools/close_native_audio.py`. A rotina apenas le arquivos existentes e escreve estes dois artefatos. Nao altera `catalogue_media.py` nem `media-catalogue.json`.

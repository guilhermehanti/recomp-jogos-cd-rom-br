# Despacho QuickTime no Big Job BR

O cruzamento liga os 17 seletores referenciados estaticamente pelo jogo aos corpos de implementação no instalador QuickTime da ISO. São 12 chamadas diretas ao film runtime e cinco chamadas a componentes. A seleção do componente depende dos módulos registrados e das condições verificadas durante a criação do controlador.

## Despacho direto em QTIM.DLL

A exportação 2, `10:0F90`, aceita `BX < 0x67` e chama um ponteiro far lido de `35:0180 + BX*4`. O offset está no WORD inicial e o segmento vem da relocação NE do segundo WORD. Os corpos observados devolvem o controle com RETF, deixando os argumentos a cargo do jogo. Um seletor fora do limite devolve `DX:AX=FFFFFFFF`.

| Stub do jogo | BX hex | Slot DGROUP | Destino NE | Bytes na pilha | Operação observada |
|---|---:|---:|---|---:|---|
| `1:068A` | 07 | 019C | `10:31E8` | 4 | Liberação de filme |
| `1:06B2` | 0B | 01AC | `10:01E0` | 0 | Inicialização |
| `1:06BC` | 0C | 01B0 | `10:0276` | 0 | Encerramento |
| `1:075C` | 1C | 01F0 | `10:3BAA` | 4 | Resolve o filme em `10:8244` e lê o campo `+10h` quando válido |
| `1:07DE` | 29 | 0224 | `10:3DC2` | 12 | Abre/carrega o filme |
| `1:081A` | 2F | 023C | `10:4108` | 12 | Filme e dois DWORDs. O jogo passa `0x10000` e zero na ordem de empilhamento |
| `1:082E` | 31 | 0244 | `10:4308` | 6 | Alterna o estado do filme segundo o WORD do chamador |
| `1:0856` | 35 | 0254 | `10:4BE2` | 4 | Atualização visual |
| `1:086A` | 37 | 025C | `10:6FE2` | 4 | Libera controlador via `32:0A7C`, Component Manager `AX=0 BX=3` |
| `1:0874` | 38 | 0260 | `10:6DDA` | 14 | Seleciona e inicializa o controlador |
| `1:087E` | 39 | 0264 | `10:4248` | 0 | Rotina sem argumentos usada no ciclo de mídia |
| `1:0888` | 3A | 0268 | `1:0456` | 0 | Rotina sem argumentos usada no ciclo de mídia |

O seletor 38 é chamado em `BIGJOBNG 33:2E8C`. O jogo empilha HWND16 em `2DDA` e DWORD 9 em `2DDE`. A chamada intermediária copia um RECT de oito bytes e limpa seus próprios argumentos. Em `2E84–2E89` entram ponteiro far para o RECT e DWORD zero. O chamador soma `0Eh` ao SP em `2E91`. A função QTIM encaminha sete WORDs em `10:6F99–6FAB`. Da posição mais próxima ao retorno da função para a mais distante, o conteúdo é DWORD zero, ponteiro far de 4 bytes, DWORD 9 e HWND16. Os oito bytes de geometria residem no objeto apontado.

A fábrica examina `play/tcpy` em `10:6E0D–6E23` e pode consultar metadados do componente. O ramo em `10:6F10` examina `NAVG`, e `32:0B5C` usa operação administrativa `AX=0 BX=13h` para abrir uma instância. `10:6FBA` chama `32:0A28`, que usa `AX=1 BX=15h` sobre a instância e retorna o controlador quando a inicialização tem sucesso. O caminho de falha devolve zero.

## Component Manager e controladores instaláveis

Os cinco stubs do jogo carregam `AX=1` e chamam `QTIMCMGR!2`. Em `2:217A`, o gerente seleciona DGROUP 5, valida o WORD baixo do handle contra `5:021E` e compara o WORD alto, rotacionado em oito bits, com o campo `+0Eh` do registro de 16 bytes. O ponteiro far do registro base está em `5:0218`. Para os seletores positivos, substitui o DWORD original da pilha pelo DWORD em `registro+4`, a referência privada do componente. `registro+0` aponta para o descritor e `descritor+18h` contém o callback far. O gerente empilha esse callback e usa RETF para alcançá-lo com o retorno original preservado. Handle inválido em `AX=1` retorna `DX:AX=0` em `2:21D3`.

No QCMC, o descritor em `7:00D6` contém `play/std /appl` e o callback aponta para `3:272C`. Em NAVG, o ponteiro em `3:0030` leva a `2:279C`. Os callbacks validam `0 <= BX < 18h` e leem tabelas de WORDs em `QCMC 3:26F0` e `NAVG 2:2760`.

| Stub do jogo | BX hex | Pilha em bytes | QCMC.QTC | NAVG.QTC | Contrato observado |
|---|---:|---:|---|---|---|
| `1:0A7A` | 01 | 10 | `3:0000 → 4:0000` | `2:01B0` | DWORD de instância, WORD de ação e DWORD de carga |
| `1:0AB2` | 05 | 8 | `3:056A` | `2:0B22` | Copia DWORD de estado para ponteiro far em `BP+10h`. Campo privado `+90h` no QCMC e `+86h` no NAVG |
| `1:0B06` | 0B | 14 | `3:074C` | `2:0CE8` | Rota de mensagem, com WORD da mensagem em `BP+12h`. O jogo chama em `33:374F` e segue à verificação de conclusão em `33:3763` |
| `1:0B76` | 13 | 14 | `3:138C` | `2:156C` | Associa o filme e a janela após validar a referência privada |
| `1:0B84` | 14 | 6 | `3:1722` | `2:18B6` | QCMC examina o WORD em `BP+10h` e o byte privado `+94h`. NAVG valida a referência e retorna zero na rota válida, `00008002` na inválida |

O motor consulta o bit `0x40` no resultado do seletor 05 em `33:36C3–36C7`. Durante a reprodução ativa, o bit limpo faz o motor reconhecer a conclusão. As duas variantes limpam esse bit no estado privado ao receber a ação 8 com taxa zero.

## Ações 8 e 12 usadas pelo jogo

Na operação 01, o código de ação é lido em `BP+10h` e a carga começa em `BP+12h`. QCMC aplica o índice ação menos um na tabela `4:007E` e NAVG faz o mesmo em `2:027C`.

| Ação | QCMC | NAVG | Carga e efeito |
|---|---|---|---|
| 8 | `4:0D76` | `2:04F8` | Taxa de 32 bits em ponto fixo 16.16. O jogo usa zero para parar e `0x10000` para taxa unitária. O bit de atividade é limpo em `4:0D9A` ou `2:050F` no caminho de taxa zero |
| 12 | `4:1102` | `2:05FA` | Ponteiro far para registro de 16 bytes. QCMC copia oito WORDs em `4:1121–1124`. NAVG preenche o DWORD `+8` em `2:061B–061F`, verifica o primeiro DWORD ante o limite do filme obtido em `2:3674` e devolve `00008002` na rota fora de faixa. As duas variantes válidas chegam à exportação 523 de QTIM, `12:1C42` |

Em `33:31C1` o jogo prepara os quatro DWORDs da ação 12 com zero, zero, o retorno de QTIM/1C e zero. A exportação 523 resolve o filme, zera seu campo `+1BCh` e passa o primeiro DWORD do registro à rotina de posicionamento `QTIM 10:8028` em `12:1C70–1C80`.

## Reprodução da evidência

`tools/inspect_qt_payload.py` lê o instalador da ISO a partir da imagem original. `tools/inspect_qt_dispatch.py` gera as listagens e os mapas de exportações em `build/qt-analysis`. No fluxo de inspeção, os bytes de código são empacotados em contêineres COFF temporários para desassemblagem de 16 bits. As anotações `NE` ao lado das instruções vêm dos registros de relocação. Quando a relocação cobre apenas o seletor de segmento, o offset efetivo permanece no operando WORD da própria instrução. Os offsets dos módulos dentro do instalador são `0x6DB00` para QTIM, `0x4AB00` para QCMC e `0x54000` para NAVG.

`analysis/30-VIDEO-FRONTEIRA.md` registra os chamadores no motor e `analysis/host-media-contracts.json` lista as referências estáticas. Os 17 destinos estão demonstrados para os binários encontrados. O componente efetivamente escolhido em cada ambiente e o comportamento da reprodução sob aquele ambiente dependem da instalação e da execução.


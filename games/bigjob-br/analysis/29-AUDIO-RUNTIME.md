# Áudio: dispositivo, mistura, filas e duração

Análise estática do executável brasileiro, sem reprodução ou conversão. Complementa a resolução dos85 sites em26. As instruções relevantes estão preservadas em `host-media-contracts.json`, gerado por `tools/trace_host_media.py`.

## Dispositivo efetivamente aberto

`31:0BF6 ->39:008E` inicializa o dispositivo. `39:011C-0154` constrói o formato PCM: tag1, um canal,22050 amostras/s,22050 bytes/s, alinhamento1 e8bits. Usa WAVE_MAPPER (-1), callback0, instância0 e flags0. **Não há callback waveOut nessa abertura:** a conclusão dos buffers é consultada pelo flag WHDR_DONE. Não inferir MCI, MIDI, DirectSound ou uma DLL proprietária de áudio a partir das extensões.

Aloca0x6180 bytes:12 unidades de0x820, cada uma com WAVEHDR Win16 de32bytes e2048bytes de áudio. Zera o cabeçalho, grava o ponteiro dos dados em+0 e a capacidade em+4. A tabela de12 ponteiros fica em DS:2338..2367; índices de produção/consumo em2334/2336; contador de bytes enviados em2368. O handle do dispositivo fica em1324. A preparação usa MMSYSTEM!406, com tamanho explícito32. Não passar esse layout diretamente a WAVEHDR x64.

Falha de alocação retorna0. Falha de abertura zera1324, libera a memória e retorna0. Falha de preparação fecha/libera e retorna0. O original não demonstra recuperação de todas as preparações anteriores nesse ramo; uma implementação nova deve controlar cada buffer preparado para evitar liberar memória ainda retida pelo host.

## Produção e consumo

`39:026B` recebe um horizonte de tempo DWORD. Primeiro percorre os buffers enviados cujo flag+10 tem bit1, subtrai seu comprimento do total2368 e avança consumo módulo12. Calcula a demanda por `horizonte*22050/1000 - bytes_em_fila`; quando há buffers pendentes acrescenta2048 ao alvo. Enquanto houver demanda e posição livre, chama `39:24F2`, grava o comprimento retornado em WAVEHDR+4 e chama waveOutWrite (!408). Retorno do mixer<=0 ou erro do dispositivo encerra a tentativa. Um anel mantém posição vazia para distinguir cheio/vazio:12 posições físicas não significam12 pendentes simultâneas.

Os produtores incluem `36:254B`, `36:3850`, controladores do segmento37 e impressão34:1A01. Portanto a alimentação é cooperativa: remover os bombeamentos durante esperas/filmes pode causar silêncio ou bloquear scripts, mesmo que os samples estejam corretos. Os valores passados são horizontes, não IDs de som.

## Três canais e oito páginas por canal

`39:0380` procura canal0..2 livre em DS:222C. Marca-o e zera os índices2230/2236/223C; guarda o canal em objeto+64. `39:03C2` libera o canal e gravaFFFF no objeto. A disponibilidade deve ser preservada; não criar canais ilimitados por conveniência.

`39:043D` enfileira uma página. Calcula próximo produtor módulo8 e rejeita se alcançar o consumidor. Resolve o sample com36:18E6, guarda seletor em2242, endereço corrente em2272 e quantidade restante em22D2, considerando índice de canal e página. O comprimento é alinhado para baixo a múltiplo de4. Guarda referência de recurso no objeto para manter os dados vivos. São8 posições físicas e até7 pendentes por canal.

`39:24F2` coleta até3 canais com página disponível. Escolhe o menor restante, limitado a2048; avança ponteiros, diminui restantes e, ao acabar uma página, avança consumidor módulo8. Com um canal copia DWORDs; com dois soma bytes unsigned e subtrai128; com três soma e subtrai256. Os ramos269F/26AA e262A/2638 saturam a saída em0..255. Isso demonstra o centro unsigned128 do mixer e a mistura com saturação. Não é mistura por média.

`39:05FD` libera referências das páginas consumidas por36:191C e35:166D. `39:0585` consulta/descarta páginas pendentes, preservando endereços de retomada. A tabela virtual36:3C5A liga esses três métodos ao player: +4=05FD, +8=0585, +C=043D. Esse vínculo é uma tabela de métodos concreta, não uma atribuição por nome do segmento.

## Encerramento

`39:01DE` solicita waveOutReset (!411), retira os12 cabeçalhos por !407, desbloqueia/libera a memória e fecha o dispositivo por !405, zerando1324. O reset precede o teste de handle no original. Para o host novo, registrar o estado de abertura e garantir que o reset terminou antes de liberar buffers. A rotina é usada pelo encerramento31:0D4B/0D96 e pelo ciclo de vídeo33:3346; portanto a coordenação vídeo/áudio é parte do contrato.

## Cabeçalhos dos962 recursos

`tools/inspect_sound_layout.py` relê6bytes de cada recurso e as tabelas de flags/duração do índice, diretamente na ISO aberta com `rb`. Confere a igualdade entre cabeçalho e índice nas962 entradas. `sound-layout.json` registra todos os resultados. O getter original36:23A7 lê a palavra de flags do índice e usa os2bits inferiores:2 ->22050,3 ->44100, demais ->11025.

| Flags originais | Quantidade | Taxa devolvida pelo getter |
|---|---:|---:|
|0012|960|22050|
|0011|2|11025|

As exceções são BIGVOX[819], `CHtoyCmbCropsGrow`, e BIGVOX[870], `CTplatformSound`. Não uniformizar os metadados dessas duas entradas. O DWORD seguinte foi preservado como `timing_field`: não substituí-lo por uma duração calculada do tamanho. Por exemplo, ALbrushSplash informa292 enquanto os6512bytes restantes, divididos por22050, dão aproximadamente295,3ms. A origem editorial dessa diferença não está demonstrada; os metadados e o consumo original devem ser mantidos separados.

A taxa declarada por esse getter não é a taxa do dispositivo. O caminho real entrega bytes sem reamostragem:39:043D ->36:18E6 ->slot+0C do provedor ->36:23EA ->36:2626/2713. O carregamento36:250D lê a página diretamente do arquivo em `offset_recurso+6+pagina*0xC000`;36:2335 calcula o comprimento a partir dos offsets consecutivos, descontando6bytes.39:24F2 avança um byte por saída em todos os canais encontrados e não recebe flags/taxa. Portanto o caminho Windows analisado envia inclusive os recursos0011 ao dispositivo22050. Isso é comportamento do código original, não autorização para corrigir a velocidade arbitrariamente.

Há um backend de temporização separado:36:32EC calcula milissegundos com comprimento*1000/taxa obtida por36:18B3;36:328B descarta páginas pela hora corrente e36:342E estima posição. O backend com som usa os slots reais39:043D/0585/05FD; o backend de temporização usa36:32EC/342E/32CF. As fábricas36:3AAA e3AFA constroem essas variantes por34C9 e31F7. Tratar silêncio como remoção de toda a máquina de estados quebraria os scripts que aguardam término.

A fronteira estática do dispositivo, as filas e a distinção entre backend audível/temporizado estão demonstradas. A equivalência acústica e o sincronismo com vídeo ainda requerem validação em execução de uma implementação. A seleção/evento das chamadas é tratada separadamente em26.

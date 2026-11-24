# Interpretador MMS e comandos compartilhados

Análise estática de BIGJOBNG.EXE. Nenhum original foi executado. Endereços: segmento NE decimal, offset hexadecimal. W0..W5 designam as seis palavras BE16 do registro no arquivo, exatamente como em mapa-jogo-v3.json.

## Conversão de representação obrigatória

37:0456 lê registros de 12 bytes e, quando a flag +4 do stream está ativa, chama 39:13AB com quantidade = registros × 3 em 37:04B8-04C8. 39:13C0-13ED troca byte0↔byte3 e byte1↔byte2 de cada DWORD. Portanto os words da memória são **W1,W0,W3,W2,W5,W4**. Ler a listagem como se houvesse apenas troca de bytes por WORD dá operandos errados.

36:049A retorna o seletor runtime em +0, isto é W1. 36:04F7 retorna runtime+4, isto é W3. 36:0515 retorna DWORD runtime+4, cujo valor numérico é (W2<<16)|W3. 36:0539 copia os oito bytes runtime+4 em diante: words W3,W2,W5,W4. O tipo estrutural W0 e o seletor W1 têm funções distintas.

## Tabela exata do executor 30:1D2E

A tabela literal em 30:1F7B contém nove offsets little endian. Incremento normal do PC é em 30:1F6F-1F72.

| W1 | Destino | Semântica comprovada |
|---|---|---|
| 000C | 30:1F42 → 1F8D | Marco temporal: label=W3; duração signed16=W2; intervalo de loop=W5; repetições=W4. Duração >0 vai a +53 em ms; <=0 vira 7FFFFFFF e liga +69. |
| 000D | 30:1D62 → 1FED | Estado/posição: W3 seleciona objeto visual, W2 participa da escolha do frame (FFFF tem ramo especial), W5/W4 formam coordenadas. Guarda seleção em +4D. |
| 000E | 30:1DBB → 36:0611 → 30:2142 | Resolve referência visual W3 por 34:0036 e aplica estado ao objeto previamente selecionado. |
| 000F | 30:1DED → 21D3 | Seleciona slot de som W3; W2 controla interrupção; W5 fica em +51; W4 configura recurso/estado via36:28CC. Guarda slot em +4F. |
| 0010 | 30:1E34 → 36:05C3 → 224B | Resolve som empacotado W3 via36:1710; reproduz no slot selecionado +4F por36:2937, com repeat=1 ou7FFF segundo +51. |
| 0011 | 30:1E70 | W3 é índice de **vetor de comandos** do proprietário, resolvido por30:11C4; se não nulo executa/enfileira comando por32:009A. Liga +69 e avança PC. |
| 0012 | 30:1EE4 | Salto para label W3 via30:1AE0; não faz incremento normal do PC. |
| 0013 | 30:1F09 → 2299 | Testa slot de som W3. Se campo +18=0, salta para label W2; caso contrário avança normalmente. |
| 0014 | 30:1EBA | Se +69 ainda zero, atribui W3 a +69; depois avança. |

O label de 000C não é a duração. 30:1F93 lê argumento+0C (W2), enquanto30:1B40 compara o primeiro word do payload (W3). Assim a interpretação anterior dos valores0032 como50 ms permanece correta, mas sua posição deve ser nomeada W2.

## Saltos e callbacks

30:1AE0: se label=FFFF, PC recebe a contagem do bloco (fim). Senão PC começa em zero e busca o primeiro registro com opcode000C e W3 igual ao label. Se não encontra, chega à contagem. Tanto o caminho de sucesso quanto de fim passam por30:23A5, que limpa o estado de loop. Não há endereço nativo no operando.

30:11C4 indexa ponteiros far em proprietário+27, passo4. Este vetor contém objetos de comando. Não é o vetor de estados visuais. 32:009A consulta a configuração do comando e o executa por chamada virtual+8 ou o clona/enfileira para execução posterior. O destino nativo depende da construção do slot no código da cena. Logo **C0000011 C0060004 executa slot4**, não C006. O campo C006 não é função far.

30:11E5 é outra associação: substitui o comando de conclusão em proprietário+2B. Não confundir esse único callback final com os slots numerados em +27. Há ainda outra associação em+2F, manipulada por30:1263/138C.

## Referências de som

36:05C3 extrai runtime+4=W3 e chama36:1710. Essa rotina usa índice = ref &3FFF em36:1735, e36:19AD seleciona o banco da tabela global1216 por ref>>14. 36:1A1A registra banco na posição derivada do identificador virtual do próprio banco. Portanto o mapeamento lógico depende dessa construção, não pode ser deduzido só pela ordem dos arquivos.

Exemplo concreto: registro0x4E007 **C0050010 013C41B0** reproduz referência41B0 (banco1, índice432), e não lança animação013C. A antiga associação entre esse registro e bloco013C era resultado da inversão dos words. Não usar esse vínculo como aresta do grafo de cenas.

## Base compartilhada de atividades, progresso e saída

5:14FF guarda o seletor da cena em +66, o ID de progresso em +64 e o modo/variante em +6C. O método5:1657 constrói os controles.

MMS020112 recebe o comando final2:2671 em5:1829. Seu executor2:26CE chama6:026C com ID de progresso, modo1/2 e proprietário. 6:026C grava o estado no vetor de progresso. Na primeira conclusão, reproduz14169 e constrói a recompensa3:0003; nos outros casos executa a continuação do proprietário. Esta cadeia registra sucesso.

MMS020116 recebe o comando final2:25BC em5:18E0. Seu executor2:260B chama5:1B5E, registra tentativa por6:03E2 e apresenta a sobreposição de repetir/sair. A apresentação usa o controlador3:24DE, cuja animação0113 recebe o comando de repetir no slot0 e o de sair ao pai no slot1.

A animação introdutória01018C fornece o comando30:101E guardado em +7C. 5:1A2F o executa para retomar a apresentação. O controle manual de saída é construído em5:0EF1 por3:35A4. Suas tabelas e destinos completos estão em18-NAVEGACAO.md.

## Artefato reproduzível

`python tools/trace_mms.py` gera `analysis/mms-control-flow.json`, com122 scripts de raiz8003, operandos após a conversão DWORD, marcos, saltos, callbacks e441 gatilhos de som. O inventário estrutural completo permanece emmapa-jogo-v3.json, com206blocos fixos e86 tabelas. A verificação contra a ISO cobre56.006 registros porverify_behavior_map.py. A atribuição de um comando numerado ao método nativo usa a instância proprietária e os registros descritos nos relatórios das atividades.

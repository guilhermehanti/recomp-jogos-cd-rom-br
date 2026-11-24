# Cortina da oficina no Big Job BR

Análise estática da ISO brasileira. Os offsets abaixo são relativos ao início de `BIGJOB.MMS`, no LBA 8162. As palavras dos registros estão em big-endian.

## Posição e ordem

O mesmo deslocamento `(x=210, y=119)` alinha cada um dos seis quadros `CHcurtns_0001` a `CHcurtns_0006` com o fundo `CHbackground` de 640 × 480. Para cada quadro, a comparação de sequências horizontais de cinco pixels RGB não transparentes, com pelo menos três cores distintas, encontrou 544 correspondências nesse deslocamento. O segundo melhor deslocamento obteve quatro.

O script confirma o par na ordem `y,x`, com as palavras `0077 00d2` em seis registros separados por 36 bytes. A palavra de sequência logo após o prefixo `c006 000e` cresce de 1 a 6, acompanhando a numeração dos quadros no PIC.

| Quadro no PIC | Registro com posição | Posição em bytes | Bloco seguinte |
|---|---:|---|---|
| 1 | 0x61CB7 | `0077 00d2` | `c006 000e c002 0001` |
| 2 | 0x61CDB | `0077 00d2` | `c006 000e c006 0002` |
| 3 | 0x61CFF | `0077 00d2` | `c006 000e ffff 0003` |
| 4 | 0x61D23 | `0077 00d2` | `c006 000e 0000 0004` |
| 5 | 0x61D47 | `0077 00d2` | `c006 000e 0000 0005` |
| 6 | 0x61D6B | `0077 00d2` | `c006 000e 0013 0006` |

Os seis registros começam com `c002 000d 00ca 0000`. O mesmo par de coordenadas reaparece em 0x61E1F e 0x61E67.

## Tempo e acionamento

O bloco `c002 000c` que antecede cada registro contém, em ordem, os valores `ffff, 0032, 0032, 0032, 0032, ffff`. O valor intermediário é 50 em decimal. A unidade, o significado de `ffff` e a condição de entrada da sequência seguem pendentes. Para confirmá-los, rastrear a interpretação de `000c` até a comparação ou chamada de temporização no motor, e identificar a referência que leva ao início dessa sequência.

A composição PNG anterior usou `(210,115)` como aproximação visual. Use `(210,119)` ao implementar a animação, preservando a ordem 1–6 e o índice transparente de cada quadro.

## Rastreamento estático do relógio

Os endereços abaixo são `segmento NE em decimal:offset em hexadecimal`, da variante `BIGJOBNG.EXE`.

| Endereço | Evidência no executável |
|---|---|
| 38:3018 | Função que devolve o relógio em DX:AX. A instrução far call em 38:301E tem fixup em 38:301F para `MMSYSTEM`, ordinal 607 (`timeGetTime`). |
| 38:3045 | Chama o relógio a partir do laço de mensagens e grava o resultado de 32 bits nos campos 0x12 e 0x14 do objeto. |
| 31:17A0 e 31:17C7 | Chamam o mesmo relógio. Um caminho grava o instante em 0x1A e limpa a flag 0x42. O outro acumula a diferença do relógio no campo 0x16 e liga a flag. |
| 37:21D1–2224 | Consulta o relógio, calcula um prazo a partir do instante base e de uma razão inteira, depois repete a leitura até atingir o prazo. |

O relógio dessas rotinas trabalha em milissegundos. A análise ainda precisa ligar o campo `0032` do registro MMS ao cálculo de prazo correspondente. A existência desse relógio não demonstra que o script use a mesma unidade sem conversão. O tratamento de `FFFF` e a referência de entrada da sequência também seguem sem confirmação.

# Estruturas Win16 observadas nos pontos de chamada

Os endereços deste relatório são do BIGJOBNG.EXE brasileiro. Os argumentos abaixo foram confrontados com as listagens anotadas em `analysis/disassembly`. Os deslocamentos são hexadecimais.

| Estrutura | Evidência de layout no chamador |
|---|---|
| WNDCLASS16 | 38:36C7–3739 zera 13 WORDs e passa o resultado a USER!57. Em 38:3746–378B repete para a segunda classe. Campos +00 estilo WORD, +02 procedimento far, +06 bytes privados da classe, +08 bytes privados da janela, +0A instância, +0C ícone, +0E cursor, +10 pincel, +12 menu far, +16 nome far. O registro reserva quatro bytes privados por janela. |
| MSG16 | 38:3031–303C fornece SS:BP−16 a PeekMessage. O teste de WM_QUIT em 3063 e os testes de teclado/mouse em 309F–30B9 leem a WORD +02. O mesmo ponteiro segue a TranslateAccelerator em 3096, TranslateMessage em 30CA e DispatchMessage em seguida. O layout de 18 bytes na referência Win16 é HWND +00, mensagem +02, WPARAM +04, LPARAM DWORD +06, tempo DWORD +0A e par XY de WORDs em +0E/+10. |
| PAINTSTRUCT16 | 33:377E–37A4 usa o mesmo buffer em SS:BP−26 para BeginPaint e EndPaint. O caminho WM_PAINT valida a região e pede ao QuickTime que pinte entre as duas chamadas. O chamador trata o conteúdo como opaco. O layout de 32 bytes na referência Win16 tem HDC +00, apagar fundo +02, RECT +04, restaurar +0C, atualização incremental +0E, reservados +10–1F. |
| WAVEHDR16 | 39:008E–01C7 e 39:026B–0380 demonstram 32 bytes de cabeçalho, ponteiro far +00, comprimento DWORD +04 e flags DWORD +10. O jogo passa tamanho 32 às funções prepare/write e consulta o bit baixo em +10. |
| WAVEFORMAT | 39:011C–0154 prepara tag PCM, canal mono, taxa 22050, bytes/s 22050, alinhamento 1 e resolução 8 bits para waveOutOpen. Os seis campos ocupam os primeiros 16 bytes. |
| TASKENTRY | 1:05C8 prepara buffer de 40 bytes para ToolHelp. 1:05E1 compara a WORD +0A com o módulo QTNOTIFY e 1:05FA usa a WORD +04 como tarefa destinatária. Ver o fluxo completo em 28. |
| DOCINFO Win16 | 34:2123–212E zera exatamente 10 bytes, grava tamanho 10 na primeira WORD e 34:2140 chama GDI!377 com HDC e ponteiro far para esse bloco. As duas referências far seguintes, nome e saída, ficam nulas. A referência Wine representa também os campos estendidos, inexistentes nesse bloco curto. |
| RECT16 de impressão | Os três sites GDI!38 em 34:22FB, 2512 e 272A usam a mesma saída far em objeto+466. O empilhamento 00030000 fornece código 3 e contagem 0, com entrada nula e saída de retângulo. Os ramos que testam retorno usam o sinal de AX. |
| LOGFONT16 | GDI!57 recebe SS:BP−40 em 37:0EA6 e SS:BP−32 em 38:3AC0, após a cópia do nome terminada por zero. O contrato Wine delimita cinco WORDs assinadas em +00–09, oito BYTEs de atributos em +0A–11 e nome de 32 bytes a partir de +12. |
| LOGPALETTE | 31:1166 e 31:3346 passam ponteiros far para buffers locais. O laço 31:1101–115E avança quatro bytes por entrada e atribui 1 ao quarto byte. 31:32D9–3337 preenche 256 entradas. O cabeçalho Win16 contém duas WORDs (versão e contagem), seguidas por quatro bytes por entrada na ordem RGB/flags. |
| OFSTRUCT | 29:2C81 grava 0x88 como primeiro byte de SS:BP−188, envia o buffer a KERNEL!74 em 2C95 e compara o retorno a FFFF. 22:053E usa um buffer equivalente em SS:BP−128. Ambos os sites usam modo 4000h. |

## Callbacks com evidência local

O procedimento de janela QuickTime usa LPARAM DWORD em BP+06, WPARAM WORD em BP+0A, mensagem WORD em BP+0C e HWND WORD em BP+0E. O caminho 33:37AF-37BC repassa a assinatura a DefWindowProc. O epílogo 33:37C9 remove dez bytes com RETF imediato. 33:3729-372A preserva DS e carrega o segmento de dados fornecido em AX. Se o objeto ativo DS:11B0 for nulo, o procedimento vai direto a DefWindowProc em 3737. Com objeto ativo, 374F entrega os dez bytes e o ponteiro far do controlador a CMgr/0B e 3763 verifica a conclusão antes de interpretar DX:AX retornado pelo despacho. Consumo de mensagem retorna DX:AX zerado em 3770-3774.


* A inicialização 38:367B e 38:36A8 chama MakeProcInstance com 38:3178 e 38:33BC e com a instância de DS:2118. Os ponteiros far retornados ficam em +3C4 e +3C8 e alimentam WNDCLASS16.+02. As janelas precisam de quatro bytes privados para o ponteiro far do objeto proprietário.
* 38:38B0 e 38:3915 passam lpParam far nulo a CreateWindow16. Os respectivos HWNDs ficam em +3CC e +3CE. 38:38D5 e 38:3938 registram o ponteiro far do objeto no índice zero com SetWindowLong. Portanto a entrega do objeto se dá após a criação. Um callback emitido durante CreateWindow deve tolerar esse estado inicial.
* A janela do filme monta o procedimento em 33:2B1C por MakeProcInstance e usa o ponteiro no registro de 33:2B63. O caminho 33:3722–37C1 usa o bloco far de argumentos da janela, trata pintura e encaminha a mensagem para o controlador, quando existente.
* 1:404D–405E enumera janelas da tarefa corrente, fornece o callback 1:4028 e passa SS:BP−02 como LPARAM de saída. O callback termina com retorno far que remove seis bytes em 1:403C. O chamador usa a WORD preenchida para decidir o retorno 2000 ou 1000.

A identidade e os tamanhos das assinaturas dos imports são inventariados em 28. Os layouts USER e GDI usados para completar os campos opacos estão nas definições [winuser16.h](https://github.com/wine-mirror/wine/blob/master/include/wine/winuser16.h) e [wingdi16.h](https://github.com/wine-mirror/wine/blob/master/include/wine/wingdi16.h). A equivalência em execução permanece por validar.

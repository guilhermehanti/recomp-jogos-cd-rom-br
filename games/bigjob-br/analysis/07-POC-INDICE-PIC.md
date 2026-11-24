# Big Job BR — prova de conceito PIC

O leitor em NE segmento 34 verifica MMFW Pictures em 075F, lê a contagem após seek para 0x64 em 07C9 e usa a tabela para buscar o início de um recurso em 1A0E–1A1A. No ramo 146E–149D, carrega a entrada seguinte e subtrai a atual com SUB/SBB, devolvendo o comprimento em DX:AX. A tradução manual em C cobre o bloco 147B–1499. O harness fornece tabela já decodificada, índice limitado e leitura do offset inicial. PCRecomp não foi usado para gerar C nesta prova.

Clang/LLD e SDK do Windows produziram PE AMD64. O teste lê a ISO em modo somente leitura, confere o SHA-256 do NE e os bytes do trecho, cruza todos os 4.744 registros dos dois bancos com inventory.json e verifica quatro limites e dois casos sintéticos. Foram executados somente o teste próprio e o binário x64 gerado.

Em terminal de desenvolvimento x64, a partir da pasta build:

~~~powershell
clang-cl /D_CRT_SECURE_NO_WARNINGS /W4 /WX /O2 /MT ..\src\pic_lookup.c -fuse-ld=lld /Fe:pic_lookup.exe
python ..\tests\check_pic_lookup.py
~~~

O próximo marco técnico é levantar automaticamente uma função inteira ou o ramo que abre o índice, incluindo o acesso far e as chamadas de leitura. A prova atual não executa o loader NE, não licencia um runtime Win16 e não decodifica imagens.

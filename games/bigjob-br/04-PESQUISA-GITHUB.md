# Pesquisa no GitHub

A consulta foi só de leitura. O código abaixo foi lido como referência. Compatibilidade prevista vem de cruzar as rotinas com os bytes da ISO e não de rodar os projetos.

## PCRecomp

- ne_parse lê segmentos, entradas, imports e fixups. ne_decode propaga pontos de entrada, anota relocations e separa destinos far. ne_lift traduz instruções x86-16 para C com estado explícito de CPU, x87, acesso segmentado e chamadas de API via shims. ne_link combina módulos, gera imagem, protótipos, dispatch indireto e stubs para destinos e imports pendentes. A fusão de módulos e a resolução de exports por ordinal exigem validar as entradas e convenções do alvo.
- O runtime Win16 permite montar e ligar um alvo inicial. Um stub linkável não equivale à implementação da API. O gerador precisa conhecer a convenção e os bytes removidos da pilha. A tabela ordinal pode usar metadados do Wine. A cobertura dos 98 fixups OS, callbacks, indiretos e bibliotecas dinâmicas do Big Job precisa ser demonstrada. Integração com APIs modernas ocorre no host e nos shims.
- SimAnt é um caso NE em pré-alpha com Quick Game funcional segundo o próprio projeto, incluindo shims multimídia sob medida. TIM é caso P0 Borland de 27 segmentos de código e escopo de APIs mais estreito. SkiFree é uma reconstrução jogável a partir de C decompilado, com variantes Win16/Win32, e não valida um lift NE genérico do Big Job. Missile Attack é um fixture simples de um segmento. Microsoft Bob e World Empire ilustram alvos que ainda exigem trabalho no host apesar de lift/link.

[PCRecomp](https://github.com/sp00nznet/pcrecomp) · [Pipeline NE](https://github.com/sp00nznet/pcrecomp/tree/main/tools/ne) · [Lifter](https://github.com/sp00nznet/pcrecomp/blob/main/tools/lift/ne_lift.py) · [SimAnt](https://github.com/sp00nznet/simant) · [TIM](https://github.com/sp00nznet/tim) · [SkiFree](https://github.com/sp00nznet/skifree)

## Extratores

MMEX fornece dump bruto por offsets para variantes documentadas de MMFW e VEC. Deark possui conversores para imagens selecionadas, áudio em variantes documentadas, e um reparador de offsets de filmes. Nenhum é um executor de script, reconstrutor confiável ou motor reimplementado. A seção de formatos registra os cruzamentos e os pontos onde o cabeçalho local é rejeitado. O neresex percorre recursos da tabela NE e pode servir de verificação independente para cursores, ícones, grupos e strings. Não levanta código nem interpreta o conteúdo de todos os tipos em formatos prontos de uso.

[MMEX](https://github.com/david47k/mmex) · [Deark mmfw](https://github.com/jsummers/deark/blob/master/modules/mmfw.c) · [Código neresex](https://github.com/david47k/neresex/blob/master/neresex.c)

## WineVDM

O README descreve execução de código Win16 via emulador de CPU e adaptação de APIs Win16 para Win32, incluindo conversão de handles e redirecionamento do diretório Windows. A implementação de MMSYSTEM tem wrappers dos sete ordinais observados. A árvore pública inclui os subsistemas de janelas e vídeo. A disponibilidade de wrappers não garante que o QuickTime antigo, os codecs, paletas ou drivers da edição funcionem. Se autorizado, testar primeiro sem instalação global e sem registrar componentes do CD. A via é provisória e não atende por si só à meta de recompilação.

[WineVDM](https://github.com/otya128/winevdm) · [MMSYSTEM](https://github.com/otya128/winevdm/blob/master/mmsystem/mmsystem.c) · [Configuração](https://github.com/otya128/winevdm/blob/master/otvdm.ini)

## Procura por implementação preexistente

As buscas públicas combinaram Big Job, Tuffa Jobb, ImageBuilder, MMFW, Win16, patch e recompilação. O filtro de issues com “Big Job” em PCRecomp, MMEX, Deark e WineVDM devolveu zero ocorrências. Os resultados encontrados apontam para ferramentas genéricas e outros jogos, sem patch, fork ou release específico verificável para esta edição. A busca não prova inexistência de projetos privados ou não indexados. Nenhuma lista exaustiva de todos os forks e releases foi verificada.


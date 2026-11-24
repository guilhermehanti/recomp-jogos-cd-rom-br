# Formatos e preservação da edição brasileira

| Arquivo | Resultado estático | Reuso e limite |
|---|---|---|
| BIGJOBCD.PIC | MMFW Pictures v0 big-endian, 4.119 entradas | Índice validado. Deark tem layout v0 Pictures. Renderização específica de cada variante ainda por testar. |
| BIGJOBHD.PIC | Mesmo cabeçalho v0, 625 entradas | Primeiro recurso começa com 640×480 e marcador 0x0055. Amostras seguintes contêm 0x0056. Mapear paletas, linhas e máscaras. |
| BIGJOB.SND | MMFW Sounds v0 big-endian, 33 entradas | Banco candidato a efeitos e música. Separação por função ainda por confirmar. |
| BIGVOX.SND | MMFW Sounds v0 big-endian, 929 entradas | Banco candidato principal de vozes. O papel por amostra e a língua precisam de verificação de conteúdo. |
| BIGJOB.MMS | prefixo MMScript File, marcador MM no offset 0x10 | Script binário com referências como cinepak.mov e indeo.mov. Esquema dos registros, opcodes e vínculos exatos ainda pendentes. |
| BIGJOB.DAT | 130.392 bytes e prefixo 4D 4D | Dados binários do jogo. Estrutura e função ainda não estabelecidas. |
| BIGJOB.MOV | Banco proprietário de 58 pares mdat/moov | Cada um anuncia vídeo cvid (Cinepak) e áudio raw. Todos os 12.093 offsets stco apontam para o mdat do clipe correspondente, medidos a partir do início do banco. Extrair um clipe para MOV comum exigirá rebase dos offsets. |
| BIGJOB.EXE / BIGJOBNG.EXE / LEIAME.WRI | Mensagens e documentação em português, recursos NE de strings | Preservar bytes originais e mapear a codificação da interface. CP1252 recupera as mensagens amostradas. |

Os quatro bancos MMFW têm marcador MM em 0x10, versão 0 em 0x12 e comentário em 0x14. A contagem BE16 está em 0x66. Há N+1 offsets BE32 absolutos dentro do arquivo, com último igual ao tamanho. A ferramenta verificou monotonicidade, limites e fim em todos os quatro. O primeiro recurso dos bancos de som começa com 00 12, seguido de um campo BE32 e amostras próximas de 0x80. Isso sugere dados PCM unsigned de 8 bits em parte do banco. Não se atribuiu taxa, canais ou codec aos bancos sem verificar os metadados e a regra do motor. O codec raw listado acima pertence ao MOV.

O MOV começa com uma contagem BE32 e 58 offsets de moov. O primeiro mdat inicia no offset 236, imediatamente após o índice, e o último moov termina exatamente no fim do arquivo. As referências a Indeo no MMS não provam presença desse codec na mídia. Os 58 sample descriptions de vídeo efetivamente lidos são cvid.

## Compatibilidade por código público

- MMEX aceita a assinatura dos PIC/SND e permite offset manual 66 em hexadecimal. O índice validado prevê extração bruta por número. Nomes automáticos podem interpretar metadados como nomes, e não existe recomposição ou decodificação equivalente ao motor. Sua checagem de magic rejeita os cabeçalhos locais de MMS/MOV. MMP, MMB, MMA, MMF e VEC são abrangidos pelo projeto em variantes documentadas, mas não há arquivos com essas extensões na ISO.
- Deark reconhece cabeçalho v0 e oferece layout e decodificadores para Pictures. O código contempla variantes paletizadas, linhas compactadas, máscaras e tentativas de LZSS. Isso não estabelece que cada uma se aplique ao Big Job. O layout Sounds v0 não tem caminho no módulo consultado. O decoder de filmes do Deark já sabe ajustar stco em MMFW Movies reconhecidos, mas o autodetector exige MMFW com assinatura textual ausente no banco MOV local. O MMS local também fica fora da checagem.
- Nenhum decodificador de terceiros foi executado. Nenhum recurso foi convertido. Preserve os contêineres byte a byte durante o port. Uma prova posterior deve referenciar hash da ISO, caminho, índice e faixa de bytes do item, criando testes de equivalência de paleta/áudio ou validação de timestamps antes de substituir qualquer leitor.

[MMEX](https://github.com/david47k/mmex/blob/master/mmex.c) · [Deark](https://github.com/jsummers/deark/blob/master/modules/mmfw.c)


# StreamMax 1.0.7 — Escopo focado na experiência Android TV

**Decisão de produto:** 20/10/2026. **Estado:** planejamento confirmado; implementação, compilação e validação final pendentes.

## Objetivo
Entregar melhorias próprias do StreamMax na TCL P7K e nas TV Boxes sem criar dependência do trabalho de IPTV Xtream. Não prometer liberação do APK antes de concluir as verificações.

## Escopo da 1.0.7 (prioridade)
1. **Pôsteres maiores na Home:** consolidar o ajuste de dimensões já iniciado, preservar proporções, clareza de imagens, foco e desempenho; comparar visualmente antes/depois na TCL.
2. **Renomear fileiras da Home:** permitir renomear as linhas de **filmes e séries**, com teclado da TV, opção de salvar/cancelar e persistência por identificador estável; não mudar o conteúdo dos catálogos. Especificação: [renomear fileiras](streammax-renomear-fileiras-home.md).
3. **Menu Fontes fluido durante reprodução 4K:** validar otimizações de layout e filtragem já feitas. O objetivo é navegar e alternar a fonte sem travar; **não reduzir resolução, bitrate, HDR ou Dolby Vision**.
4. **Regressão e APK:** revisar a integração das mudanças da 1.0.7 e executar verificações de compilação, navegação, reprodução e atualização antes de distribuir o APK.

## Fora da 1.0.7
- **Xtream IPTV (catálogos de filmes e séries, temporadas, episódios e integração ao player):** adiado para a próxima versão ou ciclo após a 1.0.7. O trabalho de API já produzido pode permanecer na branch experimental, mas não é requisito para liberar a 1.0.7; não mesclar trechos incompletos apenas para fechar a versão.
- **Cadastro/instalação/configuração/gerenciamento de add-ons dentro do StreamMax:** planejado para 1.0.8 ou etapa futura, conforme decisão anterior.
- **Compatibilidade Samsung Tizen:** expansão futura, não faz parte deste pacote.

## Condições para considerar a 1.0.7 pronta para teste
- [ ] Pôsteres maiores exibidos com foco, espaçamento e desempenho corretos na Home.
- [ ] Renomear e cancelar nome de fileiras; nome persistir após fechar/reabrir e atualizar catálogos.
- [ ] Em vídeo 4K, menu Fontes abrir e navegar sem travamento relevante; preservar stream e qualidade original.
- [ ] Fluxos existentes de Home, add-ons já instalados, troca de fontes e player sem regressão.
- [ ] Build APK concluído no CI, com artefato e logs; testar na TCL antes de considerar aprovado.

## Estratégia de integração
Trabalhar em branch isolada e usar apenas mudanças da Home e do player necessárias para o escopo. A branch `streammax/107-iptv-startup-metrics` mistura trabalho visual, player e Xtream; **não presumir que a branch inteira possa ser usada como release**. Conferir diff e dependências antes de selecionar commits para a versão final.

## Prazo desejado
O objetivo informado é **20h de 20/10/2026** no horário de São Paulo. Esse horário é uma meta, não garantia de entrega ou de validação na TV.

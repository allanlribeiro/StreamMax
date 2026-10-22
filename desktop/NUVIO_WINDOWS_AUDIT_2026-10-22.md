# Auditoria Nuvio Legacy vs Nuvio Desktop — 22/10/2026

## Fontes consultadas
- Nuvio Legacy (iQui27): https://github.com/iqui27/nuvio-native-legacy
- Releases do Legacy: https://github.com/iqui27/nuvio-native-legacy/releases
- Nuvio Desktop (NuvioMedia): https://github.com/NuvioMedia/NuvioDesktop
- Releases do Desktop: https://github.com/NuvioMedia/NuvioDesktop/releases
- Build Windows Desktop: https://github.com/NuvioMedia/NuvioDesktop/blob/Dev/.github/workflows/desktop-release.yml

## Conclusão de plataforma
**Nuvio Native Legacy (iqui27)** fornece Android TV, LG webOS, Samsung Tizen e VIDAA experimental, mas não oferece um instalador Windows na lista pública de plataformas.
**Nuvio Desktop (NuvioMedia)** é um produto distinto, em **Kotlin Multiplatform + Compose Desktop**, com instalador **MSI para Windows** e player nativo **libmpv**. Em 22/10/2026, o último release consultado era **0.1.31-alpha (17/10/2026)**. O próprio projeto recomenda apenas uso de teste, não uso cotidiano.

Não misturar bases inadvertidamente: Legacy e Nuvio Desktop não são o mesmo repositório; seu backend, modelos, processo de sincronização e fluxos de conta podem diferir.

## Comparação técnica
| Capacidade | Nuvio Desktop | StreamMax Windows (Electron) | Ação |
| --- | --- | --- | --- |
| Layout visual Desktop | Compose Multiplatform | Modern Home inicial em HTML/CSS | Ajustar fidelidade e dimensões |
| Player Windows | Player nativo via libmpv bridge e DLLs | Ainda não integrado | Pesquisar e validar libmpv/MPV nativo e distribuição das DLLs |
| Download/instalador | MSI com runtime e verificação de arquivos | Electron Builder NSIS, depende da arte aprovada | Manter o pipeline atual; adicionar verificações do player depois |
| Seleção de fontes | Fluxo funcional com correções recentes | **Consulta progressiva integrada** via addons do usuário; painel de fontes sem player | **Aplicado** (baseado no comportamento progressivo do Legacy; também útil no Desktop) |
| Tela de carregamento | Etapas e barra visual no player | **Barra de progresso na busca de addons**, não no buffer do vídeo | Parcial; replicar etapas no player quando existir |
| Navegação | Esc, fullscreen, PiP e atalhos Desktop | **Setas 4 direções, Esc fecha detalhes, F11 tela cheia** | Parcial; finalizar atalhos em player e fontes |
| Legendas/áudio | Seleção com correções ISO-639 | Pendente | Separar pt-BR, pt-PT e português desconhecido sem suposições |
| HDR, GPU e hardware decode | Otimizações 0.1.30-alpha, ainda existem reports de travamentos GPU | Sem player | Evitar copiar DLLs/flags cegamente; testar GPU Intel, NVIDIA, AMD |
| Continua assistindo | Sincronização e UI | Pendente | Prioridade seguinte |
| Perfil/conta | Conta Nuvio Desktop | Auth do backend Legacy + perfil primário | Verificar compatibilidade antes de tentar importação de dados |

## Correções estudadas por release
- Desktop **0.1.31-alpha**: barra de buffer, alinhamento da tela de fontes, impedir captura indevida de fullscreen pelo trailer, scrollbar estável.
- Desktop **0.1.30-alpha**: menor buffer e reconexão, perfis rápidos para GPU integrada, preservar shaders e limitar cache, evitar ler arquivos de vídeo na memória ao iniciar.
- Desktop **0.1.28-alpha**: tags ISO 639-2 de idioma do áudio, etapas de carregamento, reset de faixas de áudio e seleção de fontes ao reabrir.
- Desktop **0.1.27-alpha**: estabilidade do destaque, tamanho dos cartões de Continuar Assistindo, botão Esc do player, fullscreen.
- Legacy **v1.7.2**: fontes adicionadas individualmente conforme cada addon responde, sem esperar o mais lento.

## Decisão de arquitetura
**Não substituir o StreamMax Electron existente sem uma avaliação de migração.** O código Kotlin/JVM e o bridge nativo não são plug-and-play no JavaScript. Primeiro reutilizar protocolos e comportamentos; depois decidir pelo empacotamento seguro de player nativo (MPV/libmpv) sem alterar a UI e os dados existentes. Código novo mantido em `feature/streammax-windows-desktop` para preservar Android 1.0.11.

A implementação já adicionada não garante paridade, instalação ou funcionamento do player até os testes Windows completos.

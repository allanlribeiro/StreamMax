# StreamMax para Windows — plano de port nativo

Branch: `streammax/windows-desktop`
Base: `release/streammax-1.0.8-ota-keyboard`

## Objetivo

Criar uma edição **nativa para Windows 10/11 x64**, preservando a identidade do StreamMax e a maior parte do núcleo C compartilhado da versão Legacy.

A versão Windows será separada do APK Android/Android TV. Nenhuma alteração desta branch deve ser publicada como OTA da TV.

## Direção técnica

A base Legacy já possui uma execução desktop para Linux usando SDL2 + GLES/EGL. No Linux, a interface compila e roda, mas o vídeo é um stub de desenvolvimento. Isso torna o núcleo C/SDL o melhor ponto de partida para Windows.

Estratégia:

1. Reaproveitar a UI, catálogo, login, perfis, addons, fontes e navegação do núcleo C.
2. Criar alvo `NV_WINDOWS_DESKTOP` para Windows.
3. Suportar mouse, teclado e controle, mantendo a navegação por setas/Enter/Back.
4. Substituir os stubs de vídeo desktop por um backend real para Windows.
5. Gerar `StreamMax.exe` x64 e um pacote ZIP portátil primeiro; instalador vem depois.
6. Manter dados do usuário em `%LOCALAPPDATA%\\StreamMax`.
7. Não copiar credenciais para logs ou para o pacote.
8. Preservar a edição Android estável sem alterações.

## Player

O Android usa Media3 e não pode ser reutilizado diretamente no Windows.

Para Windows, o backend recomendado é **libmpv**, porque fornece reprodução de HLS, MP4, MKV, seleção de áudio/legendas, seek e boa cobertura de codecs. A integração deve ficar atrás da interface existente em `video.h`, para que `player.c` continue compartilhado.

O primeiro marco funcional deve validar:

- filme e episódio;
- HLS e arquivo direto;
- play/pause/seek;
- seleção de áudio;
- seleção de legenda;
- retomada;
- tela cheia;
- troca de fonte;
- IPTV Xtream depois que o backend básico estiver estável.

## Interface Windows

Primeira versão:

- janela 16:9 redimensionável;
- modo tela cheia;
- interface azul cinematográfica atual;
- teclado: setas, Enter, Esc/Backspace;
- mouse: foco e clique;
- controle Xbox/compatível: suporte via SDL;
- resolução lógica permanece 1920x1080 e escala para a janela.

## Fases

### W0 — bootstrap
Criar alvo Windows, diretório de dados, janela SDL e CI de compilação.

### W1 — navegação
Login, perfil, Home, detalhes, escolha de fonte e configurações sem player real.

### W2 — player
Integrar libmpv ao contrato de `video.h`.

### W3 — reprodução completa
Áudio, legendas, retomada, episódios, fontes e atalhos.

### W4 — IPTV
Xtream ao vivo, VOD e séries, preservando a separação do catálogo principal.

### W5 — distribuição
ZIP portátil, ícone, versionamento e instalador Windows.

## Critério para chamar de versão Windows

Não chamar de versão pronta enquanto o executável não abrir em Windows 10/11 x64 e pelo menos um filme/episódio não reproduzir com áudio e legenda reais.



## Candidato 0.2.0 — desktop experimental com libmpv (22/10/2026)

Branch de implementacao: `feature/streammax-windows-complete-test-20261022`; revisao: PR #39.

A edicao Windows reutiliza a UI nativa C/SDL e as rotas existentes de
login, perfis, catalogos, Home, fontes e faixas. Nao encapsula APK ou emulador.

- **Mouse:** clique esquerdo para ativar, botao direito para voltar e roda para
  percorrer listas; cursor Windows visivel e hover registrado pelo modulo ponteiro.
- **Teclado:** setas, Enter, Esc/Backspace para voltar, F11 ou Alt+Enter para
  alternar modo tela cheia. A tecla Tab deve ser conferida nos campos de texto.
- **Janela:** inicia a 1280x720, redimensiona e maximiza, com minimo 960x540.
  Viewport GLES2 e coordenadas do mouse sao recalculados.
- **Player:** libmpv nativa para URL/video real; play/pause, seek, volume,
  selecao de audio/legenda e legenda externa encaminhados pelos contratos
  existentes. A imagem chega a um FBO OpenGL ES e e composta sob os furos alfa
  da UI. O recorte do trailer ainda precisa de teste visual.
- **Icone:** o PNG S aprovado em `deploy/app/art/marcas/streammax_icon.png`
  e convertido em ICO multi-resolucao e embutido no EXE/instalador.
- **Privacidade:** arte distribuida por allowlist em `windows-art.sh`, sem
  caches, historico particular ou credenciais.

### Antes de compartilhar este instalador
A CI precisa passar pela compilacao Windows, importacao transitiva de DLLs,
HTTPS com PATH limpo, janela real + primeiro frame, setup e desinstalacao.
Mesmo um CI verde ainda **nao equivale a teste de filme no PC**.
Validar pelo menos um MKV e um MP4 reais, HLS, audio regional, legenda PT-BR,
retomada, troca de fontes e navegacao de teclado/mouse em Windows 10/11 x64.

O player e experimental: nao se deve prometer 4K/HDR/DV, todas as fontes,
DRM ou atualizacoes automaticas ate haver evidencia no Windows.

### Distribuicao de teste
A action `StreamMax Windows experimental` gera o ZIP portatil e o
`StreamMax-Setup-Windows-x64.exe`, ambos associados a execucao de CI.
O instalador e **nao assinado por certificado Authenticode** nesta etapa:
o Windows pode exibir alerta; conferir a origem do arquivo antes de instalar.

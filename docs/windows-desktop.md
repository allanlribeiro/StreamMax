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


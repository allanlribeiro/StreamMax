# StreamMax Windows (protótipo inicial)
Ramo isolado derivado de `feature/streammax-111-static-s-logo` (referência 1.0.11).
Este diretório contém a base inicial de **interface desktop**; **não** é ainda um porte funcional do player/catálogo Android.

## Executar em Windows
Instale Node.js LTS. Nesta pasta execute `npm install` e `npm start`.

## Instalador e atalhos
O `electron-builder` está configurado para um instalador NSIS com atalhos de área de trabalho e Menu Iniciar. **Antes de executar `npm run dist`, exporte o símbolo S oficial da marca para `assets/streammax.ico` em formato ICO multirresolução (16, 24, 32, 48, 64 e 256 px)**. A arte do aplicativo Android deve ser a fonte, sem reinventar a marca. Não há arquivo ICO real incluído neste commit.
O appId e o atalho já estão configurados. É preciso validar a instalação no Windows.

## Próximas integrações
1. Auditar as APIs e dados existentes no Android (login, TMDB, catálogos, addons, reprodução e histórico).
2. Implementar camada desktop apropriada, sem depender de emulação Android.
3. Integrar player compatível com codecs/legendas/fontes usados pelo StreamMax.
4. Testar controle de mouse/teclado, janela/tela cheia, atualização e instalação no Windows.

Evitar transportar credenciais ou chaves privadas do APK para o cliente desktop. Respeitar GPLv3, atribuições e licenças das dependências.

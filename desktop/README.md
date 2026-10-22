# StreamMax para Windows — prévia desktop (em desenvolvimento)

Base derivada da ramificação Android TV `feature/streammax-111-static-s-logo`. **Não é ainda uma versão completa do StreamMax para Windows.**

## Recursos implementados na prévia
- Interface inicial para mouse, teclado, modo janela e tela cheia (F11).
- Player experimental para abrir um vídeo local: o Electron/Chromium **não garante suporte** a todos os formatos (por exemplo, MKV ou codecs não suportados).
- Espaço reproduz/pausa; setas esquerda/direita buscam 5 segundos quando o player está ativo; Esc volta à página inicial.
- Separação do projeto Android TV: nenhuma alteração no APK.
- Preparação para instalador NSIS, ícone no executável, menu Iniciar e atalho na área de trabalho.

## Rodar o protótipo no Windows
Instale Node.js, clone o repositório e entre na pasta `desktop`. Execute:
```powershell
npm install
npm start
```

## Logo S, compilação e distribuição
O ícone **não pode ser substituído por outro símbolo**. O ícone triangular presente no Android não é o S aprovado para esta edição.
Coloque a imagem quadrada oficial do S (PNG de pelo menos 256x256) em:
`desktop/assets/streammax-s.png`

O CI converte essa arte para `desktop/assets/streammax.ico` com `scripts/make_icon.py`, então executa `npm run dist`. A criação do instalador de teste é **condicional à presença da logo oficial**, para impedir que o Windows ganhe um ícone incorreto. Se presente, a execução do Actions tentará produzir o instalador **não assinado** como artefato `StreamMax-Windows-Preview-unsigned`; o resultado precisa ser conferido no próprio Actions e instalado/testado em um PC Windows.

Não há APK Android embutido, emulação, nem reuso direto dos componentes Kotlin/Jetpack Compose.

## Pendente
1. Fornecer/incorporar a arte S aprovada para gerar o ICO e permitir o instalador.
2. Implementar as integrações reais de autenticação, catálogos, addons, histórico, seleção de fontes e legendas.
3. Selecionar solução de player desktop com compatibilidade adequada aos codecs e fontes usados no projeto.
4. Validar instalador, atalhos e janelas no Windows 10/11.
5. Planejar atualizações automáticas e distribuição assinada quando a edição desktop estiver funcional.

Respeitar GPLv3, atribuições e licenças das dependências. Nunca incluir credenciais privadas no cliente.

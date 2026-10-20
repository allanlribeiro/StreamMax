# Integração TCL — base RC1 aprovada

Base imutável: `streammax/rc1-distribuicao-1019` (commit `43a239f0672a091a24dae9c552c76efb7bfca0e0`).
Referência de homologação: APK assinado enviado pelo usuário `Nuvio-2.0.7-android.apk`, confirmado como instalado e funcional na TCL P7K. O nome do APK **não** prova correspondência exata com o commit; verificar manifesto e certificado antes de alegar equivalência.

## Critérios obrigatórios para liberar APK de teste

- [ ] Comparar pacote, versionCode e certificado do APK aprovado com o build novo; preservar APK de referência.
- [ ] Preservar login, perfis, identidade StreamMax, artes e fluxo funcional da RC1.
- [ ] Trailer no hero à direita, autoplay e dissolução gradual nas bordas (sem retângulo aparente); executar `legacy-source/tests/streammax-hero-trailer.sh` e inspecionar visualmente na TCL.
- [ ] Player Media3 como Nuvio original, seleção manual da primeira fonte para título nunca visto, retomada automática posterior.
- [ ] Torrentio TB/debrid: não descartar fontes; apresentar metadados de áudio e legendas sem inferir idioma ausente. Português -> 🇧🇷; inglês -> 🇺🇸.
- [ ] TV ao vivo: categorias/canais à esquerda, logos e prévia com proporção correta, sem crop/zoom.
- [ ] Executar testes de source PT-BR, first source, live TV, branding e playback contracts.
- [ ] Conferir que a nova APK **não** regrediu para interface antiga; comparar capturas reais da TCL.
- [ ] Testar instalação/atualização apenas depois de confirmar certificado e versionCode; para debug usar pacote separado `.strictdebug`.

## Estado

Este arquivo registra critérios; **não representa integração de código, testes executados ou APK compilado**. A PR #15 divergiu da RC1; incorporar mudanças seletivamente, sem merge cego.

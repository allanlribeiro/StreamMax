# StreamMax 1.0.0 — Versão estável para TCL / Google TV

Primeira versão estável do **StreamMax** autorizada pelo proprietário em 20/10/2026, após teste real na TCL P7K.

## Origem verificável (não recompilar)

- **Base de código aprovada:** commit `f7ef5c418600d29c4cbe24ff2d2709af683371b7`.
- **Build assinada aprovada:** [GitHub Actions #39406138155](https://github.com/allanlribeiro/StreamMax/actions/runs/39406138155), resultado `success`, execução n.º **146**.
- **Artefato de origem:** `StreamMax-RC1-Assinada-146`, ID **12256867752**.
- **Arquitetura:** `legacy-source` (Android C/SDL/GLES + Media3), não confundir com a variante Kotlin/fullDebug da branch `dev`.
- **Package:** `com.streammax.tv`, para instalação independente do Nuvio original.
- **Versão interna Android:** `2.0.7` / `versionCode=20007`, derivada do projeto nativo aprovado.
- **Nome público desta distribuição:** **StreamMax 1.0.0**. Não alterar o conteúdo do APK para trocar o nome comercial.

## Qualidade exigida antes da publicação

A publicação estável deve reutilizar **exatamente os bytes** do APK assinado do artefato da execução 146. Não compilar novamente, não mudar arquivos de interface, não assinar novamente.

A automação de publicação deve:
1. Conferir que a execução #39406138155 foi concluída com sucesso **no SHA aprovado**.
2. Baixar somente o artefato `StreamMax-RC1-Assinada-146` dessa execução.
3. Verificar integridade ZIP/APK, `package=com.streammax.tv`, `versionName=2.0.7`, `versionCode=20007`.
4. Verificar a assinatura do APK e comparar o certificado com a RC1 143 anteriormente instalada (a execução #39402888789, artefato `StreamMax-RC1-Assinada-143`).
5. Produzir `StreamMax-1.0.0-android.apk` e arquivo SHA-256.
6. Publicar GitHub Release estável **v1.0.0** apontando para o commit aprovado. Em caso de erro, não publicar.

## Escopo aprovado

- Login StreamMax de aspecto discreto, símbolo S oficial e fundo azul-marinho.
- Escolha de perfis e cabeçalho da TV ao vivo usando a escrita oficial.
- Player Media3, seleção de fontes, áudio, legendas, trailer, TV ao vivo e Continue Assistindo preservados da RC1 aprovada.
- Home, detalhes de filmes e comportamento de reprodução sem outras mudanças nesta finalização.

## Distribuição e atualizações futuras

- Esta primeira versão estável é distribuída pelo GitHub Releases para **TCL Android / Google TV**.
- **A atualização automática dentro do StreamMax ainda não foi homologada**: o verificador herdado do Legacy usa o repositório upstream. Não anunciar atualizações automáticas como funcionais.
- **Para próximas versões**, preservar `com.streammax.tv` e o mesmo certificado de assinatura; elevar monotonicamente o `versionCode` (por exemplo `2.0.8`/20008 no próximo APK), sem redução para 1.0.0/10000.
- A referência de interface encontrada em rede social (cards arredondados e Home mais organizada) permanece apenas **backlog futuro**, não integra esta release.
- Compatibilidade Philips, Samsung/Tizen e TV Boxes não está homologada por este teste na TCL.

Esta documentação de release não altera o APK, o aplicativo ou os códigos aprovados.

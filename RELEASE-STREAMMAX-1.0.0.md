# StreamMax 1.0.0 — primeira versão estável aprovada (TCL)

## Código congelado

- Commit da APK aprovada: `d1d705512aba58e42400ba71ccc5ad9b5349f748`
- GitHub Actions: **StreamMax APK experimental**, execução **#142** (aprovada em teste na TCL pelo usuário).
- Identificador Android: `com.streammax.tv`.
- Nome comercial da distribuição: **StreamMax 1.0.0**.

## Importante sobre a versão Android

O código aprovado ainda usa o número interno **2.0.7**, derivado de `legacy-source/deploy/app/appinfo.json`. Não reduzir `versionCode` na APK aprovada, pois isso impediria atualização sobre instalações existentes. Na próxima versão, incrementar o código interno de forma monotônica, mantendo a mesma chave de assinatura.

## Funcionalidades aprovadas

- Interface azul cinematográfica.
- Seleção de fontes em painel amplo antes da reprodução e compacto durante o vídeo.
- Destaque azul da fonte selecionada.
- Reprodução, áudio e legendas conforme testes realizados na TCL.

## Distribuição

Distribuir o **artefato original da execução #142**, sem recompilar o código com mudanças adicionais. Conferir assinatura, nome do pacote e arquivo APK antes de disponibilizar publicamente. A criação desta branch/documentação **não publica automaticamente uma GitHub Release** e não substitui o artefato original.

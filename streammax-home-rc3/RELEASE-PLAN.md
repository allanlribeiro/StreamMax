# StreamMax Home — funcional RC3 e plano de assinatura estável

## Funcionalidade preservada
Relato de validação da TV TCL P7K: Home do controle abre diretamente StreamMax Home RC3 sem exibir Google TV. Projeto em `streammax-home-rc3/standalone`. O último experimento funcional usa `HomeRedirectService` com tentativa de KEYCODE_HOME e fallback por evento de janela. **Não alterar essa lógica nas próximas melhorias sem regressão/teste no aparelho.**

A função de acessibilidade é opcional e reversível; preservar Google TV, Projectivy e serviços de sistema.

## Assinatura estável em branch isolada
Branch: `feature/streammax-home-rc4-stable-signing`. Configuração de release pelo workflow `.github/workflows/streammax-home-stable-release.yml`; exige secrets privados:

- `STREAMMAX_HOME_KEYSTORE_B64`: arquivo JKS convertido em base64
- `STREAMMAX_HOME_STORE_PASSWORD`
- `STREAMMAX_HOME_KEY_ALIAS`
- `STREAMMAX_HOME_KEY_PASSWORD`

Esses secrets precisam ser cadastrados pelo administrador em Settings > Secrets and variables > Actions. **Nunca publicar JKS, senhas, valores dos secrets ou conteúdo de chaves em chat, commits ou artifacts**. Faça uma cópia offline segura do JKS e suas senhas: perder a chave impede atualizar futuras versões com o mesmo applicationId.

O workflow falha intencionalmente se qualquer secret estiver ausente; não gera release com assinatura aleatória.

## Migração da debug RC3 já instalada
A RC3 atual foi assinada com chave debug diferente em execuções distintas. Uma nova chave de release **não atualizará por cima** dessa instalação. A primeira migração exigirá desinstalar **somente** `com.streammax.home.rc3`, com consentimento e após desabilitar sua acessibilidade, verificando os usuários da TV (0 e 10). Isso remove preferências locais da RC3. Não desinstalar Google TV ou launcher RC2.

## Antes de anunciar RC4 pronta
- Confirmar conclusão real do workflow de release e obter APK.
- Confirmar assinatura com `apksigner verify --print-certs`, guardar fingerprint publicamente apenas se desejado.
- Testar Home sem flash, navegação, apps, retorno de configurações, reinício da TV e desabilitação de acessibilidade.
- Atualização futura por `adb install -r` deve reutilizar esta mesma chave, applicationId e versionCode crescente.

## Pendente
A UI premium não foi implementada nesta branch. RC4-test identifica por enquanto somente preparação de release, não uma melhoria visual.

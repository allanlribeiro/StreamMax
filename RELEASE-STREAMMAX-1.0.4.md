# StreamMax 1.0.4 — Home em listas premium (TCL Google TV)

Publicação autorizada pelo usuário em 20/10/2026 para **atualização diretamente dentro do StreamMax**, mesmo antes da validação visual na TCL. Caso algo precise de ajuste, a correção será preparada em uma próxima versão. A versão 1.0.3 está preservada como release anterior e a confirmação da instalação permanece a cargo do Android TV.

## O que muda nesta atualização

- **Menu lateral do StreamMax:** painel rotulado com identidade da marca, nomes das áreas e foco ciano discreto; as ações do controle remoto e o menu já existentes continuam em uso.
- **Home em listas, não grade:** fileiras horizontais de filmes e séries preservadas, agora mais compactas e com espaço para mais conteúdo na TV.
- **Destaque cinematográfico compacto:** reorganizados o espaço do trailer, logo, sinopse e ações para abrir espaço às fileiras sem transformar a navegação existente.
- **Pôsteres:** escolha entre **Horizontais** e **Verticais** preservada nas configurações; perfis e preferências continuam no armazenamento atual.
- **Trailer:** permanece no destaque à direita, com o acabamento lateral e degradê inferior progressivo; a área de vídeo acompanha a nova altura do destaque.
- **Versão comercial 1.0.4**, Android `2.0.11` (`versionCode=20011`), superior à `2.0.10` (20010) da 1.0.3.

## Atualização pelo próprio aplicativo

1. Mantenha o StreamMax 1.0.3 instalado na TCL; **não desinstale** nem limpe os dados.
2. Vá a **Configurações → Atualizar o aplicativo → Procurar atualização**.
3. Quando aparecer a **1.0.4**, selecione **Atualizar agora**.
4. Aguarde a verificação do APK e confirme no instalador do Android TV.
5. Confira que a conta e os perfis continuam intactos e teste a aparência da nova Home, a alternância dos formatos de pôsteres, o menu lateral e o trailer.

Se encontrar um problema de aparência, navegação ou desempenho, relate-o para refinarmos a próxima versão. A compilação passou nos testes automatizados, mas isso não substitui a validação real na TCL.

## Procedência e proteção da atualização

- Código exato: commit `e16f26c3ce6b4af30efd245598ef2482ebdb202a`, branch `streammax/home-listas-premium-104-1020`.
- Compilação Android assinada: [RC1 151](https://github.com/allanlribeiro/StreamMax/actions/runs/39426041703), run `39426041703`, resultado `success`, artefato `StreamMax-RC1-Assinada-151` (ID `12266207691`).
- Assinatura de referência da versão anterior: [RC1 150](https://github.com/allanlribeiro/StreamMax/actions/runs/39418890601), run `39418890601`.
- O workflow valida pacote, versão crescente e certificado, publica **o mesmo APK assinado da RC1 151, byte a byte, sem recompilar nem re-assinar** com nome `StreamMax-1.0.4-android.apk`. O arquivo SHA-256 acompanha a release.

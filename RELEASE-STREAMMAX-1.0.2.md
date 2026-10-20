# StreamMax 1.0.2 — teste da atualização dentro da TCL

Esta versão foi publicada para validar a atualização **diretamente no aplicativo StreamMax**, a pedido do proprietário em 20/10/2026. **O mecanismo ainda precisa ser homologado na TCL P7K; não declarar o fluxo de atualização concluído antes do teste.**

## O que muda

- Identificador comercial atualizado de StreamMax **1.0.1** para **1.0.2**.
- Número interno Android passa de **2.0.8 (20008)** para **2.0.9 (20009)**, permitindo a instalação sobre a versão anterior sem desinstalar e sem apagar dados.
- Preservados player, seleção de fontes, áudio, legendas, TV ao vivo, Home, login, perfis e a identidade visual aprovados.
- Atualizador interno consulta o **GitHub Releases oficial da StreamMax** e valida o nome do APK e a integridade SHA-256. Antes de instalar, confere assinatura, pacote e versão mais nova.
- A confirmação final sempre é feita no instalador do Android TV; **nenhuma instalação silenciosa**.

## Instruções para o teste

1. **Mantenha a StreamMax 1.0.1 instalada na TCL. Não instale este APK manualmente primeiro.**
2. Abra StreamMax > Configurações > **Atualizar o aplicativo** (ou Procurar atualização).
3. Confirme que a versão **1.0.2** aparece e que a ação **Atualizar agora** é exibida.
4. Selecione **Atualizar agora** e acompanhe as etapas Baixar, Conferir e Instalar.
5. Caso a TCL peça permissão para instalar aplicativos desta fonte, autorize o StreamMax e repita a ação.
6. Confirme a atualização no instalador do Android TV.
7. Abra o aplicativo atualizado; confira que o login e os dados de acesso, perfis e configurações continuam preservados. Em Configurações > Atualização, deverá exibir **1.0.2** como versão mais recente.
8. Relate qualquer erro de download, verificação, instalação ou dados de acesso.

## Procedência do binário

- **Código exato do APK:** commit `5d5c1abb2282054e2fbdf3e7716257913d3e1762`, branch de teste `streammax/ota-end-to-end-102-1020`.
- **Compilação assinada:** [GitHub Actions RC1 149](https://github.com/allanlribeiro/StreamMax/actions/runs/39413759022), execução `39413759022` (`success`).
- **Artefato fonte:** `StreamMax-RC1-Assinada-149`, ID `12261495067`.
- **Assinatura de referência instalada na TCL:** RC1 148, execução `39410535210`.
- O publicador automático copia os bytes aprovados do APK, verifica assinatura e versão, e apenas renomeia o arquivo externo para `StreamMax-1.0.2-android.apk`; **não recompila nem re-assina**.

## Distribuição

Esta release é visível publicamente e será apontada como a mais recente no GitHub para que o aplicativo 1.0.1 possa detectá-la. É uma **publicação de validação funcional**, não homologação definitiva do recurso de atualização. A versão 1.0.0 original continua disponível como versão histórica.

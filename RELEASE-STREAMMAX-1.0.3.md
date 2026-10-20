# StreamMax 1.0.3 — Home cinematográfica e trailer ajustado (TCL Google TV)

Publicação autorizada em 20/10/2026 para **atualização diretamente no aplicativo StreamMax**. A nova Home ainda não foi homologada visualmente na TCL; o usuário optou por disponibilizar a atualização agora e corrigir quaisquer problemas em futuras versões. Esta release não altera o instalador: a confirmação final continua sendo feita pelo Android TV.

## Alterações da Home

- **Escolha do formato de pôsteres** em Configurações: "Horizontais" ou "Verticais". O índice da configuração anterior é preservado para não perder preferências existentes.
- **Acabamento suave dos cards:** cantos arredondados configuráveis, anel de foco mais discreto (2 px no desenho da Home), expansão de foco reduzida e transições mantidas.
- **Primeira fileira mais visível:** menor deslocamento com o destaque em foco na Home Moderna para TV; layout compacto do telefone permanece independente.
- **Trailer do destaque da Home Moderna na TCL:** recorte vertical maior (de 422 para 558 px no canvas 1920x1080) e início do degradê inferior deslocado de 58% para 75%; dissolução lateral mais delicada.
- **Versão comercial 1.0.3**, versão interna Android `2.0.10` (`versionCode=20010`).

## Instalação pelo próprio aplicativo

1. Mantenha a StreamMax 1.0.2 instalada na TCL.
2. Abra **StreamMax → Configurações → Atualizar o aplicativo → Procurar atualização**.
3. Escolha **Atualizar agora** quando aparecer a 1.0.3.
4. Aguarde download e validação. Autorize o instalador Android quando solicitado.
5. Confira que perfis, login, player e fontes continuam funcionando. Teste a Home e o degradê do trailer; relate problemas para correções nas próximas releases.

## Origem e verificação

- Commit **exato do código compilado**: `4cefb77c8ef0ccc168abc8a16056e9e445ebc14d` na branch `streammax/home-cinematica-trailer-103-1020`.
- GitHub Actions: [RC1 150](https://github.com/allanlribeiro/StreamMax/actions/runs/39418890601), run `39418890601`, resultado `success`.
- Artefato assinado: `StreamMax-RC1-Assinada-150`, ID `12263167309`.
- Base assinada usada para conferência: [RC1 149](https://github.com/allanlribeiro/StreamMax/actions/runs/39413759022), run `39413759022`.
- O workflow de publicação valida o APK, o pacote, a assinatura e a versão Android, copia os bytes intactos e renomeia para `StreamMax-1.0.3-android.apk`, **sem recompilar nem re-assinar**.

**Situação:** compilação aprovada; liberação no canal de atualizações com aceite do usuário; aguardando feedback de aparência e desempenho na TV.

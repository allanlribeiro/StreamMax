# StreamMax - cadastro de addons na TV (experimental)

Base: StreamMax 1.0.11 Android TV aprovada, Legacy C/SDL/Media3.
Branch de desenvolvimento: feature/streammax-addons-cadastro-tv-20261023.
Estado: implementação preliminar com testes offline; NÃO publicada em APK ou OTA.

## Uso previsto

Ajustes > Fontes e addons > Addons > Adicionar addon por URL.
Pressione OK e digite/cole a URL do manifesto pelo teclado da TV ou pelo
recurso de digitação pelo celular. Formatos: https://, http:// e stremio://;
este último vira https://. Uma URL terminada em /manifest.json também funciona.

A entrada aparece na lista e sua capacidade é conferida posteriormente pela
sondagem do manifesto. O app não executa código de addon diretamente: consulta
os endpoints de provedor existentes no aplicativo.

## Persistência e privacidade

- Guardar as URLs em armazenamento privado da TV, separados por conta e perfil.
- As URLs podem conter tokens pessoais. NÃO publicar em issues, commits,
  logs públicos ou assets da build.
- Após sincronizar a conta, mesclar a lista remota com as instalações locais
  sem alterar a lista de addons do servidor ou sobrescrever os já cadastrados.
- Reaproveitar o teclado, navegação pelo controle e lógica existente de addons.
- Nenhuma mudança na assinatura, dados do app oficial, login e motor de vídeo.

## Limitações / pendências

- Não testado em TCL P7K nem TX6.
- Não remove URLs locais nesta primeira implementação; adicionar/validar
  remoção em atualização antes de considerar o gerenciador completo.
- Alternar ativo/inativo dos itens locais ainda pode ser revertido pelo
  sincronismo; precisa de teste e persistência específicos.
- URLs de até 2047 bytes no formulário inicial; a própria base suporta mais
  por outro mecanismo já existente.
- Sem validação remota do manifesto antes de gravar; o teste do manifesto é
  posterior à inclusão.
- Não houve compilação completa, assinatura de APK ou OTA nesta branch.

## Testes automatizados

bash legacy-source/tests/streammax-addon-cadastro-tv.sh

Valida URL, persistência, isolamento de conta/perfil, deduplicação com
addons da conta e ligações da UI. O teste compila o corpo real do módulo de
armazenamento com dependências SDL/login substituídas por dublês, NÃO o APK.

## Antes de lançar

Compilar base Legacy, usar a chave de assinatura já aprovada e package
com.streammax.tv, instalar sobre a 1.0.11 preservando addons, perfis e
configurações. Testar instalação, manifestação das capacidades, atualização
da Home, fontes, reinício, sincronização e navegação pelo controle na TCL e
TX6 antes de publicar atualização automática.

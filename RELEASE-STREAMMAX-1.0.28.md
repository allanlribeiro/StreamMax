# StreamMax 1.0.28

Agora voce pode criar a conta diretamente no StreamMax, sem abrir o Nuvio
primeiro. Na tela de acesso, escolha **Nao tem conta? Criar conta** e preencha
e-mail, senha e confirmacao. As contas existentes continuam usando **Entrar**.

- Login com identidade StreamMax: fundo escuro, marca, tons azuis e formulario
  separado da apresentacao em TVs de tela larga.
- Login e perfis usam a mesma logo atual da abertura, sem o simbolo antigo.
- Nova arte estatica azul para a escolha de perfil. As outras opcoes continuam
  disponiveis em Ajustes > Tela inicial > Escolha de perfil.
- Cadastro com entrada automatica quando o servidor devolve uma sessao. Se o
  servidor exigir confirmacao por e-mail, o aplicativo orienta a confirmacao.
- Avisos em portugues para senha recusada, e-mail invalido, conta existente,
  limite de tentativas e falha de conexao.
- Cadastro sem repeticao automatica do envio. Em falha de rede, orienta tentar
  entrar antes de cadastrar de novo; senha nao e gravada nem enviada ao log.

**Validacao:** testes automatizados do codigo real de sessao, persistencia,
controle, ponteiro, teclado e limites do formulario; configuracao publica do
servidor consultada. Sem criacao de conta real nos testes. Cadastro ponta a
ponta e apresentacao na TCL ainda precisam de teste no aparelho.

APK Android TV: `StreamMax-1.0.28-android.apk`, pacote `com.streammax.tv`,
codigo 20035. Assinado com a mesma chave para atualizar a instalacao existente.
O instalador do Android pede a confirmacao habitual da atualizacao.

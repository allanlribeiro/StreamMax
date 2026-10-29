# Cadastro e identidade StreamMax — 1.0.28

Na tela de acesso, **Nao tem conta? Criar conta** abre e-mail, senha e
confirmacao. **Ja tenho conta** ou Voltar retorna ao login e apaga as senhas.
As contas continuam no servidor Nuvio; nao existe um segundo banco de contas.

Na TV, o login passa a ter marca e apresentacao a esquerda, formulario a direita,
fundo escuro e tons azuis. Nas telas menores o formulario fica centralizado.
A navegacao por controle remoto e o teclado existente sao preservados.

A escolha de perfil recebe [uma nova arte azul estatica](../../legacy-source/deploy/app/art/marcas/streammax_profiles.png),
criada para manter o centro livre e legivel. Ela e carregada no cache de imagens,
sem video, particulas ou mural de capas neste modo. A opcao **StreamMax** e
selecionada uma vez nesta atualizacao; as cinco opcoes anteriores continuam
nos Ajustes e escolhas posteriores sao mantidas.

## Validacao

- `streammax-signup.sh`: compila sessao.c completo com JSON/scheduler reais e
  rede/disco controlados. Testa cadastro, login existente, persistencia/restauro,
  respostas vazias/truncadas, confirmacao, conta duplicada, senha fraca, e-mail,
  limite, captcha, falhas de rede, clique duplicado e limpeza dos buffers.
- Os logs da fixture sao auditados contra e-mail/senha/tokens sinteticos.
- A UI real e executada com desenho/teclado/sessao controlados: D-pad, ponteiro,
  teclado sequencial, voltar/trocar modo, senhas divergentes e formularios/avisos
  nos limites da tela em escalas 85-150%. ASan/UBSan.
- Renderizador e testes da Home Profissional, incluindo indices/preferencias;
  regressao de addons, VOD, guia, Watch Next, ausencia de frames, identidade,
  isolamento Android, contratos de reproducao e atualizador.
- Servidor real: somente GET da configuracao publica. Cadastro e e-mail estao
  habilitados e o servidor informa confirmacao automatica nesta consulta.

**Limites:** nao foi criada conta real nem feito cadastro/login ponta a ponta;
sem dispositivo TCL conectado, navegacao completa, visual e desempenho fisico
permanecem pendentes. Testes locais e compilacao nao substituem esse teste.

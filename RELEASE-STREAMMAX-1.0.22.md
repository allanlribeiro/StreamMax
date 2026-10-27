O trailer do destaque foi ajustado tomando como referência o Nuvio original: ocupa a mesma área da arte, com preenchimento proporcional e as mesmas paradas do degradê nas bordas esquerda e inferior. A área não encolhe mais quando o trailer tem proporção quadrada ou vertical.

A transição da foto para o vídeo dura 480 ms e só começa após um quadro real. O degradê passa para a cor do fundo, eliminando a mistura entre a foto parada e o trailer. O modo de animações reduzidas continua respeitado.

Preservados os formatos da Home, a preferência de fundo em tela cheia, as fileiras, a correção de rolagem da 1.0.21, o player de filmes e episódios, a prioridade de áudio/legendas em português e a retirada dos frames de avanço.

**Atualizar pelo aplicativo:** Ajustes → Sobre e ajuda → Procurar atualização → Atualizar agora. Confirme a instalação quando o Android solicitar.

Pacote `com.streammax.tv`, versão 1.0.22, código 20029 e assinatura oficial. Atualiza sobre a versão anterior preservando os dados do aplicativo.

Validação: geometria real isolada, primeiro quadro, proporção, transição, regressões da Home/player e composição GLES executada sem janela. O shader e a mistura foram comparados às paradas do Nuvio original em 273.920 valores de canais de pixels. A execução na TCL P7K ainda precisa ser conferida; o teste gráfico não mede FPS na TV.

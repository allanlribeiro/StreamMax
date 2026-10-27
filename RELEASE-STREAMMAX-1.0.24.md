Corrigida uma falha na abertura do trailer da Home: o contexto e a janela de preenchimento agora acompanham a URL antes de o Android preparar a primeira imagem. Na versão anterior, a abertura começava com encaixe e tentava corrigir o quadro depois.

O enquadramento também fica guardado durante a liberação assíncrona do player anterior e é aplicado antes de anexar a superfície de vídeo. Pedidos de fontes anteriores são descartados por geração. O degradê e a disposição da Home da 1.0.23 foram mantidos.

**Atualizar pelo aplicativo:** Ajustes → Sobre e ajuda → Procurar atualização → Atualizar agora. Confirme a instalação quando o Android solicitar.

Pacote `com.streammax.tv`, versão 1.0.24, código 20031 e assinatura oficial. Instala sobre a versão anterior mantendo os dados do aplicativo.

Validação: teste do `trailer.c` completo reproduziu FIT como primeiro comando na 1.0.23; o candidato envia COVER desde a abertura fria/quente e durante atualizações de metadata. Testados fechamento, falhas, destinos cartão/detalhes, retenção da janela e descarte de gerações antigas. A nova ponte JNI é conferida no DEX e nas duas bibliotecas nativas do APK. A confirmação visual na TCL P7K ainda depende de teste físico; os testes de host não emulam o compositor da TV.

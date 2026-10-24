# StreamMax Legacy 1.0.15 — trailers e menu

Base: candidato 1.0.14 (b1ebc1e), app `com.streammax.tv`, Android code 20022.

- Destaque no canto superior direito, proporção original, sem zoom/cover de filmes.
- Máscara alfa com transição nos 16% à esquerda e nos 14% inferiores. Centro, topo e direita sem véu. Backdrop preservado para a transição não formar um retângulo preto. Mesma geometria para máscara e SurfaceView.
- Consulta Apple no catálogo brasileiro; cache próprio `-apple-br-v2` invalida o antigo resultado americano. Uma variante fixa até 1080p, sem ABR nem blur adicional.
- O master reduzido escolhe áudio pt-BR, depois português e depois o áudio disponível. Preserva legenda pt-BR/português do grupo associado à variante. Evita audiodescrição e legendas apenas forçadas quando há alternativas regulares.
- Media3 aplica a preferência só ao cache local de trailers brasileiros. Sem áudio português, habilita as legendas portuguesas disponíveis. As legendas do destaque usam a SubtitleView existente, alinhada ao retângulo do vídeo.
- Perfil e Stats retirado do menu, inclusive navegação e prévia de ajustes. Troca de perfil pelo avatar preservada.
- Login por e-mail/senha, addons e logo 1.0.14 preservados.

## Verificação

Testes C reais de proporção (16:9, cinema, quadrado, vertical e preparo), seleção de variantes, prioridade pt-BR/pt-PT/original, fallback com legenda, ausência de grupos e limites de qualidade. Master público real de um trailer de Ruptura consultado na Apple: áudio e legenda pt-BR encontrados e preservados na redução. Gates de addons, abertura, updater, autenticação e isolamento executados. Compilação Android ARM64/ARMv7 e assinatura conferidas pelo workflow antes de disponibilizar o APK.

Ainda requer teste visual/reprodução na TCL. Nem todo título/fonte oferece áudio ou legendas em português: o fallback IMDb não ganha dublagem artificialmente. Não foi acrescentada extração de vídeos do YouTube no Android. Som e escolhas de áudio/legenda dos filmes permanecem independentes. Candidato de teste: não altera o canal estável/OTA 1.0.11.

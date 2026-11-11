# StreamMax 1.0.30 — Preferências entre TVs

Esta versão mantém o StreamMax 1.0.29 como base e adiciona sincronização das preferências pessoais do StreamMax por conta e perfil, para utilizar a mesma experiência em mais de uma Android TV.

### Novidades
- Preferências de aparência, idioma, organização da Home e opções de Continuar Assistindo podem acompanhar o perfil conectado entre TVs compatíveis.
- As preferências específicas do StreamMax usam um registro separado `streammax_tv`, sem substituir as configurações oficiais do Nuvio.
- Ajustes locais de hardware, rede, buffer, decodificador e periféricos continuam exclusivos de cada aparelho.
- O aplicativo continua procurando atualizações no GitHub e permite baixar e instalar o APK pelo próprio StreamMax.

### Instalação e segurança
- Atualização da versão 1.0.29, mantendo o mesmo identificador `com.streammax.tv` e a assinatura oficial. `versionCode`: 20037.
- Instalador do Android exige confirmação; não há instalação silenciosa nem acesso root.
- APK verificado em GitHub Actions: assinatura igual à versão 1.0.29, versão superior, bibliotecas nativas de rede/imagem presentes para as duas arquiteturas.

### Estado dos testes
A compilação, a assinatura e as verificações automatizadas foram aprovadas. A sincronização entre duas TVs, a navegação completa e a instalação na TCL P7K ainda não foram verificadas em aparelho real. Preserve uma cópia de suas configurações importantes antes de atualizar.

**APK para o atualizador integrado:** `StreamMax-1.0.30-android.apk`.

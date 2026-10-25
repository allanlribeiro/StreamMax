# Filmes e séries da lista IPTV

O StreamMax 1.0.18 parte da versão Android estável **1.0.17**,
commit `5dea61bf27c300f32b12b7318dfea8add5b0f224`.

Em **TV ao vivo**, as abas **Ao vivo**, **Filmes** e **Séries** usam o
cadastro Xtream existente: servidor, usuário e senha. As categorias vêm do
provedor. As abas VOD ficam disponíveis quando a consulta identifica o serviço;
erros de rede ou autorização permitem tentar novamente. Uma lista sem categorias
também é consultada para identificar um catálogo disponível.

O visual segue a referência enviada: três colunas, fundo azul profundo,
seleção magenta, categorias à esquerda, lista central e informações à direita.
Filmes abrem o player; séries abrem temporadas e episódios para selecionar.
A busca e o filtro de favoritos atuam na categoria carregada.

As consultas usam um único worker, com descarte de resultados de categorias,
cadastros e perfis antigos. A lista desenha apenas as linhas visíveis e carrega
capas sob demanda. Há um teto de 4.000 itens por categoria; quando houver corte,
a interface informa o total recebido. Navegar por VOD não inicia reprodução.

O ponto de reprodução é associado ao filme/episódio e à conta Xtream. A retomada
reconstrói a fonte com as credenciais atuais, inclusive se a senha mudou.
Os IDs persistidos não contêm a URL de reprodução nem a senha. Os favoritos
pertencem ao perfil e são apagados ao sair da conta do aplicativo.

## Verificação

```sh
cd legacy-source
bash tests/streammax-xtream-vod.sh
bash tests/streammax-livetv-columns.sh
bash tests/streammax-first-source.sh
```

`tests/iptv-vod/fixture.py` e `interface.cjs` exercitam a interface C real
compilada para WASM, com um provedor de teste local e vídeo sintético. A conexão
HTTPS fictícia é interceptada apenas nessa bancada. Os serviços externos não
recebem login nem dados de uma conta real. As imagens usam capas ilustrativas
do pacote para demonstrar o layout, sem afirmar disponibilidade desses títulos.

Os resultados e capturas ficam em `output/iptv-vod/`. Os testes Chromium
verificam navegação e reprodução, sem representar um emulador Android/Samsung
ou medir o desempenho na TCL. O APK Android é compilado separadamente pelo
workflow `streammax-118-iptv-vod-apk.yml`, com assinatura e identidade conferidas.

## Na TCL

1. Instale o APK de teste como atualização do StreamMax existente.
2. Confira o cadastro em Ajustes e abra **TV ao vivo**.
3. Aguarde a consulta VOD e selecione **Filmes** ou **Séries**.
4. Teste categorias, busca, favoritos, temporadas, reprodução e retorno ao guia.
5. Confira pausa, busca no vídeo, áudio e legendas com as mídias do seu provedor.

O funcionamento e os formatos disponíveis dependem da lista e do aparelho.
A publicação `v1.0.18` usa o APK já conferido da prévia e o disponibiliza pelo
atualizador do aplicativo. O Android/Google TV solicita confirmação para instalar.
Em Ajustes → Sobre e ajuda → Procurar atualização é possível consultar na hora.

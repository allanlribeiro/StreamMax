#!/usr/bin/env bash
# Empacotamento de arte segura para o Windows: nunca exportar perfis,
# colecoes/cache, tokens, URLs autenticadas ou ajustes privados.
set -euo pipefail
cd "$(dirname "$0")/.."
dest="${1:-build/windows/package/art}"
src="deploy/app/art"
mkdir -p "$dest"
for dir in icones icones-app badges selos marcas prov editorial logo poster ep elenco texturas; do
  if [[ -d "$src/$dir" ]]; then cp -r "$src/$dir" "$dest/"; fi
done
cp "$src"/*.jpg "$dest/" 2>/dev/null || true
test -s "$src/discord-ca.pem" || { echo "CA publica obrigatoria ausente" >&2; exit 1; }
cp "$src/discord-ca.pem" "$dest/discord-ca.pem"
test -s "$src/marcas/streammax_icon.png" || { echo "S oficial ausente" >&2; exit 1; }
# Nenhuma pasta gerada em runtime (dados pessoais/cache/catálogo particular).
for dir in collections cache cinematic dados atualizacao; do
  test ! -e "$dest/$dir" || { echo "Arte privada ou demasiada: $dir" >&2; exit 1; }
done
# Bloquear qualquer arte que contenha dados privados camuflados.
if find "$dest" -type f \( -iname '*.txt' -o -iname '*.bin' -o -iname '*.tmp' -o -iname '*.jsonl' \) | grep -q .; then
  echo "Arquivo potencialmente privado em arte distribuivel" >&2
  exit 1
fi
echo "Windows: pacote de arte selecionada, sem dados pessoais."

#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
out="$(mktemp)"
trap 'rm -f "$out"' EXIT
${CC:-cc} -std=c11 -Wall -Wextra -Werror -pedantic \
  tests/test_streammax_next_prefetch.c -o "$out"
"$out"
# Teste de integracao estatica: o fluxo VOD nao pode herdar autoplay Legacy.
python3 - <<'PY'
from pathlib import Path
source = Path("src/posplay.c").read_text()
segment = source.split("static void prepararProximo(int ehSerie, double resta) {", 1)[1].split("\n}\n", 1)[0]
assert "const int manual = 1;" in segment, "VOD deve exigir escolha manual antes de prefetch"
assert "streammax_next_video_preload(preloadVideo, manual)" in segment
assert "streammax_next_metadata_prefetch(" in segment
print("OK: prefetch VOD respeita escolha manual do StreamMax")
PY
echo "OK: prefetch da lista do proximo episodio preserva escolha manual"

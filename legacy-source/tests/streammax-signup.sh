#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
tmp=$(mktemp -d)
trap 'rm -rf "$tmp"' EXIT
flags=()
if [ "${SM_SANITIZERS:-0}" = 1 ]; then flags+=(-fsanitize=address,undefined -fno-omit-frame-pointer); fi
${CC:-cc} -std=gnu11 -O1 -g -Wall -Wextra -ffunction-sections -fdata-sections \
  -Isrc "${flags[@]}" tests/test_streammax_signup.c src/js.c src/jsw.c \
  -Wl,--gc-sections -pthread -o "$tmp/signup"
"$tmp/signup" > "$tmp/audit.log"
if rg -q 'cadastro-fixture@|Fixture-Senha|fixture-refresh|e30\.eyJ' "$tmp/audit.log"; then
  echo 'FALHOU: dado sensivel no log de autenticacao' >&2; exit 1
fi
tail -1 "$tmp/audit.log"
${CC:-cc} -std=gnu11 -O1 -g -Wall -Wextra -ffunction-sections -fdata-sections \
  -DNV_ANDROID -DNV_TOUCH_UI -Isrc ${SM_CFLAGS:-$(pkg-config --cflags sdl2)} "${flags[@]}" \
  tests/test_streammax_signup_ui.c -Wl,--gc-sections -lm -o "$tmp/ui"
"$tmp/ui"

python3 - <<'PYCODE'
from pathlib import Path
login=Path('src/login.c').read_text()
assert 'logoapp_streammax_abertura(' in login
assert 'logoapp_streammax_simbolo(' not in login
assert 'logoapp_streammax_wordmark(' not in login
profile=Path('src/perfilsel.c').read_text()
assert 'logoapp_streammax_abertura(' in profile
assert 'logoapp_streammax_wordmark(' not in profile
print('OK: acesso e perfis usam a mesma logo oficial da abertura, sem arte antiga')
PYCODE

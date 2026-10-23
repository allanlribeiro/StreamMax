#!/usr/bin/env bash
# Empacotar cada dependencia UCRT64 exatamente uma vez (libmpv carrega muitas DLLs).
# A busca anterior repetia "find" para cada import de cada DLL em todo ciclo,
# aumentando quadraticamente o tempo de pacote do Windows.
set -euo pipefail
PACKAGE="${1:?informe diretorio de destino}"
UCRT_BIN="${2:-/ucrt64/bin}"
test -s "$PACKAGE/StreamMax.exe" || { echo "EXE ausente no pacote" >&2; exit 1; }
command -v objdump >/dev/null || { echo "objdump indisponivel" >&2; exit 1; }

declare -A paths=()
declare -A packaged=()
declare -A visited=()
queue=()

# Indexar a pasta do MSYS2 uma unica vez, com nome sem diferenca de maiusculas.
while IFS= read -r -d '' item; do
  name="${item##*/}"
  paths["${name,,}"]="$item"
done < <(find "$UCRT_BIN" -maxdepth 1 -type f -iname '*.dll' -print0)

# O sistema operacional fornece estas bibliotecas, nao empacotar duplicatas.
is_system_dll() {
  case "${1,,}" in
    api-ms-win-*|ext-ms-win-*|kernel32.dll|kernelbase.dll|ntdll.dll|ucrtbase.dll|msvcrt.dll|msvcp_win.dll)
      return 0 ;;
  esac
  [ -f "/c/Windows/System32/$1" ] || [ -f "/c/Windows/SysWOW64/$1" ]
}

# Copiar e enfileirar apenas bibliotecas UCRT64 ainda nao vistas.
include_dll() {
  local dll="$1" key="${1,,}" found
  [ -n "$dll" ] || return 0
  if [[ -v 'packaged[$key]' ]]; then return 0; fi
  if is_system_dll "$dll"; then
    packaged["$key"]=system
    return 0
  fi
  found="${paths[$key]:-}"
  if [ -z "$found" ]; then
    echo "ERRO: DLL nao localizada no UCRT64 nem no Windows: $dll" >&2
    exit 1
  fi
  cp "$found" "$PACKAGE/${found##*/}"
  packaged["$key"]="$PACKAGE/${found##*/}"
  queue+=("${packaged[$key]}")
}

required=(
  SDL2.dll SDL2_image.dll SDL2_ttf.dll
  libEGL.dll libGLESv2.dll libdl.dll libcurl-4.dll libmpv-2.dll
)
queue+=("$PACKAGE/StreamMax.exe")
for dll in "${required[@]}"; do include_dll "$dll"; done

# Fila incremental, sem revarrer as DLLs ja verificadas. Inclui transitivas do mpv.
head=0
while (( head < ${#queue[@]} )); do
  binary="${queue[head]}"
  head=$((head + 1))
  if [[ -v 'visited[$binary]' ]]; then continue; fi
  visited["$binary"]=1
  while IFS= read -r dll; do
    include_dll "$dll"
  done < <(objdump -p "$binary" | sed -nE 's/^[[:space:]]*DLL Name:[[:space:]]*([^[:space:]]+).*/\1/p')
done

for dll in "${required[@]}" zlib1.dll; do
  # no Windows os nomes existentes devem ser encontrados ignorando maiusculas
  key="${dll,,}"
  [ -n "${packaged[$key]:-}" ] || { echo "DLL obrigatoria nao encontrada: $dll" >&2; exit 1; }
done
echo "OK: ${#visited[@]} binarios examinados, ${#packaged[@]} dependencias resolvidas."

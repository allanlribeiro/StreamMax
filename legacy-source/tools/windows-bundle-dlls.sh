#!/usr/bin/env bash
# Copia recursivamente DLLs UCRT64 importadas pelo StreamMax e suas dependencias.
# Evita a lista manual incompleta (ex.: zlib1.dll, libpng, harfbuzz, etc.).
set -euo pipefail
PACKAGE="${1:?informe diretorio de destino}"
UCRT_BIN="${2:-/ucrt64/bin}"
test -s "$PACKAGE/StreamMax.exe" || { echo "EXE ausente no pacote" >&2; exit 1; }
command -v objdump >/dev/null || { echo "objdump nao encontrado" >&2; exit 1; }

changed=1
while [ "$changed" = 1 ]; do
  changed=0
  while IFS= read -r binary; do
    imports="$(objdump -p "$binary" | sed -nE 's/^[[:space:]]*DLL Name:[[:space:]]*([^[:space:]]+).*/\1/p')"
    while IFS= read -r dll; do
      [ -n "$dll" ] || continue
      [ -f "$PACKAGE/$dll" ] && continue
      found="$(find "$UCRT_BIN" -maxdepth 1 -type f -iname "$dll" -print -quit)"
      if [ -n "$found" ]; then
        cp "$found" "$PACKAGE/"
        echo "Incluida dependencia: $dll"
        changed=1
        continue
      fi
      # As API sets e as DLLs do Windows ficam no sistema; nao distribuimos.
      dll_lower="$(printf '%s' "$dll" | tr '[:upper:]' '[:lower:]')"
      case "$dll_lower" in
        api-ms-win-*|ext-ms-win-*|kernel32.dll|kernelbase.dll|ntdll.dll|ucrtbase.dll|msvcrt.dll|msvcp_win.dll) continue ;;
      esac
      if [ -f "/c/Windows/System32/$dll" ] || [ -f "/c/Windows/SysWOW64/$dll" ]; then
        continue
      fi
      echo "ERRO: DLL nao localizada (nem UCRT64 nem Windows): $dll importada por $binary" >&2
      exit 1
    done <<< "$imports"
  done < <(find "$PACKAGE" -maxdepth 1 -type f \( -iname '*.exe' -o -iname '*.dll' \))
done
echo "OK: todas as dependencias UCRT64 diretas e transitivas estao no pacote"
find "$PACKAGE" -maxdepth 1 -type f -iname '*.dll' -printf '%f\n' | sort

#!/usr/bin/env bash
# Copia recursivamente DLLs UCRT64 importadas pelo StreamMax e suas dependencias.
# Evita a lista manual incompleta (ex.: zlib1.dll, libpng, harfbuzz, etc.).
set -euo pipefail
PACKAGE="${1:?informe diretorio de destino}"
UCRT_BIN="${2:-/ucrt64/bin}"
test -s "$PACKAGE/StreamMax.exe" || { echo "EXE ausente no pacote" >&2; exit 1; }
command -v objdump >/dev/null || { echo "objdump nao encontrado" >&2; exit 1; }

# Algumas bibliotecas sao carregadas por dlopen/LoadLibrary em tempo de execucao
# e NAO aparecem nos imports de StreamMax.exe.
# Evita erros de sistema com SDL2.dll, libGLESv2.dll e libdl.dll.
required=(SDL2.dll SDL2_image.dll SDL2_ttf.dll libEGL.dll libGLESv2.dll libdl.dll)
for dll in "${required[@]}"; do
  found="$(find "$UCRT_BIN" -maxdepth 1 -type f -iname "$dll" -print -quit)"
  [ -n "$found" ] || {
    echo "ERRO: biblioteca essencial $dll nao encontrada em $UCRT_BIN" >&2
    exit 1
  }
  cp "$found" "$PACKAGE/"
  echo "Incluida DLL essencial: $dll"
done

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
# Guardas finais: SDL, ANGLE, libdl, zlib e fechamento de imports.
for dll in "${required[@]}" zlib1.dll; do
  test -s "$PACKAGE/$dll" || {
    echo "ERRO: pacote sem DLL obrigatoria $dll" >&2
    exit 1
  }
done
echo "OK: DLLs essenciais e fechamento recursivo de dependencias presentes"
find "$PACKAGE" -maxdepth 1 -type f -iname '*.dll' -printf '%f\n' | sort

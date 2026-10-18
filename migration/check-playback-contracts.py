#!/usr/bin/env python3
"""Verificação estática de regressões na retomada; não reproduz vídeo."""
from pathlib import Path
import sys

root = Path(sys.argv[1]) if len(sys.argv) == 2 else Path("legacy-source")
checks = {
    "src/app.c": [
        'fontevolta_esquecer("fonte fora dos filtros atuais")',
        "if (!preparoEspera && tocarFonteGuardada())",
        "stream_folha_abrir();",
    ],
    "src/player.c": [
        "void player_abrir(",
        "armarTituloGuardado(void)",
        "cacheboost_backend_titulo(alvo)",
        "ajustes_retomada_morna()",
    ],
    "android/app/src/main/java/space/nuvio/nativelegacy/NvPlayer.kt": [
        "p.setMediaItem(item, s.inicioMs.toLong())",
        "p.seekTo(d * f / 10000L)",
    ],
    "src/fontevolta.c": [
        "int fontevolta_toca_guardada(",
        "if (idadeLink >= FONTEVOLTA_VALIDADE_MS) return;",
        "strlen(alvo) >= sizeof eAlvo || strlen(conta) >= sizeof eConta",
        "if (escolherManual || falhaAtiva || canal || !playerAberto) return 0;",
    ],
    "src/fontevolta.h": [
        "FONTEVOLTA_PRAZO_MS",
        "FONTE_MANUAL_PRAZO_MS",
        "fontevolta_decidir(",
    ],
    "src/ajustes.c": [
        "int ajustes_cw_estilo(void)",
        "int ajustes_cw_fonte(void)",
    ],
}
errors = []
for name, needles in checks.items():
    path = root / name
    if not path.is_file():
        errors.append(f"Ausente: {name}")
        continue
    text = path.read_text(encoding="utf-8")
    errors += [f"{name}: falta {needle}" for needle in needles if needle not in text]
if errors:
    print("\n".join(errors), file=sys.stderr)
    sys.exit(1)
print("OK: contratos estáticos de retomada, cache e continuar assistindo presentes")
print("PENDENTE: validação funcional com vídeo e login real na TV")

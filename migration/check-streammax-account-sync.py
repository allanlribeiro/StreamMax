#!/usr/bin/env python3
"""Verificacoes de contrato da sincronizacao StreamMax entre Android TVs.

Estaticas/offline: NAO substituem compilacao Android, servidor nem teste em TV.
Uso: python3 migration/check-streammax-account-sync.py legacy-source
"""
from pathlib import Path
import re
import sys


def expect(text: str, needle: str, name: str) -> None:
    if needle not in text:
        raise AssertionError(f"{name}: ausente: {needle}")


def main(root: Path) -> int:
    ajuste = (root / "src/ajustes.c").read_text(encoding="utf-8")
    header = (root / "src/ajustes.h").read_text(encoding="utf-8")
    sync = (root / "src/sync.c").read_text(encoding="utf-8")

    for signature in (
        "char *ajustes_streammax_exportar_blob(void)",
        "int ajustes_streammax_aplicar_blob(const char *json)",
    ):
        expect(ajuste, signature, "ajustes.c")
        expect(header, signature.replace("{", ";"), "ajustes.h")

    for needle in ('"streammax_version"', '"streammax_preferences"',
                   "jsw_ci(&w, CHAVE[i], valor[i])",
                   "strtol(bruto, &resto, 10)",
                   "if (mudou) gravar();"):
        expect(ajuste, needle, "serializacao")

    portability = ajuste.split("static int ajustes_streammax_portatil(int op)", 1)[1]
    portability = portability.split("char *ajustes_streammax_exportar_blob", 1)[0]
    for forbidden in ("case AJ_DV:", "case AJ_ATMOS:", "case AJ_RESOLUCAO:",
                      "case AJ_BUFFER:", "case AJ_TEX_MB:", "case AJ_P2P_DESTINO:",
                      "case AJ_POSTER_TOKEN:", "case AJ_XTREAM_SENHA:"):
        if forbidden in portability:
            raise AssertionError(f"preferencia nao-portatil exposta: {forbidden}")
    for allowed in ("case AJ_TEMA:", "case AJ_HOME_LAYOUT:", "case AJ_CW_LIGADO:",
                    "case AJ_LEG_LINGUA:", "case AJ_AUD_LINGUA:"):
        expect(portability, allowed, "preferencias pessoais")

    for needle in ('#define SY_STREAMMAX_PLATAFORMA "streammax_tv"',
                   'jsw_cs(&w, "p_platform", SY_STREAMMAX_PLATAFORMA);',
                   'rpcDoCiclo("sync_pull_profile_settings_blob"',
                   'rpcDoCiclo("sync_push_profile_settings_blob"',
                   "streammaxPendente(perfil)",
                   "if (!ok2xx(resposta, st) || !perfilDoCicloAtual())",
                   "if (!remotoAusente) { free(resposta); return; }",
                   "streammaxFotografar(0);",
                   "streammaxFotografar(1);",
                   "streammaxConsumirRemoto(perfis_ativo())",
                   "ajustes_streammax_aplicar_blob(fotoStreammax)",
                   "streammaxFotoGeracao",
                   "dados_apagar(nome);"):
        expect(sync, needle, "fluxo protegido")

    # Sem novidade remota, so autorizamos bootstrap em [] valido e HTTP 2xx.
    function = sync.split("static void sincronizarAjustesStreammax(void)", 1)[1]
    function = function.split("static int puxarCatHome", 1)[0]
    for needle in ("remotoAusente", "remotoExiste", "tvSettingsPullOk",
                   "if (remotoExiste && !pendente)", "return;", "stPush"):
        expect(function, needle, "bootstrap conservador")
    if re.search(r'printf\s*\([^;]*tokenDoCiclo', function):
        raise AssertionError("token nao deve aparecer em log")
    print("OK: contratos estaticos de sincronizacao StreamMax entre TVs")
    return 0


if __name__ == "__main__":
    raise SystemExit(main(Path(sys.argv[1] if len(sys.argv) > 1 else "legacy-source")))

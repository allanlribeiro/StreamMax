#!/usr/bin/env python3
"""Cria um .wgt Tizen de avaliação, NÃO assinado, a partir de um build Tizen upstream.

Isto NÃO converte o APK StreamMax/Compose para Tizen. Reaproveita o runtime
Tizen aberto do Nuvio Native Legacy 2.0.8 e personaliza somente o invólucro
(nome na lista de apps, package ID, ícone e título da abertura HTML).
As diferenças de interface/funções continuam e precisam ser portadas.
"""
from __future__ import annotations
import argparse
import hashlib
import pathlib
import re
import sys
import tempfile
import zipfile
import xml.etree.ElementTree as ET

WIDGET_NS = "http://www.w3.org/ns/widgets"
TIZEN_NS = "http://tizen.org/ns/widgets"
ET.register_namespace("", WIDGET_NS)
ET.register_namespace("tizen", TIZEN_NS)
APP_ID = "StreamMax1.StreamMax"
PACKAGE_ID = "StreamMax1"
OLD_PACKAGE = "NuvioTV002"


def build(source: pathlib.Path, dest: pathlib.Path, icon: pathlib.Path) -> None:
    if not source.is_file() or not icon.is_file():
        raise SystemExit("Pacote Tizen upstream ou ícone StreamMax ausente.")
    with zipfile.ZipFile(source) as upstream:
        names = set(upstream.namelist())
        essential = {"config.xml", "index.html", "index.js", "index.wasm", "icon.png"}
        missing = essential - names
        if missing:
            raise SystemExit("WGT incompleto: " + ", ".join(sorted(missing)))
        xml = ET.fromstring(upstream.read("config.xml"))
        widget_id = xml.attrib.get("id", "")
        app = xml.find("{" + TIZEN_NS + "}application")
        if app is None:
            raise SystemExit("Manifesto sem tizen:application")
        if app.attrib.get("package") != OLD_PACKAGE:
            raise SystemExit("ID do upstream mudou (" + app.attrib.get("package", "") + "); revisar antes de empacotar.")
        xml.set("id", "https://streammax.app/tizen-preview")
        xml.set("version", "0.1.0")
        app.set("id", APP_ID)
        app.set("package", PACKAGE_ID)
        name = xml.find("{" + WIDGET_NS + "}name")
        if name is None:
            name = ET.SubElement(xml, "{" + WIDGET_NS + "}name")
        name.text = "StreamMax Tizen Preview"
        xml_bytes = ET.tostring(xml, encoding="utf-8", xml_declaration=True)
        html = upstream.read("index.html")
        html = re.sub(rb"<title>[^<]*</title>", b"<title>StreamMax Tizen Preview</title>", html, count=1)
        origin = (
            "StreamMax Tizen Preview 0.1.0 (AVALIACAO)\n"
            "Interface interna e motor: Nuvio Native Legacy 2.0.8, GPL-3.0.\n"
            "Fonte: https://github.com/iqui27/nuvio-native-legacy/tree/v2.0.8\n"
            "Icone/nome na lista de apps: StreamMax 1.0.11 Android.\n"
            "Fonte StreamMax: https://github.com/allanlribeiro/StreamMax\n"
            "Nao corresponde funcionalmente ao APK StreamMax 1.0.11.\n"
            "Pacote distribuido sem assinatura: assinar para o DUID da TV.\n"
        ).encode("utf-8")
        dest.parent.mkdir(parents=True, exist_ok=True)
        with zipfile.ZipFile(dest, "wgt-unused.zip" if False else "w", compression=zipfile.ZIP_DEFLATED, compresslevel=6) as out:
            for info in upstream.infolist():
                nm = info.filename
                if nm.endswith("/") or nm in {"config.xml", "index.html", "icon.png", "ORIGEM.txt"}:
                    continue
                # Nunca reutilizar assinatura do pacote original apos alterar manifesto.
                if re.fullmatch(r"(?:author-)?signature\d*\.xml", pathlib.PurePosixPath(nm).name, re.I):
                    continue
                if nm.startswith("/") or ".." in pathlib.PurePosixPath(nm).parts:
                    raise SystemExit("Nome de arquivo inseguro no WGT: " + nm)
                out.writestr(nm, upstream.read(nm), compress_type=zipfile.ZIP_DEFLATED)
            out.writestr("config.xml", xml_bytes)
            out.writestr("index.html", html)
            out.writestr("icon.png", icon.read_bytes())
            out.writestr("ORIGEM.txt", origin)

    with zipfile.ZipFile(dest) as built:
        data = built.read("config.xml").decode("utf-8")
        assert APP_ID in data and PACKAGE_ID in data and "StreamMax Tizen Preview" in data
        assert not any(re.fullmatch(r"(?:author-)?signature\d*\.xml", p, re.I) for p in built.namelist())
        assert "index.wasm" in built.namelist()
        assert built.testzip() is None
    digest = hashlib.sha256(dest.read_bytes()).hexdigest()
    (dest.parent / (dest.name + ".sha256")).write_text(digest + "  " + dest.name + "\n", encoding="utf-8")
    print("WGT de avaliacao montado (SEM assinatura): " + str(dest))
    print("SHA-256: " + digest)
    print("Proveniencia original: " + widget_id)


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("upstream_wgt", type=pathlib.Path)
    parser.add_argument("out_wgt", type=pathlib.Path)
    parser.add_argument("streammax_icon", type=pathlib.Path)
    args = parser.parse_args()
    build(args.upstream_wgt, args.out_wgt, args.streammax_icon)

#!/usr/bin/env python3
"""Regressao atual do destaque: renderizador completo, escala e janela Android.

A referencia original de 1.0.22 fica documentada em docs/trailer-original.
A foto da TCL exigiu validar a integracao da Home compacta, nao so colorStops.
"""
from pathlib import Path
import runpy
runpy.run_path(str(Path(__file__).with_name("streammax-trailer-edges.py")), run_name="__main__")

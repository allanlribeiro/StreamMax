#!/usr/bin/env python3
"""Inclui o SDK público exigido pelo AVPlay, somente no WGT Samsung.
Referência: developer.samsung.com/smarttv/develop/guides/multimedia/
media-playback/using-avplay.html (Prerequisites).
"""
import re
import sys
from pathlib import Path
html=Path(sys.argv[1])
s=html.read_text()
url='$WEBAPIS/webapis/webapis.js'
if url not in s:
    s,n=re.subn(r'(<head\s*>)',r'\1\n<script src="'+url+'"></script>',s,count=1,flags=re.I)
    if n!=1: raise SystemExit('WGT sem <head>: biblioteca Samsung não pode ser instalada')
    html.write_text(s)
if url not in s: raise SystemExit('Biblioteca AVPlay ausente')

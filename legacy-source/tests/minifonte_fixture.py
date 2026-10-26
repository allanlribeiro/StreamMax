"""Origem e CDN locais; nenhuma credencial ou lista IPTV real e necessaria."""
import http.server
import struct
import subprocess
import sys
import threading
import time
import zlib


def chunk(kind, data):
    return struct.pack('!I', len(data)) + kind + data + struct.pack('!I', zlib.crc32(kind + data))


row = b'\0' + b'\xff\0\0' * 320 + b'\0\xff\0' * 320 + b'\0\0\xff' * 320
image = b'\x89PNG\r\n\x1a\n' + chunk(b'IHDR', struct.pack('!IIBBBBB', 960, 180, 8, 2, 0, 0, 0))
image += chunk(b'IDAT', zlib.compress(row * 180)) + chunk(b'IEND', b'')
requests = []
errors = []
other = ''


class Handler(http.server.BaseHTTPRequestHandler):
    def log_message(self, *_):
        pass

    def do_GET(self):
        path = self.path
        requests.append((self.server.role, path))
        if self.server.role == 'cdn':
            if self.headers.get('Authorization') or self.headers.get('X-Source-Key'):
                errors.append('Credenciais enviadas a outra origem')
        elif path != '/large.vtt':
            if self.headers.get('Authorization') != 'Bearer fixture':
                errors.append('Credencial da fonte ausente: ' + path)
        if 'forbidden' in self.headers.get('Host', ''):
            errors.append('Host externo aceito')
        if path == '/redirect/master.m3u8':
            return self.redirect('/catalog/master.m3u8')
        if path == '/jump.vtt':
            return self.redirect(other + '/public/frames.vtt')
        body = None
        if path == '/catalog/master.m3u8':
            body = '#EXTM3U\n#EXT-X-IMAGE-STREAM-INF:BANDWIDTH=100,CODECS="png",RESOLUTION=320x180,URI="thumbs/index.m3u8"\n#EXT-X-STREAM-INF:BANDWIDTH=9000\nvideo.m3u8\n'
        elif path == '/catalog/thumbs/index.m3u8':
            body = '#EXTM3U\n#EXT-X-IMAGES-ONLY\n#EXTINF:25,\n#EXT-X-TILES:RESOLUTION=320x180,LAYOUT=3x1,DURATION=10\nsheet.png\n#EXT-X-ENDLIST\n'
        elif path in ('/catalog/thumbs/sheet.png', '/public/sheet.png'):
            body = image
        elif path in ('/plain.vtt', '/public/frames.vtt', '/bad-image.vtt', '/slow.vtt'):
            if path == '/slow.vtt':
                time.sleep(.4)
            target = 'broken.png' if path == '/bad-image.vtt' else '/catalog/thumbs/sheet.png'
            if path == '/public/frames.vtt':
                target = 'sheet.png'
            body = 'WEBVTT\n\n00:00.000 --> 00:10.000\n' + target + '#xywh=0,0,320,180\n'
        elif path == '/large.vtt':
            body = b'x' * (2 * 1024 * 1024 + 1)
        elif path == '/broken.png':
            body = b'not an image'
        else:
            errors.append('Requisicao inesperada: ' + path)
        if body is None:
            self.send_error(404)
            return
        if isinstance(body, str):
            body = body.encode()
        self.send_response(200)
        self.send_header('Content-Length', str(len(body)))
        self.end_headers()
        try:
            self.wfile.write(body)
        except (BrokenPipeError, ConnectionResetError):
            pass

    def redirect(self, target):
        self.send_response(302)
        self.send_header('Location', target)
        self.send_header('Content-Length', '0')
        self.end_headers()


servers = []
for role in ('source', 'cdn'):
    server = http.server.ThreadingHTTPServer(('127.0.0.1', 0), Handler)
    server.role = role
    threading.Thread(target=server.serve_forever, daemon=True).start()
    servers.append(server)
origin, other = ['http://127.0.0.1:' + str(s.server_port) for s in servers]
try:
    subprocess.run([sys.argv[1], origin, other], check=True, timeout=35)
    assert not errors, errors
    assert sum(p == '/catalog/master.m3u8' for _, p in requests) == 2
    print('OK: fixture confirmou ausencia de pedidos ao video e de vazamento de cabecalhos')
finally:
    for server in servers:
        server.shutdown()

"""Provedor Xtream local de teste. Nenhuma conta ou midia de terceiros."""
from http.server import SimpleHTTPRequestHandler, ThreadingHTTPServer
from pathlib import Path
from urllib.parse import urlparse, parse_qs
import json
import os

ROOT = Path(__file__).resolve().parents[2]
BUILD = Path(os.environ.get('STREAMMAX_WEB_DIR', str(ROOT / 'legacy-source/build/iptv-vod-ui')))
OUT = ROOT / 'output/iptv-vod'
OUT.mkdir(parents=True, exist_ok=True)
ORIGIN = 'https://fixture-iptv.example.invalid'


class Handler(SimpleHTTPRequestHandler):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, directory=str(BUILD), **kwargs)

    def log_message(self, *_):
        pass

    def end_headers(self):
        self.send_header('Cross-Origin-Opener-Policy', 'same-origin')
        self.send_header('Cross-Origin-Embedder-Policy', 'require-corp')
        self.send_header('Cross-Origin-Resource-Policy', 'cross-origin')
        super().end_headers()

    def json(self, data, status=200):
        b = json.dumps(data, ensure_ascii=False).encode()
        self.send_response(status)
        self.send_header('Content-Type', 'application/json; charset=utf-8')
        self.send_header('Content-Length', str(len(b)))
        self.end_headers()
        self.wfile.write(b)

    def do_GET(self):
        u = urlparse(self.path)
        if u.path == '/blank':
            self.send_response(200)
            self.end_headers()
            self.wfile.write(b'<!doctype html><title>Fixture StreamMax</title>')
            return
        if u.path == '/player_api.php':
            q = parse_qs(u.query)
            action = q.get('action', [''])[0]
            if q.get('username') != ['fixture'] or q.get('password') != ['teste-local']:
                self.json({'user_info': {'auth': 0}}, 401)
                return
            with (OUT / 'requests.jsonl').open('a') as log:
                log.write(json.dumps({'action': action, 'category': q.get('category_id')}) + '\n')
            cats = [{'category_id': '10', 'category_name': 'Ação e aventura'},
                    {'category_id': 20, 'category_name': 'Drama'},
                    {'category_id': '30', 'category_name': 'Comédia'}]
            if action in ('get_vod_categories', 'get_series_categories'):
                self.json(cats)
            elif action in ('get_vod_streams', 'get_series'):
                series = action == 'get_series'
                cat = q.get('category_id', ['10'])[0]
                movies = [('Perdido em Marte', 12, 2015), ('O Grande Truque', 2, 2006),
                          ('Os Suspeitos', 9, 2013), ('O Astronauta', 10, ''),
                          ('Devoradores de Estrelas', 13, ''), ('Mercy', 16, ''),
                          ('O Nevoeiro', 19, ''), ('Off the Grid', 26, ''),
                          ('A Filha do Rei do Pântano', 28, ''), ('June & John', 30, ''),
                          ('Último Suspiro', 32, ''), ('1992', 34, ''),
                          ('Extermínio: A Evolução', 36, '')]
                shows = [('Fallout', 0, ''), ('Origem', 4, ''), ('Frequência', 5, ''),
                         ('O Problema dos 3 Corpos', 15, ''), ('Extrapolações', 17, ''),
                         ('Lost', 21, ''), ('A Lista Terminal', 23, ''), ('Nancy Drew', 25, ''),
                         ('Locke & Key', 27, ''), ('It: Bem-vindos a Derry', 29, ''),
                         ('WandaVision', 31, ''), ('O Dia do Chacal', 33, ''),
                         ('The Umbrella Academy', 35, '')]
                data = []
                for i, (name, poster, year) in enumerate(shows if series else movies):
                    it = {('series_id' if series else 'stream_id'): 51 + i,
                          'name': name,
                          'category_id': cat, 'year': year, 'rating': '8.4',
                          'container_extension': 'mp4',
                          ('cover' if series else 'stream_icon'): ORIGIN + f'/poster-{poster:02d}.jpg',
                          'plot': 'Catálogo de demonstração para testar o StreamMax. '
                                  'Este título usa uma mídia sintética local; não representa um serviço IPTV real.'}
                    data.append(it)
                self.json(data)
            elif action == 'get_series_info':
                self.json({'episodes': {
                    '2': [{'id': 902, 'season': 2, 'episode_num': 1, 'title': 'Um novo caminho', 'container_extension': 'mp4'}],
                    '1': [{'id': 800 + i, 'season': 1, 'episode_num': i,
                           'title': ['A chegada', 'O reencontro', 'A descoberta'][i-1],
                           'container_extension': 'mp4', 'info': {'plot': 'Episódio de demonstração.'}}
                          for i in range(1, 4)]}})
            elif action == 'get_live_categories':
                self.json([{'category_id': 1, 'category_name': 'Canais de teste'}])
            elif action == 'get_live_streams':
                self.json([{'stream_id': 101, 'name': 'Canal de teste', 'category_id': 1,
                            'stream_icon': ORIGIN + '/poster-00.jpg'}])
            elif action == 'get_short_epg':
                self.json({'epg_listings': []})
            else:
                self.json({'user_info': {'auth': 1, 'status': 'Active', 'allowed_output_formats': ['ts', 'm3u8']}})
            return
        if u.path.startswith('/poster-') and u.path.endswith('.jpg'):
            name = Path(u.path).name.removeprefix('poster-')
            p = ROOT / 'legacy-source/deploy/app/art/poster' / name
            b = p.read_bytes()
            self.send_response(200)
            self.send_header('Content-Type', 'image/jpeg')
            self.send_header('Content-Length', str(len(b)))
            self.end_headers()
            self.wfile.write(b)
            return
        if u.path.startswith(('/movie/', '/series/', '/live/')):
            with (OUT / 'requests.jsonl').open('a') as log:
                log.write(json.dumps({'media': u.path.split('/')[1], 'arquivo': Path(u.path).name}) + '\n')
            p = ROOT / 'tests/iptv-vod/midia-teste.mp4'
            size = p.stat().st_size
            start, end = 0, size-1
            partial = self.headers.get('Range', '')
            if partial.startswith('bytes='):
                bounds = partial[6:].split('-', 1)
                start = int(bounds[0] or 0)
                end = min(int(bounds[1]) if bounds[1] else size-1, size-1)
            self.send_response(206 if partial else 200)
            self.send_header('Content-Type', 'video/mp4')
            self.send_header('Accept-Ranges', 'bytes')
            if partial:
                self.send_header('Content-Range', f'bytes {start}-{end}/{size}')
            self.send_header('Content-Length', str(end-start+1))
            self.end_headers()
            with p.open('rb') as f:
                f.seek(start)
                self.wfile.write(f.read(end-start+1))
            return
        super().do_GET()


if __name__ == '__main__':
    ThreadingHTTPServer(('127.0.0.1', 8788), Handler).serve_forever()

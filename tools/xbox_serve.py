#!/usr/bin/env python3
"""Laptop side of the Xbox network loader (src/pc/xbox/loader, docs/xbox.md "Network loader").

    tools/xbox_serve.py [--port 8765] [--xbe build/xbox/nv2a/default.xbe]

Serves build/xbox/default.xbe (the null-graphics build; --xbe for another) to the loader as /mh1.xbe, with /version
(size and time of that file, plus build/xbox/rerun.txt when present, so
touching that file makes a waiting loader start the same build again).
Logs the Xbox sends (POST /log) are saved to build/xbox/logs/<time>.txt and
printed. Only these paths are served; nothing else in the repo is reachable.
"""
import argparse
import http.server
import os
import sys
import time

ROOT = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', 'build', 'xbox')
XBE = os.path.join(ROOT, 'default.xbe')
RERUN = os.path.join(ROOT, 'rerun.txt')
LOGS = os.path.join(ROOT, 'logs')


def version():
    st = os.stat(XBE)
    v = '%d-%d' % (st.st_size, int(st.st_mtime))
    if os.path.exists(RERUN):
        v += '-' + open(RERUN).read().strip()
    return v


class Handler(http.server.BaseHTTPRequestHandler):
    def reply(self, body, ctype='application/octet-stream'):
        self.send_response(200)
        self.send_header('Content-Type', ctype)
        self.send_header('Content-Length', str(len(body)))
        self.end_headers()
        self.wfile.write(body)

    def do_GET(self):
        if self.path == '/version':
            self.reply(version().encode(), 'text/plain')
        elif self.path == '/mh1.xbe':
            self.reply(open(XBE, 'rb').read())
        else:
            self.send_error(404)

    def do_POST(self):
        body = self.rfile.read(int(self.headers.get('Content-Length', 0)))
        if self.path == '/log':
            os.makedirs(LOGS, exist_ok=True)
            name = os.path.join(LOGS, time.strftime('%Y%m%d-%H%M%S') + '.txt')
            open(name, 'wb').write(body)
            print('== Xbox log -> %s (%d bytes)' % (name, len(body)))
            sys.stdout.write(body.decode('utf-8', 'replace')[-3000:] + '\n')
        elif self.path == '/note':
            print('== Xbox: %s' % body.decode('utf-8', 'replace'))
        else:
            self.send_error(404)
            return
        self.reply(b'ok', 'text/plain')

    def log_message(self, fmt, *args):
        print('%s %s' % (time.strftime('%H:%M:%S'), fmt % args))
        sys.stdout.flush()


def main():
    ap = argparse.ArgumentParser(description=__doc__.split('\n')[0])
    ap.add_argument('--port', type=int, default=8765)
    ap.add_argument('--xbe', help='the build to serve (default build/xbox/default.xbe)')
    a = ap.parse_args()
    if a.xbe:
        global XBE
        XBE = os.path.abspath(a.xbe)
    print('serving %s as /mh1.xbe on port %d (version %s)' % (os.path.normpath(XBE), a.port, version()))
    sys.stdout.flush()
    http.server.ThreadingHTTPServer(('0.0.0.0', a.port), Handler).serve_forever()


if __name__ == '__main__':
    main()

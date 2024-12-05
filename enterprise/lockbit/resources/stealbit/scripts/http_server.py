#!/usr/bin/env python

import argparse
import http.server
import os

class HTTPRequestHandler(http.server.SimpleHTTPRequestHandler):
    def do_PUT(self):
        print("PUT Request!")
        path = self.translate_path(self.path)
        if path.endswith('/'):
            self.send_response(405, "Method Not Allowed")
            self.wfile.write("PUT not allowed on a directory\n".encode())
            return
        else:
            try:
                os.makedirs(os.path.dirname(path))
            except FileExistsError: pass
            length = int(self.headers['Content-Length'])
            print(length)
            byteRange = self.headers['Content-Range']
            print(byteRange)
            with open(path, 'ab') as f:
                f.write(self.rfile.read(length))
            self.send_response(201, "Created")

    def do_GET(self):
        print("GET Request!")
        content = bytes("8bef61f7-2776-4e55-9ee4-409b6061d6f1","utf-8")
        self.send_response(200, "OK")

        if self.path == "/bs/portal":
            self.send_header("Content-type", "text/html")
            self.send_header("Content-Length", str(len(content)))
            self.end_headers()
            self.wfile.write(content)

        return

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('--bind', '-b', default='', metavar='ADDRESS',
                        help='Specify alternate bind address '
                             '[default: all interfaces]')
    parser.add_argument('port', action='store',
                        default=8000, type=int,
                        nargs='?',
                        help='Specify alternate port [default: 8000]')
    args = parser.parse_args()

    http.server.test(HandlerClass=HTTPRequestHandler, port=args.port, bind=args.bind)
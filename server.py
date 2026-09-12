from http.server import BaseHTTPRequestHandler, HTTPServer

class Handler(BaseHTTPRequestHandler):

    def do_POST(self):

        length = int(self.headers['Content-Length'])
        data = self.rfile.read(length)

        print("Received:")
        print(data.decode())

        self.send_response(200)
        self.end_headers()
        self.wfile.write(b"OK")


server = HTTPServer(("0.0.0.0", 8000), Handler)

print("Listening on port 8000...")

server.serve_forever()
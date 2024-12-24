import http.server
import socketserver


class Site2Handler(http.server.SimpleHTTPRequestHandler):

    def do_GET(self):
        if self.path == '/':
            self.send_response(200)
            self.send_header('Content-Type', 'text/plain')
            self.end_headers()
            self.wfile.write(b"Welcome to the text-only site!")
        else:
            self.send_response(404)
            self.end_headers()


PORT = 8080
httpd = socketserver.TCPServer(("", PORT), Site2Handler)
print(f"Serving on port {PORT}")
httpd.serve_forever()

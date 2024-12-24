import http.server
import socketserver

REDIRECT_HOST = "localhost"
REDIRECT_PORT = 8082


class Site3Handler(http.server.SimpleHTTPRequestHandler):

    def do_GET(self):
        if self.path == '/':
            self.send_response(302)
            self.send_header('Location',
                             f'http://{REDIRECT_HOST}:{REDIRECT_PORT}/')
            self.end_headers()
        else:
            self.send_response(404)
            self.end_headers()


PORT = 8080
httpd = socketserver.TCPServer(("", PORT), Site3Handler)
print(f"Serving on port {PORT}")
httpd.serve_forever()

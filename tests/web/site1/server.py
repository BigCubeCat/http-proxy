import http.server
import socketserver

PORT = 8080


class Site1Handler(http.server.SimpleHTTPRequestHandler):

    def do_GET(self):
        self.send_response(200)
        self.send_header('Content-Length', '5242880')
        self.send_header('Content-Type', 'application/zip')
        self.end_headers()
        with open('largefile.zip', 'rb') as file:
            self.wfile.write(file.read())


httpd = socketserver.TCPServer(("", PORT), Site1Handler)
print(f"Serving on port {PORT}")
httpd.serve_forever()

import SocketServer
import BaseHTTPServer
import CGIHTTPServer
import sys

class ThreadingCGIServer(SocketServer.ThreadingMixIn,
                   BaseHTTPServer.HTTPServer):
    pass

if sys.argv[1:]:
    address = sys.argv[1]
else:
    address = '0.0.0.0'

if sys.argv[2:]:
    port = int(sys.argv[2])
else:
    port = 8080;

server = ThreadingCGIServer((address, port), CGIHTTPServer.CGIHTTPRequestHandler)

print('Started HTTP server on ' +  address + ':' + str(port))

try:
    server.serve_forever()
except KeyboardInterrupt:
    print "Finished"

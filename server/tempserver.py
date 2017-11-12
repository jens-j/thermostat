#!/usr/bin/env python3

import socket
import netifaces
import struct

port = 8888

serverSocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
serverSocket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1) # This is regarded as a highly controversial move
serverSocket.bind(('', port))
print('server bound to %s:%d' % (socket.gethostname(), port))
serverSocket.listen(1)

try:
    clientSocket, address = serverSocket.accept() 
    print("Got a connection from %s" % str(address))

    while True:
        recvBuffer = clientSocket.recv(4096)
        print('received %d bytes' % len(recvBuffer))
        print('received: %s' % recvBuffer)

        # if len(recvBuffer) == 4:
        temperature = struct.unpack('<f', recvBuffer[-4:])
        print('temperature = %.2f C' % temperature)
        # else:
        #     print("incorrect number of bytes received")

except:
    print('closing socket')
    serverSocket.close()
    exit()
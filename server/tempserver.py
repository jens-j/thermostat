#!/usr/bin/env python3

import socket
import netifaces
import struct
from datetime import datetime

port = 8888

serverSocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
serverSocket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1) # This is regarded as a highly controversial move
serverSocket.bind(('', port))
print('server bound to %s:%d' % (socket.gethostname(), port))
serverSocket.listen(1)

try:
    clientSocket, address = serverSocket.accept() 
    print("Got a connection from %s" % str(address))

    t0 = datetime.now().strftime('%d-%m-%y_%H:%M:%S')
    
    while True:

        with open('../log/temperature_%s.log' % t0, 'a') as f:

            recvBuffer = clientSocket.recv(4096)
            print('received %d bytes' % len(recvBuffer))
            print('received: %s' % recvBuffer)

            temperature = struct.unpack('<f', recvBuffer[-4:])[0]
            print('temperature = %.2f C' % temperature)

            t = datetime.now().strftime('%d-%m-%y_%H:%M:%S')
            f.write('%s %.2f\n' % (t, temperature))


except Exception as e:
    print('Exception: %s' % str(e))
    print('closing socket')
    serverSocket.close()
    exit()
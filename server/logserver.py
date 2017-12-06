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
    while True:

        clientSocket, address = serverSocket.accept()
        print("Got a connection from %s" % str(address))

        t0 = datetime.now().strftime('%d-%m-%y_%H:%M:%S')

        while True:

            with open('../log/temperature_%s.log' % t0, 'a') as f:

                recvBuffer = clientSocket.recv(4096)

                if not recvBuffer:
                    print('client closed connection')
                    break;

                print('received %d bytes' % len(recvBuffer))
                print('received: %s' % recvBuffer)

                try:
                    msgType, input, output, setPoint, iTerm, kP, kI, kD = \
                        struct.unpack('<Bfffffff', recvBuffer[-29:])
                except Exception as e:
                    print('exception: %s' % str(e))
                else:
                    if msgType != 0:
                        print('unknown message type (%x)' % msgType)
                    else:
                        dateTime = datetime.now().strftime('%d-%m-%y_%H:%M:%S')
                        s = '%s: ti = %.2f C, to = %.2f C, ts = %.2f C, iTerm = %.2f, kP = %.2f, kI = %.2f, kD = %.2f\n' \
                            % (dateTime, input, output, setPoint, iTerm, kP, kI, kD)

                        f.write(s)
                        print(s)
                        
except Exception as e:
    print('Exception: %s' % str(e))
    print('closing socket')
    serverSocket.close()
    exit()

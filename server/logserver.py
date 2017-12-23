#!/usr/bin/env python3

import socket
import struct
import threading
from datetime import datetime
from collections import deque


cmdQueue = deque()


def server(name, port, threadFunction):

    serverSocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    serverSocket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1) # This is regarded as a highly controversial move
    serverSocket.bind(('', port))
    print('(%s) server bound to %s:%d' % (name, socket.gethostname(), port))
    serverSocket.listen(1)

    threads = []

    while True:
        clientSocket, address = serverSocket.accept()
        print('(%s) got a connection from %s' % (name, str(address)))
        clientThread = threading.Thread(target=threadFunction, args=(clientSocket,)) 
        clientThread.start()   
        threads.append(clientThread)



def cmdThread(clientSocket):

    # try to receive from the client
    try:
        recvBuffer = clientSocket.recv(256)
    except Exception as e:
        print('(cmd) socket receive error: %s' % str(e))
    if not recvBuffer:
        print('(cmd) client closed connection')
        clientSocket.close();
        return

    print('(cmd) received %d bytes' % len(recvBuffer))
    print('(cmd) received: %s' % recvBuffer)

    cmdQueue.append(recvBuffer)


def espThread(clientSocket):


    clientSocket.settimeout(1.0)

    while True:

        # send queued messages to the client
        while True:
            try:
                msg = cmdQueue.popleft()
            except IndexError:
                print('(esp) cmd queue empty')
                break
            else:
                print('(esp) send: %s' % msg)
                clientSocket.send(msg)

        # try to receive from the client
        try:
            recvBuffer = clientSocket.recv(256)
        except socket.timeout:
            print('(esp) receive timeout')
            continue
        else:
            if not recvBuffer:
                print('(esp) client closed connection')
                clientSocket.close();
                return

            print('(esp) received %d bytes' % len(recvBuffer))
            print('(esp) received: %s' % recvBuffer)

            # parse the received message
            try:
                msgType, input, output, setPoint, iTerm, kP, kI, kD, heater_status, temperature = \
                    struct.unpack('<BfffffffBf', recvBuffer[-34:])
            except Exception as e:
                print('message parse error: %s' % str(type(e)))
            else:
                # check message header
                if msgType != 0:
                    print('unknown message type (%x)' % msgType)
                # unpack, print and log the message
                else:
                    dateTime = datetime.now().strftime('%d-%m-%y_%H:%M:%S')
                    s = '%s, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f, 0x%02x\n' \
                        % (dateTime, input, output, setPoint, iTerm, 
                           kP, kI, kD, heater_status)

                    print(s)
                    with open('../log/temperature_%s.log' % t0, 'a') as f:  
                        f.write(s)


# the log file spans the lifetime of the logserver
t0 = datetime.now().strftime('%d-%m-%y_%H:%M:%S')
with open('../log/temperature_%s.log' % t0, 'a') as f:
    f.write('# timestamp, Ti, To, Ts, iTerm, kP, kI, kD, heater status\n')

espSocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
cmdSocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

tEsp = threading.Thread(target=server, args=('esp', 8888, espThread)) 
tEsp.start()           

tCmd = threading.Thread(target=server, args=('cmd', 9999, cmdThread)) 
tCmd.start()    
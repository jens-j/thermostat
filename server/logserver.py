#!/usr/bin/env python3

import socket
import struct
import threading
from datetime import datetime
from enum import Enum
from collections import deque


class MsgType(Enum):
    STATE_LOG           = 0
    OT_RECV_ERROR_LOG   = 1
    OT_PARSE_ERROR_LOG  = 2
    RESET_LOG           = 3

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

    while True:

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
                #print('(esp) cmd queue empty')
                break
            else:
                print('(esp) send: %s' % msg)
                clientSocket.send(msg)

        # try to receive from the client
        try:
            recvBuffer = clientSocket.recv(256)
        except socket.timeout:
            #print('(esp) receive timeout')
            continue
        else:
            if not recvBuffer:
                print('(esp) client closed connection')
                clientSocket.close();
                return

            print('(esp) received %d bytes' % len(recvBuffer))
            print('(esp) received: %s' % recvBuffer)

            timestamp = datetime.now().strftime('%d-%m-%y_%H:%M:%S')

            # parse possibly concatenated messages 
            while len(recvBuffer) > 0:
            
                # check message header
                msgType = recvBuffer[0]

                if msgType == MsgType.STATE_LOG.value and len(recvBuffer) >= 39:
                    try:
                        input, output, setPoint, errorSum, kP, kI, kD, \
                            heater_status, heater_temperature, room_temperature, otError = \
                            struct.unpack('<fffffffBffB', recvBuffer[1:39])
                        recvBuffer = recvBuffer[39:]
                    except Exception as e:
                        print('state log parse error: %s' % str(e))
                    else:
                        s = '%s, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f, %.5f, %.2f, 0x%02x\n' % \
                            (timestamp, input, setPoint, output, heater_temperature, errorSum, 
                             kP, kI, kD, heater_status)
                        print(s)
                        with open(paramLogFile, 'a') as f:  
                            f.write(s)

                elif msgType == MsgType.OT_RECV_ERROR_LOG.value and len(recvBuffer) >= 3:
                    try:
                        errorFlags, dataId = struct.unpack('<BB', recvBuffer[1:3])
                        recvBuffer = recvBuffer[3:]
                    except Exception as e:
                        print('ot receive error log parse error: %s' % str(e))
                    else:
                        s = '[%s] OT receive error (errorType = 0x%d, dataId = %d)\n' % \
                            (timestamp, errorFlags, dataId)
                        print(s)
                        with open(eventLogFile, 'a') as f:  
                            f.write(s)
                            
                elif msgType == MsgType.OT_PARSE_ERROR_LOG.value and len(recvBuffer) >= 8:
                    try:
                        errorType, sendDataId, parity, msgType, recvDataId, dataValue = \
                            struct.unpack('<BBBBBH', recvBuffer[1:8])
                        recvBuffer = recvBuffer[8:]
                    except Exception as e:
                        print('ot parse error log parse error:' % str(e))
                    else:
                        l = {'[%s] OT parse error (errorFlags = %02x, dataId = %d):\n' % \
                                (timestamp, errorType, sendDataId),
                             '\tparity    = %d\n' % parity,
                             '\tmsgType   = %d\n' % msgType,
                             '\tdataId    = %d\n' % recvDataId,
                             '\tdataValue = %d\n' % dataValue,}
                        with open(eventLogFile, 'a') as f:
                            f.write(str(l))
                            for s in l:
                                print(s)
                                f.write(s)
                elif msgType == MsgType.RESET_LOG.value:
                    recvBuffer = recvBuffer[1:]
                    s = '[%s] Arduino reset' % timestamp
                    print(s)
                    with open(eventLogFile, 'a') as f:
                       f.write(s)
                else: # skip any at commands echoed by the esp
                    print('unknown message type (%x)' % msgType)
                    try:
                        idx = recvBuffer.index(b'\n')
                        recvBuffer = recvBuffer[idx + 1:]
                    except:
                        break

# the parameter and event log files spans the lifetime of the logserver
t0 = datetime.now().strftime('%d-%m-%y_%H:%M:%S')
paramLogFile = '../log/parameters_%s.log' % t0
eventLogFile = '../log/events_%s.log' % t0
print(eventLogFile)
with open(paramLogFile, 'a') as f:
    f.write('# timestamp, Ti, Ts, To, Th, errorSum, kP, kI, kD, heaterStatus\n')

espSocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
cmdSocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

tEsp = threading.Thread(target=server, args=('esp', 8888, espThread)) 
tEsp.start()           

tCmd = threading.Thread(target=server, args=('cmd', 9999, cmdThread)) 
tCmd.start()    

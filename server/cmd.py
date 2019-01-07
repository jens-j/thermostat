#!/usr/bin/env python3

import socket
import struct
from scanf import scanf

clientSocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
clientSocket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1) # This is regarded as a highly controversial move
clientSocket.connect(('localhost', 9999))

def setpoint(raw):
    try:
        setpoint = scanf('s %f', raw)[0]
    except:
        print('invalid setpoint command: %s' % raw)
    else:
        clientSocket.send(struct.pack('<Bf', 4, setpoint))

def coeffs(raw):
    try:
        kP, kI, kD = scanf('c %f %f %f', raw)
    except:
        print('invalid coefficients command: %s' % raw)
    else:
        clientSocket.send(struct.pack('<Bfff', 5, kP, kI, kD))

while True:
    raw = input(">")
    if raw:
        if raw[0] == 's':
            setpoint(raw)
        elif raw[0] == 'c':
            coeffs(raw)
#!/usr/bin/env python3

import socket
import struct
import argparse
from scanf import scanf

class Cmd():

    def __init__(self):

        self.clientSocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.clientSocket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1) # This is regarded as a highly controversial move
        self.clientSocket.connect(('localhost', 9999))

    def __del__(self):

        self.clientSocket.close()


    def setpoint(self, setpoint):

        self.clientSocket.send(struct.pack('<Bf', 4, setpoint))


    def coeffs(self, p, i ,d):

        self.clientSocket.send(struct.pack('<Bfff', 5, p, i, d))


if __name__ == '__main__':

    parser = argparse.ArgumentParser()
    parser.add_argument('-s', '--setpoint', type=int, help='temperature setpoint')
    arguments = parser.parse_args()

    print(arguments.setpoint)

    cmd = Cmd()
    cmd.setpoint(arguments.setpoint)



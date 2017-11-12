#!/usr/bin/env python3

import socket

port = 8888

client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
client.connect(("192.168.2.3", port))
client.send(b'a')
print(client.recv(4096))    
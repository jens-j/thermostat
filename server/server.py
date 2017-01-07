#!/usr/bin/env python3

import socket
import netifaces

port = 8888

serverSocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
#serverSocket.bind((socket.gethostname(), port))
serverSocket.bind(('', port))
print('bound to %s:%d' % (socket.gethostname(), port))
serverSocket.listen(5)

try:
	while True:
		clientSocket, address = serverSocket.accept() 
		print("Got a connection from %s" % str(address))
		recvBuffer = clientSocket.recv(4096)

		if recvBuffer == b'':
			print('error')
		else:	
			print('received: %s' % recvBuffer)
			clientSocket.send(b'b')

		clientSocket.close()
except:
	serverSocket.close()
	exit()

# -*- coding: utf-8 -*-
import socket

# HOST = '127.0.0.1'  # Standard loopback interface address (localhost)
# PORT = 65432        # Port to listen on (non-privileged ports are > 1023)
HOST = '192.168.100.11'
PORT = 30000

# Kaksi ennalta määritettyä käden asentoa eli drop-off pointia.
paikat = [
#str.encode("(-0.586169,-0.413107,0.242636,-1.91015,-1.85181,0.584344)"),
str.encode("(4.68707,-3.11296,-0.667016,-0.920144,1.58624,-0,0302075)"),
str.encode("(5.89284,-2.85641,-1.68474,-0.149206,1.56736,-0.316181)")
]

print("Palvelin odottaa.")
with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
	s.bind((HOST, PORT))
	s.listen()
	conn, addr = s.accept()
	with conn:
		print('Connected by', addr)
		print(type(addr[0]))
		while True:
			data = conn.recv(1024)
			if not data:
				break
			print(data)
			
			kumpi = int(input("Kumpi 0 vai 1: "))
			conn.sendall(paikat[kumpi])

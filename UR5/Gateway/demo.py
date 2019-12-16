# -*- coding: utf-8 -*-
import socket

# HOST = '127.0.0.1'  # Standard loopback interface address (localhost)
# PORT = 65432        # Port to listen on (non-privileged ports are > 1023)
HOST = '192.168.100.11'
PORT = 30000

# Kaksi ennalta määritettyä käden asentoa eli drop-off pointia.
paikat3 = [
	str.encode("4.23932,-2.47179,-1.31964,-0.908956,1.5553,1.06779)"),
	str.encode("3.965,-2.11616,-2.05918,-0.526054,1.55791,0.797925)"),
	str.encode("3.42745,-1.99363,-2.32193,-0.3814,1.56627,0.255482)")
	]

paketti = str(int(input("Anna pakettiID mitä siiretään: ")))

i = 0

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
			if data == str.encode("Kiinni"):
				break
			print(data)
			if i > 2:
				i = 0
			viesti = str.encode("(" + paketti) + paikat3[i]
			print(viesti)
			conn.sendall(viesti)
			i = i + 1

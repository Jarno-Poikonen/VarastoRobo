# -*- coding: utf-8 -*-
import socket

# HOST = '127.0.0.1'  # Standard loopback interface address (localhost)
# PORT = 65432        # Port to listen on (non-privileged ports are > 1023)
HOST = '192.168.100.11'
PORT = 30000

# Kaksi ennalta määritettyä käden asentoa eli drop-off pointia.
paikat = [
	str.encode("(4.25674,-2.39935,-1.5091,-0.827786,1.62284,1.02993)"),
	str.encode("(4.04182,-2.09472,-2.12977,-0.506068,1.58517,0.867442)"),
	str.encode("(3.60708,-1.8002,-2.46198,-0.392773,1.59138,0.444104)")
	]

paikat2 = [
	str.encode(",4.25674,-2.39935,-1.5091,-0.827786,1.62284,1.02993)"),
	str.encode(",4.04182,-2.09472,-2.12977,-0.506068,1.58517,0.867442)"),
	str.encode(",3.60708,-1.8002,-2.46198,-0.392773,1.59138,0.444104)")
	]

paikat3= [
	str.encode(",4.25055,-2.26264,-1.68326,-0.763072,1.57923,1.09164)"),
	str.encode(",3.9939,-2.01335,-2.21448,-0.482631,1.59763,0.872768)"),
	str.encode(",3.51784,-1.8909,-2.4487,-0.383809,1.59626,0.396642)")
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

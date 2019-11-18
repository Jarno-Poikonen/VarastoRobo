# -*- coding: utf-8 -*-
import socket
from time import sleep
import threading

MasterIP = ""
MasterPort = None

# Mahdollista tehdä paremmin.
# Muutetaan viestin pituus esitettäväksi neljällä bytellä
def Laske_pituus(viesti, pituus=4):
	
	palautus = bytearray([])
	
	palautus.append(viesti)
	
	for x in range(pituus-1):
		palautus.append(0)
	
	
	return palautus

# Säie/funktio joka kuuntelee broadcast viestejä.
def Broadcast_communication():
	# Luodaan socket broadcastia varten.
	print("Broadcast")
	bSocket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
	bSocket.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, True)
	bSocket.bind(("", 1732))
	
	# Luetaan broadcast viestejä IP:n selvittämiseksi.
	wait_for_message = True
	while wait_for_message :

		message, master_address = bSocket.recvfrom(512)
		
		if len(message) >= 8 and message[0] == 1 and message[1] == 7 :

			system_status = int(message[2])
			master_id = int(message[3])
			wait_for_message = False
			global MasterIP
			global MasterPort 
			MasterIP, MasterPort = master_address
			MasterPort = 1739
	
	# Luetaan broadcast viestejä ja reagoidaan hätäseis käskyyn.
	# while True:
		# message, master_address = bSocket.recvfrom(512)
		
		# if len(message) >= 8 and message[0] == 1 and message[1] == 7 :
			# system_status = int(message[2])
			# if system_status != 1:
				# seis = True
				# break

def Luo_NCM(tyyppi, id, tila):
	viesti = bytearray([2])
	viestin_loppu = bytearray([tyyppi, id, 0, 0, 0, tila])
	pituus = Laske_pituus(len(viestin_loppu))
	
	for b in pituus:
		viesti.append(b)
		
	for b in viestin_loppu:
		viesti.append(b)
	
	return viesti

# Luodaan WFM viesti annetuista parametreista.
def Luo_WFM(vastaukseksi, virhe, suoritus, tila):
	viesti = bytearray([4])
	viestin_loppu = bytearray([vastaukseksi, virhe, suoritus, 0, 0, 0, tila])
	pituus = Laske_pituus(len(viestin_loppu))
	
	for b in pituus:
		viesti.append(b)
		
	for b in viestin_loppu:
		viesti.append(b)
	
	return viesti

if __name__ == "__main__":

	# Käsien paikat.
	paikat = [
	#str.encode("(-0.586169,-0.413107,0.242636,-1.91015,-1.85181,0.584344)"),
	str.encode("(-0.586169,-0.413107,0.242636,-1.91015,-1.85181,0.584344)"),
	str.encode("(5.89284,-2.85641,-1.68474,-0.149206,1.56736,-0.316181)")
	]
	
	broadcast_saie = threading.Thread(target=Broadcast_communication, args=())
	broadcast_saie.start()
	
	ID = 55
	
	# URIP = ""
	# URPort = 0
	
	# URSocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
	# URSocket.bind((URIP, URPort))
	# URSocket.listen()
	
	# URYhteys, URAddr = URSocket.accept()
	
	print("Odotetaan Masterin IP.")
	while MasterPort is None:
		pass
	
	print("IP saatu.")
	MasterSocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
	MasterSocket.connect((MasterIP, MasterPort))
	viesti = Luo_NCM(3, ID, 2)
	print(viesti)
	MasterSocket.send(viesti)
	sleep(1)
	while True:
		MasterData = MasterSocket.recv(512)
		print(MasterData)
		try:
			if MasterData[0] == 3:
				MasterSocket.sendall(Luo_WFM(MasterData[0], 0, 1, 1))
				break
		except:
			sleep(5)
			continue
	
	print("Valmis")

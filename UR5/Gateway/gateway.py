# -*- coding: utf-8 -*-
import socket
from time import sleep
import threading

MasterIP = None
MasterPort = None
seis = False

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
	'''
	# Luodaan socket hätäseis toiminnallisuutta varten
	SEISsocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
	SEISsocket.bind(("192.168.100.11", 30001))
	SEISsocket.listen()
	
	# Luetaan broadcast viestejä ja reagoidaan hätäseis käskyyn.
	while True:
		message, master_address = bSocket.recvfrom(512)
		
		if len(message) >= 8 and message[0] == 1 and message[1] == 7 :
			system_status = int(message[2])
			if system_status != 1:
				global seis
				seis = True
				SEISyhteys, SEISosoite = SEISsocket.accept()
				SEISyhteys.sendall("seis")
				SEISyhteys.close()
				break
	'''

def Luo_NCM(tyyppi, id, tila):
	viesti = bytearray([2])
	viestin_loppu = bytearray([tyyppi, id, 0, 0, 0, tila])
	pituus = Laske_pituus(len(viestin_loppu))
	
	for b in pituus:
		viesti.append(b)
		
	for b in viestin_loppu:
		viesti.append(b)
	
	print("NCM: ", viesti)
	
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
	
	print("WFM: ", viesti)
	
	return viesti

if __name__ == "__main__":

	# Käsien paikat.
	# Ensimämisenä tuotteen numero, sen jälkeen 6 float arvoa servojen asennoille.
	paikat = [
	str.encode(",4.25674,-2.39935,-1.5091,-0.827786,1.62284,1.02993)"),
	str.encode(",4.04182,-2.09472,-2.12977,-0.506068,1.58517,0.867442)"),
	str.encode(",3.60708,-1.8002,-2.46198,-0.392773,1.59138,0.444104)")
	]
	
	broadcast_saie = threading.Thread(target=Broadcast_communication, args=())
	broadcast_saie.start()
	
	ID = 55
	tila = 1
	
	URIP = "192.168.100.11"
	URPort = 30000
	
	URSocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
	URSocket.bind((URIP, URPort))
	URSocket.listen()
	
	URYhteys, URAddr = URSocket.accept()
	
	print("Odotetaan Masterin IP.")
	while MasterIP is None:
		pass
	print("IP saatu.")
	
	MasterSocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
	MasterSocket.connect((MasterIP, MasterPort))
	
	MasterSocket.send(Luo_NCM(3, ID, 2))
	
	while not seis:
		MasterData = MasterSocket.recv(512)
		print(MasterData)
		try:
			if MasterData[0] == 3:		# Setup Connection Message
				print("Setup Connection Message saatu")
				ID = MasterData[6]
				MasterSocket.sendall(Luo_WFM(MasterData[0], 0, 1, tila))
				# continue
				
			elif MasterData[0] == 5:	# Closed Connection Message
				print("Closed Connection Message saatu")
				MasterSocket.sendall(Luo_WFM(MasterData[0], 2, 1, tila))
				# break
				
			elif MasterData[0] == 6:	# Status Query Message
				print("Status Query Message saatu")
				MasterSocket.sendall(Luo_WFM(MasterData[0], 0, 1, tila))
				# continue
				
			elif MasterData[0] == 7:	# System Startup Message
				print("System Shutdown Message saatu")
				tila = 1
				MasterSocket.sendall(Luo_WFM(MasterData[0], 0, 1, tila))
				
			elif MasterData[0] == 8:	# System Shutdown Message
				print("System Shutdown Message saatu")
				tila = 2
				MasterSocket.sendall(Luo_WFM(MasterData[0], 0, 1, tila))
				
			elif MasterData[0] == 12:	# Move Product Message
				print("Move Product Message saatu")
				
				data = str.encode("(") + str.encode(str(MasterData[5])) + paikat[MasterData[6]]
				print(data)
				URYhteys.sendall(data)
				
				URData = URYhteys.recv(512)
				
				print(URData)
				utf = URData.decode("utf-8")
				print(utf)
				
				if "Valmis" in utf:
					MasterSocket.sendall(Luo_WFM(MasterData[0], 0, 1, tila))
				elif "Fail" in utf:
					MasterSocket.sendall(Luo_WFM(MasterData[0], 4, 1, tila))
			
			elif MasterData[0] == 9 or MasterData[0] == 10 or MasterData[0] == 11 or MasterData[0] == 13 or MasterData[0] == 14:
				print("Ei tuettu komento")
				MasterSocket.sendall(Luo_WFM(MasterData[0], 5, 1, tila))
			
			else:
				print("Tuntematon komento")
				MasterSocket.sendall(Luo_WFM(MasterData[0], 2, 1, tila))
		except IndexError:
			sleep(1)
			print("Yhteys poikki.")
			seis = True
			continue
		# except socket.error as msg:
			# print("Socket error: ", msg)
			# print("Yhdistetään uudestaan.")
			# MasterSocket.connect((MasterIP, MasterPort))
			# MasterSocket.send(Luo_NCM(3, ID, 2))
			# continue
		except KeyboardInterrupt:
			MasterSocket.close()
			seis = True
	
	print("Valmis")

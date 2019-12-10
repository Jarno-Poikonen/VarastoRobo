# -*- coding: utf-8 -*-
import socket
from time import sleep
from datetime import datetime
import threading
import RPi.GPIO as GPIO

MasterIP = None
MasterPort = None
seis = False

# Funktio jolla tulostetaan myös viestin ajan hetki
def Lokiin(mika, viesti):
	print(datetime.now(), " - ", mika, ": ", viesti)

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
	
	GPIO.setmode(GPIO.BCM)
	GPIO.setup(26, GPIO.OUT)
	GPIO.output(26, GPIO.LOW)
	global seis
	
	# Luodaan socket broadcastia varten.
	Lokiin("Broadcast", "Aloitus")
	bSocket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
	bSocket.settimeout(10)
	bSocket.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, True)
	bSocket.bind(("", 1732))
	
	# Luetaan broadcast viestejä IP:n selvittämiseksi.
	wait_for_message = True
	while wait_for_message :
		try:
			message, master_address = bSocket.recvfrom(512)
		except socket.timeout:
			Lokiin("Broad", "Timeout")
			continue
		
		if len(message) >= 8 and message[0] == 1 and message[1] == 7 :

			system_status = int(message[2])
			master_id = int(message[3])
			wait_for_message = False
			global MasterIP
			global MasterPort 
			MasterIP, MasterPort = master_address
			MasterPort = 1739
	
	# Luetaan broadcast viestejä ja reagoidaan hätäseis käskyyn.
	while True:
		try:
			message, master_address = bSocket.recvfrom(512)
		except timeout:
			Lokiin("Broad", "Timeout")
			message = str.encode("")
		if seis:
			GPIO.output(26, GPIO.HIGH)
			sleep(5)
			GPIO.output(26, GPIO.LOW)
			GPIO.cleanup()
			break
		if len(message) >= 8 and message[0] == 1 and message[1] == 7 :
			system_status = int(message[2])
			if system_status == 0 or system_status == 4:
				seis = True
				Lokiin("Broad", "SEIS")
				Lokiin("Broad", system_status)
				GPIO.output(26, GPIO.HIGH)
				sleep(5)
				GPIO.output(26, GPIO.LOW)
				GPIO.cleanup()
				break

# Luodaan WFM viesti annetuista parametreista.
def Luo_NCM(tyyppi, id, tila):
	viesti = bytearray([2])
	viestin_loppu = bytearray([tyyppi, id, 0, 0, 0, tila])
	pituus = Laske_pituus(len(viestin_loppu))
	
	for b in pituus:
		viesti.append(b)
		
	for b in viestin_loppu:
		viesti.append(b)
	
	Lokiin("NCM", viesti)
	
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
	
	Lokiin("WFM", viesti)
	
	return viesti

if __name__ == "__main__":
	
	# Mahdolliset virheet
	virheet = {
	"Ei virheitä": 0,
	"Viesti viallinen": 1,
	"Ei tueta": 2,
	"Param virheelliset": 3,
	"Kohdetta ei löytynyt": 4,
	"Kieltäytyi": 5,
	"Vikatila": 6,
	"Resurssit ei riitä": 7,
	"Pysäytetty": 8,
	"Reittiä ei olemassa": 9,
	"Reittiä ei voi kulkea": 10
	}
	
	# Käsien paikat.
	# Ensimämisenä tuotteen numero, sen jälkeen 6 float arvoa servojen asennoille.
	paikat = [
	str.encode(",4.25055,-2.26264,-1.68326,-0.763072,1.57923,1.09164)"),
	str.encode(",3.9939,-2.01335,-2.21448,-0.482631,1.59763,0.872768)"),
	str.encode(",3.51784,-1.8909,-2.4487,-0.383809,1.59626,0.396642)")
	]
	
	broadcast_saie = threading.Thread(target=Broadcast_communication, args=())
	broadcast_saie.start()
	
	ID = 55
	tila = 1
	
	Lokiin("Main", "Odotetaan UR5 yhteyttä.")
	
	URIP = "192.168.100.11"
	URPort = 30000
	
	URSocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
	URSocket.bind((URIP, URPort))
	URSocket.listen()
	
	URYhteys, URAddr = URSocket.accept()
	
	Lokiin("Main", "Odotetaan Masterin IP.")
	while MasterIP is None:
		pass
	Lokiin("Main", "IP saatu.")
	
	MasterSocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
	MasterSocket.connect((MasterIP, MasterPort))
	
	MasterSocket.send(Luo_NCM(3, ID, 2))
	
	# Niin kauan kuin seis ei ole True niin kuunnellaan mestarilta käskyjä ja toteutetaan ne if elif lausekkeissa.
	while not seis:
		Lokiin("Main", "Odotetaan Masterin viestejä.")
		MasterData = MasterSocket.recv(512)
		Lokiin("Main", MasterData)
		try:
			if MasterData[0] == 3:		# Setup Connection Message
				Lokiin("Main", "Setup Connection Message saatu")
				ID = MasterData[6]
				MasterSocket.sendall(Luo_WFM(MasterData[0], virheet["Ei virheitä"], 1, tila))
				
			elif MasterData[0] == 5:	# Closed Connection Message
				Lokiin("Main", "Closed Connection Message saatu")
				MasterSocket.sendall(Luo_WFM(MasterData[0], virheet["Ei tueta"], 1, tila))
				
			elif MasterData[0] == 6:	# Status Query Message
				Lokiin("Main", "Status Query Message saatu")
				MasterSocket.sendall(Luo_WFM(MasterData[0], virheet["Ei virheitä"], 1, tila))
				
			elif MasterData[0] == 7:	# System Startup Message
				Lokiin("Main", "System Shutdown Message saatu")
				tila = 1
				MasterSocket.sendall(Luo_WFM(MasterData[0], virheet["Ei virheitä"], 1, tila))
				
			elif MasterData[0] == 8:	# System Shutdown Message
				Lokiin("Main", "System Shutdown Message saatu")
				tila = 2
				MasterSocket.sendall(Luo_WFM(MasterData[0], virheet["Ei virheitä"], 1, tila))
				
			elif MasterData[0] == 12:	# Move Product Message
				Lokiin("Main", "Move Product Message saatu")
				
				data = str.encode("(") + str.encode(str(MasterData[5])) + paikat[MasterData[6]]
				Lokiin("Main -> UR5", data)
				URYhteys.sendall(data)
				valmis = False
				while not valmis:
					
					URData = URYhteys.recv(512)
					
					utf = URData.decode("utf-8")
					Lokiin("Main <- UR5", utf)
					
					if "Valmis" in utf:
						MasterSocket.sendall(Luo_WFM(MasterData[0], virheet["Ei virheitä"], 1, tila))
						valmis = True
					elif "Fail" in utf:
						MasterSocket.sendall(Luo_WFM(MasterData[0], virheet["Kohdetta ei löytynyt"], 1, tila))
						valmis = True
					elif not utf:
						raise IndexError
				
			elif MasterData[0] == 9 or MasterData[0] == 10 or MasterData[0] == 11 or MasterData[0] == 13 or MasterData[0] == 14:
				Lokiin("Main", "Ei tuettu komento")
				print(MasterData[0])
				MasterSocket.sendall(Luo_WFM(MasterData[0], virheet["Kieltäytyi"], 1, tila))
			
			else:
				Lokiin("Main", "Tuntematon komento")
				MasterSocket.sendall(Luo_WFM(MasterData[0], virheet["Ei tueta"], 1, tila))
		
		# Jos saadaaLokiinn IndexError niin oletetaan sen johtuvan siitä että mestarilta saatiin tyhjä tieto minkä oletetaan tarkoittavan yhteyden poikki olemista.
		except IndexError:
			sleep(1)
			Lokiin("Main", "Yhteys poikki. Sammutetaan laitteisto.")
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
	MasterSocket.close()
	URSocket.close()
	Lokiin("Main", "Valmis")

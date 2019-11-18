# -*- coding: utf-8 -*-
import socket
from time import sleep
import threading

'''
Käynnistys:
- Luodaan yhteys UR5:een ja aletaan kuuntelemaan broadcast viestejä.
- Broadcast viestistä luetaan masterin ip osoite ja lähetetään se master säikeelle.
- Master säie luo yhteyden Masteriin ja alkaa kuuntelemaan sen viestejä välittäen ne UR5 säikeelle.

Case 1 - Anna paketti:
- Saadaan Masterilta viesti "Anna paketti x paikkaan y"
- Välitetaan viesti viesti taulukoita käyttäen UR5 säikeelle
- Odotetaan "valmis" viestiä UR5:lta.
- Lähetetään "valmis" viesti Masterille.

Case 2 - Seis:
- Broadcast viesteistä saadaan seis käsky.
- Pysäytetään kaikki.
'''

# Luetaan annetusta jonosta uusin viesti ja poistetaan se.
def Lue_viesti(jono, paikka=0):
	viesti = jono[paikka]
	jono.pop(jono.index(viesti))
	return viesti

# Mahdollista tehdä paremmin.
# Muutetaan viestin pituus esitettäväksi neljällä bytellä
def Laske_pituus(viesti, pituus=4):
	
	palautus = bytearray([])
	
	for x in range(pituus-1):
		palautus.append(0)
	
	palautus.append(viesti)
	
	return palautus

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

# Luodaan NCM viesti annetuista parametreista.
def Luo_NCM(tyyppi, id, tila):
	viesti = bytearray([2])
	viestin_loppu = bytearray([tyyppi, id, 0, 0, 0, tila])
	pituus = Laske_pituus(len(viestin_loppu))
	
	for b in pituus:
		viesti.append(b)
		
	for b in viestin_loppu:
		viesti.append(b)
	
	return viesti

# Säie/funktio joka kuuntelee broadcast viestejä.
def Broadcast_communication(viestit):
	# Luodaan socket broadcastia varten.
	
	bSocket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
	bSocket.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, True)
	bSocket.bind(("0.0.0.0", 1732))
	
	# Luetaan broadcast viestejä IP:n selvittämiseksi.
	wait_for_message = True
	while wait_for_message :

		message, master_address = bSocket.recvfrom(512)
		
		if len(message) >= 8 and message[0] == 1 and message[1] == 7 :

			system_status = int(message[2])
			master_id = int(message[3])
			wait_for_message = False
			if not yhteysMasteriin:
				viestit.append((1, "IP", master_address))
	
	# Luetaan broadcast viestejä ja reagoidaan hätäseis käskyyn.
	while True:
		message, master_address = bSocket.recvfrom(512)
		
		if len(message) >= 8 and message[0] == 1 and message[1] == 7 :
			system_status = int(message[2])
			if system_status != 1:
				seis = True
				break
	
# Säie/funktio joka huolehtii masterille ja UR5:lle kommunikoinnista.
def Master_communication(viestit):
	kaskyKesken = False
	
	#Mestarin IP ja portti
	Mip = ""
	Mport = None
	
	# Odotetaan Masterin IP
	while True:
		while len(viestit) == 0:
			pass
		
		if viestit[0][0] == 0 and viestit[0][1] == "IP":
			Mip = viestit[0][2][0]
			Mport = viestit[0][2][1]
			viestit.pop(0)
		
		MasterSocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
		MasterSocket.bind(Mip, Mport)
		MasterSocket.listen()
	
	while True:
		yhteys, osoite = MasterSocket.accept()
		with yhteys:
			print(yhteys)
			
			yhteys.sendall(Luo_NCM(3, 55, 2))
			
			while True:
				data = yhteys.recv(512)
				
				if data[0] == 3: # Setup Connection Message
					id = str(data[6])
					yhteys.sendall(Luo_WFM(data[0], 0, 1, 1))
				
				if data[0] == 12: # Move Product Message
					tuote = data[5]
					paamaara = data[6]
					
					viestit.append([1, "Tuote", tuote, paamaara])
					kaskyKesken = True
				
				if viestit[0][0] == 0 and viestit[0][1] == "Tuote":
					viesti = Lue_viesti(viestit)
					if viesti[2] == "Valmis":
						yhteys.sendall(Luo_WFM(12, 0, 1, 1))
					elif viesti[2] == "Fail": # Pakettia ei löytynyt
						yhteys.sendall(Luo_WFM(12, 4, 0, 1))
					kaskyKesken = False
				
				if viestit[0][1] == "Seis":
					# Ei ehkä tarpeellinen.
					yhteys.sendall("Seis")
					break
	
def UR5_communication(viestit):
	#Gateway laitteen IP ja portti UR5:en suuntaan
	URip = "192.168.100.11"
	URport = 30000
	
	
	URsocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
	URsocket.setblocking(0) # Non-blockinf
	URsocket.bind((URip, URport))
	URsocket.listen()
	
	while True:
		# Luetaan viestejä.
		
		# Jos ei ole viestejä ei tehdä mitään.
		# if len(viestit) == 0:
			# continue
		
		try:
			yhteys, osoite = URsocket.accept()
		except Exception as e:
			sleep(1)
			continue
		
		with yhteys:
			print(yhteys)
			
			while True:
				# Mitä yhteydellä tehdään.
				
				try:
					data = yhteys.recv(512)
					print("UR5: ", data)
				except socket.error:
					data = str.encode("")
				except Exception as e:
					print(e)
				
				try:
					if viestit[0][0] == 0:
						viesti = Lue_viesti(viestit)
						yhteys.sendall(viesti[3])
				except:
					pass
				
				# if viestit[0][1] == "Seis":
					# yhteys.sendall("Seis")
					# break
				
				if data == str.encode("Valmis"):
					viestit.append((1, "Tuote", "Valmis"))
					break
				
				if data == str.encode("Fail"):
					viestit.append((1, "Tuote", "Fail"))
				
				# data = yhteys.recv(512)
				# print("UR5: ", data)
				if data == str.encode("Kiinni") or data == str.encode("Ei objekteja"):
					break
				if not data:
					print("Kpois")
					break
				# elif data == "Ei objekteja"
		break

yhteysMasteriin = False
seis = False

# Main säie huolehtii logiikasta.
if __name__ == "__main__":
	# Käsien paikat.
	paikat = [
	#str.encode("(-0.586169,-0.413107,0.242636,-1.91015,-1.85181,0.584344)"),
	str.encode("(-0.586169,-0.413107,0.242636,-1.91015,-1.85181,0.584344)"),
	str.encode("(5.89284,-2.85641,-1.68474,-0.149206,1.56736,-0.316181)")
	]

	# Luodaan viestimis listat, säikeet ja käynnistetään säikeet.
	# viestien rakenne = (0/1, "nimi", data)
	# 0/1 kummasta suunnasta viesti tulee. 0 = main, 1 = säie
	# "nimi" käskyn nimi minkä mukaan tehdään päätöksiä.
	# data viestin data esim. IP osoite.
	# "nimi" = komento
	broadcast_viestit = []
	master_viestit = []
	ur5_viestit = []
	
	broadcast_saie = threading.Thread(target=Broadcast_communication, args=(broadcast_viestit,))
	broadcast_saie.start()
	
	master_saie = threading.Thread(target=Master_communication, args=(master_viestit,))
	master_saie.start()
	
	ur5_saie = threading.Thread(target=UR5_communication, args=(ur5_viestit,))
	ur5_saie.start()
	
	'''
	# Odotetaan että broadcast_saie saa masterin IP:n selville.
	while len(broadcast_viestit) == 0:
		pass
	
	master_viestit.append(broadcast_viestit[0])
	broadcast_viestit.pop(0)
	'''
	
	while True:
		# Käydään läpi broadcast viestit.
		if len(broadcast_viestit) != 0:
			if broadcast_viestit[0][0] == 1:
				# Tarkista tämän toimivuus.
				viesti = Lue_viesti(broadcast_viestit)
				print("Broadcast viesti: ", viesti)
				if viesti[1] == "IP":
					master_viestit.append((0, "IP", viesti[2]))
					
		# Käydään läpi master viestit.
		if len(master_viestit) != 0:
			if master_viestit[0][0] == 1:
				viesti = Lue_viesti(master_viestit)
				print("Master viesti: ", viesti)
				
				if viesti[1] == "Tuote":
					ur5_viestit.append((0, "Tuote", viesti[2], paikat[viesti[3]]))
					
		# Käydään läpi UR5 viestejä.
		if len(ur5_viestit) != 0:
			if ur5_viestit[0] == 1:
				viesti = Lue_viesti(ur5_viestit)
				print("UR5 viesti: ", viesti)
				pass
		# Kaikki seis.
		if seis:
			print("SEIS")
			# Broadcast osa turha koska säie sulkee itsensä hätä seis viestin jälkeen.
			#broadcast_viestit.clear()
			#broadcast_viestit.append((0, "Seis", 0))
			master_viestit.clear()
			master_viestit.append((0, "Seis", 0))
			ur5_viestit.clear()
			ur5_viestit.append((0, "Seis", 0))
			break
			
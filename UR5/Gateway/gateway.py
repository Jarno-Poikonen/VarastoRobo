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
			if not yhteysMasteriin: viestit.append((1, "IP", master_address))
	
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
	#Mestarin IP ja portti
	Mip = ""
	Mport = None
	
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
	
	# Lähetetään New Connection Message
	
	while True:
		# Kuunnellaan masteria.
		yhteys, osoite = MasterSocket.accept()
		with yhteys:
			print(yhteys)
			while True:
				if viestit[0][1] == "Seis":
					# Ei ehkä tarpeellinen.
					yhteys.sendall("Seis")
					break
			
def UR5_communication(viestit):
	#Gateway laitteen IP ja portti UR5:en suuntaan
	URip = "192.168.100.11"
	URport = 30000
	
	
	URsocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
	URsocket.bind(URip, URport)
	URsocket.listen()
	
	while True:
		# Luetaan viestejä.
		
		# Jos ei ole viestejä ei tehdä mitään.
		if len(viestit) == 0:
			continue
		
		
		yhteys, osoite = URsocket.accept()
		with yhteys:
			print(yhteys)
			while True:
				# Mitä yhteydellä tehdään.
				
				if viestit[0][1] == "Seis":
					yhteys.sendall("Seis")
					break
				
				# data = yhteys.recv(512)
				# print("UR5: ", data)
				# if data == "Kiinni":
					# break
				# elif data == "Ei objekteja"

yhteysMasteriin = False
seis = False

# Main säie huolehtii logiikasta.
if __name__ == "__main__":
	# Käsien paikat.
	paikat = [
	str.encode("(-0.586169,-0.413107,0.242636,-1.91015,-1.85181,0.584344)"),
	str.encode("(-0.586169,-0.413107,0.242636,-1.91015,-1.85181,0.584344)"),
	str.encode("(-0.586169,-0.413107,0.242636,-1.91015,-1.85181,0.584344)"),
	str.encode("(-0.586169,-0.413107,0.242636,-1.91015,-1.85181,0.584344)")
	]

	# Luodaan viestimis listat, säikeet ja käynnistetään säikeet.
	# viestien rakenne = (0/1, "nimi", data)
	# 0/1 kummasta suunnasta viesti tulee. 0 = main, 1 = säie
	# "nimi" käskyn nimi minkä mukaan tehdään päätöksiä.
	# data viestin data esim. IP osoite.
	broadcast_viestit = []
	master_viestit = []
	ur5_viestit = []
	
	broadcast_saie = threading.Thread(target=Broadcast_communication, args=(broadcast_viestit,))
	broadcast_saie.start()
	
	master_saie = threading.Thread(target=Master_communication, args=(master_viestit))
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
				viesti = broadcast_viestit[0]
				print("Broadcast viesti: ", viesti)
				broadcast_viestit.pop(broadcast_viestit.index(viesti))
				if viesti[1] == "IP":
					master_viestit.append((0, "IP", viesti[2]))
					
		# Käydään läpi master viestit.
		if len(master_viestit) != 0:
			if master_viestit[0] == 1:
				viesti = master_viestit[0]
				print("Master viesti: ", viesti)
				pass
		# Käydään läpi UR5 viestejä.
		if len(ur5_viestit) != 0:
			if ur5_viestit[0] == 1:
				viesti = ur5_viestit[0]
				print("UR5 viesti: ", viesti)
				pass
		# Kaikki seis.
		if seis:
			print("SEIS")
			# Broadcast osa turha koska se sulkee itsensä hätä seis viestin jälkeen.
			#broadcast_viestit.clear()
			#broadcast_viestit.append((0, "Seis", 0))
			master_viestit.clear()
			master_viestit.append((0, "Seis", 0))
			ur5_viestit.clear()
			ur5_viestit.append((0, "Seis", 0))
			break
			
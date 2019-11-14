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
			if not yhteysMasteriin: viestit.append(master_address)
	
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
	
	while len(viestit) == 0:
		pass
	
	Mip = viestit[0][0]
	Mport = viestit[0][1]
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
	broadcast_viestit = []
	master_viestit = []
	ur5_viestit = []
	
	broadcast_saie = threading.Thread(target=Broadcast_communication, args=(broadcast_viestit,))
	broadcast_saie.start()
	
	master_saie = threading.Thread(target=Master_communication, args=(master_viestit))
	master_saie.start()
	
	ur5_saie = threading.Thread(target=UR5_communication, args=(ur5_viestit,))
	ur5_saie.start()
	
	# Odotetaan että broadcast_saie saa masterin IP:n selville.
	while len(broadcast_viestit) == 0:
		pass
	
	master_viestit.append(broadcast_viestit[0])
	broadcast_viestit.pop(0)
	
	while True:
		if len(broadcast_viestit) != 0:
			# Käydään läpi broadcast viestit.
		if len(master_viestit) != 0:
			# Käydään läpi master viestit.
		if len(ur5_viestit) != 0:
			# Käydään läpi UR5 viestejä.
		if seis:
			# Kaikki seis.

# -*- coding: utf-8 -*-
import socket
from time import sleep
import threading

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
				# SEIS.
				pass
	
# Säie/funktio joka huolehtii masterille ja UR5:lle kommunikoinnista.
def Instruction_communication(viestit):
	#Gateway laitteen IP ja portti UR5:en suuntaan
	URip = "192.168.100.11"
	URport = 30000
	
	#Mestarin IP ja portti
	Mip = ""
	Mport = None
	
	URsocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
	URsocket.bind(URip, URport)
	URsocket.listen()
	
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
		# Vuorotellen kuunnellaan masteria ja UR5:stä
		pass
	
yhteysMasteriin = False

# Main säie huolehtii logiikasta.
if __name__ == "__main__":
	# Mahdollisten muuttujien alustus
	broadcast_viestit = []
	instruction_viestit = []
	
	broadcast_saie = threading.Thread(target=Broadcast_communication, args=(broadcast_viestit,))
	broadcast_saie.start()
	
	instruction_saie = threading.Thread(target=Instruction_communication, args=(instruction_viestit))
	instruction_saie.start()
	
	while len(broadcast_viestit) == 0:
		pass
	
	instruction_viestit.append(broadcast_viestit[0])
	broadcast_viestit.pop(0)
	
	while True:
		if len(instruction_viestit) != 0:
			# Käydään läpi kommukointi viestit.
		if len(broadcast_viestit) != 0:
			# Käydään läpi broadcast viestit.

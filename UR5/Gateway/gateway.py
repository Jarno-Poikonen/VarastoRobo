# -*- coding: utf-8 -*-
import socket
from time import sleep
import threading

# Säie/funktio joka kuuntelee broadcast viestejä.
def Broadcast_communication(viestit):
	# Luetaan broadcast viestejä ja tutkitaan sieltä mestain IP osoite.
	# Luetaan broadcast viestejä ja reagoidaan hätäseis käskyyn.
	
# Säie/funktio joka huolehtii masterille ja UR5:lle kommunikoinnista.
def Instruction_communication(viestit):
	#Gateway laitteen IP ja portti UR5:en suuntaan
	URip = "192.168.100.11"
	URport = 30000
	
	#Mestarin IP ja portti
	Mip = ""
	Mport = None
	
	
	
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
	
	while True:
		if len(instruction_viestit) != 0:
			# Käydään läpi kommukointi viestit.
		if len(broadcast_viestit) != 0:
			# Käydään läpi broadcast viestit.

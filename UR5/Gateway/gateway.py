# -*- coding: utf-8 -*-
import socket
from time import sleep
import threading

# Säie/funktio joka kuuntelee broadcast viestejä.
def Broadcast_communication:
	pass
	
# Säie/funktio joka huolehtii masterille ja UR5:lle kommunikoinnista.
def Instruction_communication:
	pass

# Main säie huolehtii logiikasta.
if __name__ == "__main__":
	# Mahdollisten muuttujien alustus
	
	broadcast_saie = threading.Thread(target=Broadcast_communication, args=())
	broadcast_saie.start()
	
	instruction_saie = threading.Thread(target=Instruction_communication, args=())
	instruction_saie.start()
	
	
	

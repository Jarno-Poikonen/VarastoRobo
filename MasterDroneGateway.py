

import socket
from time import sleep


InPort = 1732
OutHost = '192.168.1.20'
OutPort = 5000


incoming = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)           # UDP
incoming.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, True)     # Enable port reusage
incoming.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, True)     # Enable broadcasting mode
incoming.bind(('', InPort))

outgoing = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
outgoing.connect((OutHost,OutPort))



def CheckStopMessage():
    data, addr = incoming.recvfrom(5)
    print("Received: %s" % data)    
    return data[2] == 0



print("Listening...")


while True:
    if CheckStopMessage():
        outgoing.send(str.encode("STOP\n"))
        print("Stop message forwarded!")
        sleep(1.5)



# Currently never reached

print("Exiting.")
incoming.close()
outgoing.close()


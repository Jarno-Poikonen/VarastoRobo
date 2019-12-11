#example python script for emergency stop by Santtu Nyman.

import socket

def emergency_stop():

    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, True)
    sock.sendto(bytearray(b'\x01\x07\x00\x00\x00\x00\x00\x00'), ("255.255.255.255", 1732))
    sock.shutdown(socket.SHUT_RDWR)
    sock.close()
   
emergency_stop()
exit()

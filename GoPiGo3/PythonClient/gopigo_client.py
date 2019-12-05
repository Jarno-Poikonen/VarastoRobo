# -*- coding: utf-8 -*-
import socket
import time
import threading
import ClientControlled
import kuvantunnistus

masterIp = None
emergencyStop = False
no_packet = 255

#Listens SBM and emergency stop messages
def udp_broadcast():
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, True)
    sock.bind(("", 1732))
    waitMessage = True
    while waitMessage == True:
        message, masterAddress = sock.recvfrom(512)
        message = bytearray(message) 

        if len(message) >= 8 and message[0] == 1 and message[1] == 7:
            masterId = message[3]           
            if masterId != 0:
                global masterIp
                masterIp = str(masterAddress[0])
                print(masterIp)
                waitMessage = False
        
    while True:
        message, masterAddress = sock.recvfrom(512)
        message = bytearray(message)
        
        if len(message) >= 8 and message[0] == 1 and message[1] == 7:
            masterId = message[3]
            if masterId == 0 and message[2] != 1:
                global emergencyStop
                #print("Hataseis!!")
                emergencyStop = True
                while True:
                    ClientControlled.gpg.stop()

def form_NCM(devType,devID,cordX,cordY,direction,devState):
    content_length = 6
    
    message = bytearray([2])
    message.extend(bytearray((content_length).to_bytes(4, 'little')))
    message.extend(bytearray([devType,devID,cordX,cordY,direction,devState]))
    
    return message

def form_WFM(commandNum,errorCode,atom,cordX,cordY,direction,devState,packetNum):
    content_length = 7
    if packetNum != no_packet:
        content_length += 1
        
    message = bytearray([4])
    message.extend(bytearray((content_length).to_bytes(4, 'little')))
    message.extend(bytearray([commandNum,errorCode,atom,cordX,cordY,direction,devState]))
    if packetNum != no_packet:
        message.append(packetNum)
        
    return message

def main():
    disconnect = 0
    deviceState = 0
    latestDir = 0
    errorCode = 0
    carriedPacket = no_packet
    
    broadcast_thread = threading.Thread(target=udp_broadcast,args=())
    broadcast_thread.start()

    while masterIp is None:
        pass
    
    masterSocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    masterSocket.connect((masterIp, 1739))
    pos, orientation = ClientControlled.PosOri()
    message = form_NCM(2,255,pos[0],pos[1],orientation,deviceState)
    masterSocket.send(message)
    
    while disconnect == 0 and emergencyStop == False:
        fromMaster = masterSocket.recv(512)
        #print("Masterin vastaus",fromMaster)

        #SCM
        if fromMaster[0] == 3:
            deviceID = fromMaster[6]
            deviceState = 1
            pos, orientation = ClientControlled.PosOri()
            carriedPacket = kuvantunnistus.kuvantunnistus()
            reply = form_WFM(fromMaster[0],0,1,pos[0],pos[1],orientation,deviceState,carriedPacket)
            masterSocket.send(reply)
            print("SCM vastattu")

        #CCM
        elif fromMaster[0] == 5:
            print("Vastaanotettu CCM")
            pos, orientation = ClientControlled.PosOri()
            carriedPacket = kuvantunnistus.kuvantunnistus()
            reply = form_WFM(fromMaster[0],0,1,pos[0],pos[1],orientation,deviceState,carriedPacket)
            masterSocket.send(reply)
            disconnect = 1

        #SQM    
        elif fromMaster[0] == 6:
            pos, orientation = ClientControlled.PosOri()
            carriedPacket = kuvantunnistus.kuvantunnistus()
            reply = form_WFM(fromMaster[0],0,1,pos[0],pos[1],orientation,deviceState,carriedPacket)
            masterSocket.send(reply)
            print("SQM vastattu")

        #MCM
        elif fromMaster[0] == 13 and deviceState == 1:
            print("MCM vastaanotettu")
            direction = fromMaster[5]
            errorCode = ClientControlled.Move(direction)
            pos, orientation = ClientControlled.PosOri()
            carriedPacket = kuvantunnistus.kuvantunnistus()
            reply = form_WFM(fromMaster[0],errorCode,1,pos[0],pos[1],orientation,deviceState,carriedPacket)
            masterSocket.send(reply)
        
        #Komentoa ei tunnistettu
        else:
            print("Komentoa ei tueta")
            pos, orientation = ClientControlled.PosOri()
            carriedPacket = kuvantunnistus.kuvantunnistus()
            reply = form_WFM(fromMaster[0],2,1,pos[0],pos[1],orientation,deviceState,carriedPacket)
            masterSocket.send(reply)
    
    masterSocket.close()
    print("Yhteys katkaistu")

if __name__ == "__main__":    
    main()
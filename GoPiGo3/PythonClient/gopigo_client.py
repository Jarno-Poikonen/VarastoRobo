# -*- coding: utf-8 -*-
import socket
import time
import threading
import Clientohjattu

masterIp = None 
deviceID = None
no_packet = 128

def count_length(message, length=4):   
    returnVar = bytearray([])
    returnVar.append(message)
    
    for x in range(length-1):
        returnVar.append(0)
    
    return returnVar

def udp_broadcast():
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, True)
    sock.bind(("", 1732))
    waitMessage = False
    while waitMessage == True:
        message, masterAddress = sock.recvfrom(512)
        message = bytearray(message)
        
        #SBM
        if len(message) >= 8 and message[0] == 1 and message[1] == 7:
            masterId = message[3]
            
            if masterId != 0:
                global masterIp
                masterIp = str(masterAddress[0])
                print(masterIp)
                waitMessage = False

def form_NCM(devType,devID,cordX,cordY,direction,devState):
    message = bytearray([2])
    mesEndPart = bytearray([devType,devID,cordX,cordY,direction,devState])
    length = count_length(len(mesEndPart))
    
    for i in length:
        message.append(i)
    
    for i in mesEndPart:
        message.append(i)
        
    return message

def form_WFM(commandNum,errorCode,atom,cordX,cordY,direction,devState,packetNum):
    content_length = 7
    if packetNum != no_packet:
        content_length = +1
        
    message = bytearray([4])
    message.extend(bytearray(content_length).to_bytes(4, 'little'))
    message.extend(bytearray([commandNum,errorCode,atom,cordX,cordY,direction,devState]))
    if packetNum != no_packet:
        message.append(packetNum)
        
    return message
    
def main():
    systemStop = 0
    deviceState = 0
    latestDir = 0
    carriedPacket = no_packet
    broadcast_thread = threading.Thread(target=udp_broadcast,args=())
    broadcast_thread.start()

    while masterIp is None:
        #print("ei IP:ta")
        #time.sleep(1)
        pass
    
    #print("Ip saatu")
    masterSocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    masterSocket.connect((masterIp, 1739))
    #print("Yhteys muodostettu")
    #systemStatus = 1
    message = form_NCM(2,255,0,0,0,deviceState)
    #print(message)
    masterSocket.send(message)
    
    while systemStop == 0:
        fromMaster = masterSocket.recv(512)
        print("Masterin vastaus",fromMaster)

        #SCM
        if fromMaster[0] == 3:
            deviceID = fromMaster[6]
            deviceState = 1
            pos, orientation = Clientohjattu.PosOri()
            reply = form_WFM(fromMaster[0],0,1,pos[0],pos[1],orientation,deviceState,carriedPacket)
            masterSocket.send(reply)
            #print(reply)
            print("SCM vastattu")

        #CCM
        elif fromMaster[0] == 5:
            print("Vastaanotettu CCM")
            pos, orientation = Clientohjattu.PosOri()
            reply = form_WFM(fromMaster[0],0,1,pos[0],pos[1],orientation,deviceState,carriedPacket)
            masterSocket.send(reply)
            systemStop = 1

        #SQM    
        elif fromMaster[0] == 6:
            pos, orientation = Clientohjattu.PosOri()
            reply = form_WFM(fromMaster[0],0,1,pos[0],pos[1],orientation,deviceState,carriedPacket)
            masterSocket.send(reply)
            print("SQM vastattu")

        #MCM
        elif fromMaster[0] == 13 and deviceState == 1:
            direction = fromMaster[5]
            Clientohjattu.Liiku(direction)
            pos, orientation = Clientohjattu.PosOri()
            reply = form_WFM(fromMaster[0],0,1,pos[0],pos[1],orientation,deviceState,carriedPacket)
            masterSocket.send(reply)
            
        else:
            print("Komentoa ei tueta/tunnistettu")
            pos, orientation = Clientohjattu.PosOri()
            reply = form_WFM(fromMaster[0],0,1,pos[0],pos[1],orientation,deviceState,carriedPacket)
            masterSocket.send(reply)
    
    masterSocket.close()
    print("Yhteys katkaistiin")   

if __name__ == "__main__":    
    main()
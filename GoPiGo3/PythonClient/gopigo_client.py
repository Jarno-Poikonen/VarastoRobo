# -*- coding: utf-8 -*-
import socket
import time
import threading
import gpgDrive

masterIp = None

deviceID = None

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
    waitMessage = True
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

def form_WFM(commandNum,errorCode,atom,cordX,cordY,direction,deviceState,packetNum):
    message = bytearray([4])
    mesEndPart = bytearray([commandNum,errorCode,atom,cordX,cordY,direction,deviceState,packetNum])
    length = count_length(len(mesEndPart))
    
    for i in length:
        message.append(i)
    
    for i in mesEndPart:
        message.append(i)
        
    return message
    
def main():
    systemStop = 0
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
    message = form_NCM(2,255,0,0,0,1)
    masterSocket.send(message)
    
    while systemStop == 0:
        fromMaster = masterSocket.recv(512)
        print("Masterin vastaus",fromMaster)
        
        #SCM
        if fromMaster[0] == 3:
            deviceID = fromMaster[6]
            #print(deviceID)
            pos = gpgDrive2.PosOri()
            print(pos[0])
            reply = form_WFM(3,0,1,pos[0],pos[1],0,1,0)
            masterSocket.send(reply)
            print(reply)
            print("SCM vastattu")
            #risteys = gpgDrive.Liiku(4)
            #print(risteys)
        
        #CCM
        if fromMaster[0] == 5:
            print("Vastaanotettu CCM")
            reply = form_WFM(5,0,1,0,0,0,1,0)
            masterSocket.send(reply)
            systemStop = 1
            
        #SQM    
        if fromMaster[0] == 6:
            reply = form_WFM(6,0,1,0,0,0,1,0)
            masterSocket.send(reply)
            print("SQM vastattu")
            
        
        #MCM
        if fromMaster[0] == 13:
            direction = fromMaster[5]
            
    
    masterSocket.close()
    print("Yhteys katkaistiin")   

if __name__ == "__main__":    
    main()
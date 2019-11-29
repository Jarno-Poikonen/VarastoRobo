# -*- coding: utf-8 -*-
import socket
import time
import threading

masterIp = None
systemStop = False
deviceID = None

def count_length(message, length=4):
    
    returnVar = bytearray([])    
    returnVar.append(message)
    
    for x in range(length-1):
        returnVar.append(0)
    
    return returnVar

def udp_broadcast():
    #Thread for broadcast messages
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
                #waitMessage = False
            
            else:
                systemStop = True

def form_NCM(devType,devID,devX,devY,direction,devState):
    message = bytearray([2])
    mesEndPart = bytearray([devType,devID,devX,devY,direction,devState])
    length = count_length(len(mesEndPart))
    
    for i in length:
        message.append(i)
    
    for i in mesEndPart:
        message.append(i)
        
    return message

def form_WFM(commandNum,errorCode,atom,cordX,cordY,direction,deviceState):
    message = bytearray([4])
    mesEndPart = bytearray([commandNum,errorCode,atom,cordX,cordY,direction,deviceState])
    length = count_length(len(mesEndPart))
    
    for i in length:
        message.append(i)
    
    for i in mesEndPart:
        message.append(i)
        
    return message

if __name__ == "__main__":    
    broadcast_thread = threading.Thread(target=udp_broadcast,args=())
    broadcast_thread.start()

    while masterIp is None:
        #print("ei IP:ta")
        #time.sleep(1)
        pass
    
    print("IP saatu")
    masterSocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    masterSocket.connect((masterIp, 1739))
    message = form_NCM(2,255,0,0,0,1)
    masterSocket.send(message)
    
    while not systemStop:
        fromMaster = masterSocket.recv(512)
        
        #SCM
        if fromMaster[0] == 3:
            deviceID = fromMaster[6]
            #print("%s %s %s %s %s %s %s"% tuple(fromMaster))
            #print(deviceID)
            reply = form_WFM(3,0,1,0,0,0,1)
            masterSocket.send(reply)
            #time.sleep(1)
            
        #SQM    
        #if fromMaster[0] == 6:
            
            
#example python script for reading five last log entries from VarastoRobotti master server by Santtu Nyman.

import socket
import math

def find_master_server():

    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, True)
    sock.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, True)
    sock.bind(("0.0.0.0", 1732))

    configuration = { "system_status" : 255, "master_device_id" : 255, "map_height" : 0, "map_width" : 0, "master_device_address" : "0.0.0.0", "map" : [], "block_list" : [], "device_list" : [] }
    wait_for_message = True
    while wait_for_message :

        message, master_address = sock.recvfrom(512)
        message = bytearray(message)
        
        if len(message) >= 8 and message[0] == 1 and message[1] == 7 :
        
            system_status = int(message[2])
            master_id = int(message[3])
            map_height = int(message[4])
            map_width = int(message[5])
            block_count = int(message[6])
            device_count = int(message[7])
            
            map_size = int(math.floor(((map_height * map_width) + 7) / 8))
            block_list_size = block_count * 2;
            device_list_size = device_count * 8;
            
            if master_id != 0 and (8 + map_size + block_list_size + device_list_size) == len(message) :
                
                configuration["system_status"] = system_status
                configuration["master_device_id"] = master_id
                configuration["map_height"] = map_height
                configuration["map_width"] = map_width
                configuration["master_device_address"] = str(master_address[0])
    
                for i in range(map_height * map_width) :
                    byte_index = 8 + int(math.floor(i / 8))
                    bit_index = i % 8;
                    location_value = ((message[byte_index] >> bit_index) & 1) == 1
                    configuration["map"].append(location_value)
    
                for i in range(block_count) :
                    block_x = int(message[8 + map_size + (i * 2) + 0])
                    block_y = int(message[8 + map_size + (i * 2) + 1])
                    configuration["block_list"].append({ "x" : block_x, "y" : block_y })
    
                for i in range(device_count) :
                    device_type = int(message[8 + map_size + block_list_size + (i * 8) + 0])
                    device_id = int(message[8 + map_size + block_list_size + (i * 8) + 1])
                    device_x = int(message[8 + map_size + block_list_size + (i * 8) + 2])
                    device_y = int(message[8 + map_size + block_list_size + (i * 8) + 3])
                    device_ip = str(int(message[8 + map_size + block_list_size + (i * 8) + 7])) + "." + str(int(message[8 + map_size + block_list_size + (i * 8) + 6])) + "." + str(int(message[8 + map_size + block_list_size + (i * 8) + 5])) + "." + str(int(message[8 + map_size + block_list_size + (i * 8) + 4]))
                    configuration["device_list"].append({ "type" : device_type, "id" : device_id, "x" : device_x, "y" : device_y, "ip" : device_ip })
                
                wait_for_message = False
    
    return configuration

def is_block_at_location(configuration, x, y):
    for i in range(len(configuration["block_list"])) :
        if x == configuration["block_list"][i]["x"] and y == configuration["block_list"][i]["y"]:
            return True
    return False
    
def is_gopigo_at_location(configuration, x, y):
    for i in range(len(configuration["device_list"])) :
        if configuration["device_list"][i]["type"] == 2 and x == configuration["device_list"][i]["x"] and y == configuration["device_list"][i]["y"]:
            return True
    return False

configuration = find_master_server()
print("Master device " + str(configuration["master_device_id"]) + " found at address " + configuration["master_device_address"])

sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
#assume socket created

sock.connect((configuration["master_device_address"], 1739))
#assume connection created

ncm_data = bytearray(b'\x02\x06\x00\x00\x00\x01\xFF\xFF\xFF\xFF\x01')
sock.send(ncm_data)
#assume ncm sent

scm_data = bytearray(sock.recv(4096))
#assume valid scm

pom_data = bytearray(b'\x0B\x03\x00\x00\x00\x01\x07\x01')
sock.send(pom_data)
#assume pom sent
print("send order for product id 1 to coordinate 7,1")

rlm_wfm_data = bytearray(sock.recv(4096))
#assume valid wfm

ccm_data = bytearray(b'\x05\x00\x00\x00\x00')
sock.send(ccm_data)
#assume ccm sent

ccm_wfm_data = bytearray(sock.recv(4096))
#assume valid wfm

exit()

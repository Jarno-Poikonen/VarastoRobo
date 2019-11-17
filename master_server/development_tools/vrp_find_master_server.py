#example python script for receiving VarastoRobotti system broadcast message (SMB) by Santtu Nyman.

import socket

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
    
            map_size = ((map_height * map_width) + 7) / 8
            block_list_size = block_count * 2;
            device_list_size = device_count * 8;
            
            if master_id != 0 and (8 + map_size + block_list_size + device_list_size) == len(message) :
                
                configuration["system_status"] = system_status
                configuration["master_device_id"] = master_id
                configuration["map_height"] = map_height
                configuration["map_width"] = map_width
                configuration["master_device_address"] = str(master_address[0])
    
                for i in range(map_height * map_width) :
                    byte_index = 8 + (i / 8)
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

print("map")
line = "     +"
for x in range(configuration["map_width"]) :
    line += "-"
line += "+"
print(line)
for y in range(configuration["map_height"]) :
    line = "    " + str(((configuration["map_height"] - 1) - y) % 10) + "|"
    for x in range(configuration["map_width"]) :
        if is_gopigo_at_location(configuration, x, ((configuration["map_height"] - 1) - y)):
            line += "G"
        elif is_block_at_location(configuration, x, ((configuration["map_height"] - 1) - y)):
            line += "B"
        elif configuration["map"][((configuration["map_height"] - 1) - y) * configuration["map_width"] + x] :
            line += "#"
        else:
            line += " "
    line += "|"
    print(line)
line = "     +"
for x in range(configuration["map_width"]) :
    line += "-"
line += "+"
print(line)
line = "      "
for x in range(configuration["map_width"]) :
    line += str((x % 10))
print(line)
  
print("blocks(B)")
for i in range(len(configuration["block_list"])) :
	print("    Block found at coordinates x=" + str(configuration["block_list"][i]["x"]) + ",y=" + str(configuration["block_list"][i]["y"]) + "")
    
print("devices(GoPiGo=G)")
for i in range(len(configuration["device_list"])) :
	print("    Device id=" + str(configuration["device_list"][i]["id"]) + " type=" + str(configuration["device_list"][i]["type"]) + " found at coordinates x=" + str(configuration["device_list"][i]["x"]) + ",y=" + str(configuration["device_list"][i]["y"]) + " and address " + str(configuration["device_list"][i]["ip"]) + "")
exit()

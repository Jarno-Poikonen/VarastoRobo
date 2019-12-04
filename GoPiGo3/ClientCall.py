import ClientControlled
while True: #For Console testing
    pos, ori = ClientControlled.PosOri()
    print("Position: ",pos)
    print("Orientation: ",ori)
    error_value = ClientControlled.Move(int(input("Give direction value: ")))
    print("Error value: ",error_value)
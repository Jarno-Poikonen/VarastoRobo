from __future__ import print_function
from __future__ import division
# the above lines are meant for Python3 compatibility.
# they force the use of Python3 functionality for print(), 
# the integer division and input()
# mind your parentheses!

import easygopigo3 as easy
import time

############

esteetaisyys = 230 #eteen
esteetaisyysvjo = 230 #vasen ja oikea


def lahella():
    gpg.stop()
    servo.rotate_servo(40)
    time.sleep(0.2)
    if etaisyys.read_mm() <= esteetaisyysvjo:
        gpg.stop()
        print("este 40, oikealla")

        while True:
            if etaisyys.read_mm() > esteetaisyysvjo:
                break

    servo.rotate_servo(95)
    time.sleep(0.2)
    if etaisyys.read_mm() <= esteetaisyys:
        gpg.stop()
        print("este 95")

        while True:
            if etaisyys.read_mm() > esteetaisyys:
                break

    servo.rotate_servo(150)
    time.sleep(0.2)
    if etaisyys.read_mm() <= esteetaisyysvjo:
        gpg.stop()
        print("este 150, vasemmalla")

        while True:
            if etaisyys.read_mm() > esteetaisyysvjo:
                break
                
    servo.rotate_servo(95)
    time.sleep(0.2)
    if etaisyys.read_mm() <= esteetaisyys:
        gpg.stop()
        print("este 95, keskella")

        while True:
            if etaisyys.read_mm() > esteetaisyys:
                break
                
##########

sensor_readings = None

gpg = easy.EasyGoPiGo3()
etaisyys = gpg.init_distance_sensor()
servo = gpg.init_servo("SERVO1")

try:
    my_linefollower = gpg.init_line_follower()
    time.sleep(0.1)
except:
    print('Line Follower not responding')
    time.sleep(0.2)
    exit()
my_linefollower.read_position()
my_linefollower.read_position()

#risteykset = [None, 0, 0, 0, 0, None]
#risteykset = [None, 1, 1, 1, 1, None]
#risteykset = [None, 0, 1, 1, 1, 0, 1, 1, 0, 2]
#risteykset = [None, None, 0, 0, None, None, 0, 2, 1, None, None, 1, 1, None, None, 2]
#risteykset = [0, 0, 1, 1, 0, 0, 3]
#risteykset = [0, 0, 0, 0, 3]
#risteykset = [None, None, 3]
#risteykset = [0, 1, 0, None, None, 1, None, None, None, 1, 0, None, 2, None, 1, 0, None, None, None, 0, None, None, 1, 0, 1, 2]
#risteykset = 0, 1, 0, 1, None, None, 1, 0, None, None, 2, None, None, 1, 0, None, None, 0, None, 1, 4
risteykset = None, None, 0, None, 1, None, 1, 0, None, None, 2, None, None, 1, 0, None, None, 0, None, 1,
vasen = False
oikea = False
monesRisteys = int(input("Mones risteys: "))
perus = 250
korjaus = 150
servo.rotate_servo(95)
gpg.set_speed(perus)
try:
    # start
    gpg.forward()
    while True:
        paikka = my_linefollower.read(representation="bivariate")
        print(paikka)
        if(paikka[0] == 0): vasen = True
        if(paikka[len(paikka)-1] == 0): oikea = True
        
        if(vasen or oikea):
            if vasen and oikea:
                print("Risteys vasemmalle ja oikealle.")
            elif vasen :
                print("Risteys vasemmalle.")
            elif oikea:
                print("Risteys oikealle.")
            
            print(monesRisteys)
            # gpg.stop()
            # suunta = int(input("Suunta (vasen:0, oikea:1, ympari:2 suoraan: 4):"))
            
            # if(suunta == 4):
                # suunta = None
                
            if(risteykset[monesRisteys] is None):
            #if(suunta is None):
                lahella()
                gpg.drive_cm(3)
            elif(risteykset[monesRisteys] == 2):
            #elif(suunta == 2):
                gpg.turn_degrees(180)
                ##########
                time.sleep(0.5)
                lahella()
                ##########
                #monesRisteys = -1
                #break
            elif(risteykset[monesRisteys] == 0):
            #elif(suunta == 0):
                servo.rotate_servo(179)
                gpg.blinker_on(1)
                lahella()
                
                gpg.drive_cm(6)
                gpg.turn_degrees(-90)
                ##########
                time.sleep(0.5)
                lahella()
                ##########
                
                gpg.blinker_off(1)
                servo.rotate_servo(95)
            elif(risteykset[monesRisteys] == 1):
            #elif(suunta == 1):
                servo.rotate_servo(10)
                gpg.blinker_on(0)
                lahella()
                
                gpg.drive_cm(6)
                gpg.turn_degrees(90)
                ##########
                time.sleep(0.5)
                lahella()
                ##########
                
                gpg.blinker_off(0)
                servo.rotate_servo(95)
            elif(risteykset[monesRisteys] == 3):
                break
            vasen = False
            oikea = False
            
            monesRisteys = monesRisteys + 1
            if (monesRisteys >= len(risteykset)):
                monesRisteys = 0
            
        if my_linefollower.read_position() == 'center':
            gpg.set_speed(perus)
            gpg.forward()
        if my_linefollower.read_position() == 'left':
            gpg.set_speed(korjaus)
            gpg.left()
        if my_linefollower.read_position() == 'right':
            gpg.set_speed(korjaus)
            gpg.right()
        if my_linefollower.read_position() == 'white':
            gpg.stop()
            
    gpg.stop()
except KeyboardInterrupt as e:
    print(e)
    gpg.stop()
except Exception as e:
    print(e)
    gpg.stop()

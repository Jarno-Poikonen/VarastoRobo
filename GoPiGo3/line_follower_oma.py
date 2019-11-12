from __future__ import print_function
from __future__ import division
# the above lines are meant for Python3 compatibility.
# they force the use of Python3 functionality for print(), 
# the integer division and input()
# mind your parentheses!

import easygopigo3 as easy
import time

def lahella():
    gpg.stop()
    time.sleep(0.5)
    if etaisyys.read_mm() <= 250:
        gpg.stop()
        print("ESTE")
        while True:
            if etaisyys.read_mm() > 150:
                break
                
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
risteykset = [None, 0, 1, 1, 1, 0, 1, 1, 0, 2]
vasen = False
oikea = False
monesRisteys = 0
servo.rotate_servo(95)
gpg.set_speed(200)
try:
    # start
    gpg.forward()
    while True:
        paikka = my_linefollower.read(representation="bivariate")
        
        if(paikka[0] == 0): vasen = True
        if(paikka[len(paikka)-1] == 0): oikea = True
        
        if(vasen or oikea):
            if vasen and oikea:
                print("Risteys vasemmalle ja oikealle.")
            elif vasen :
                print("Risteys vasemmalle.")
            elif oikea:
                print("Risteys oikealle.")
            
            if(risteykset[monesRisteys] is None):
                gpg.drive_cm(3)
            elif(risteykset[monesRisteys] == 2):
                gpg.turn_degrees(180)
                monesRisteys = -1
                break
            elif(risteykset[monesRisteys] == 0):
                servo.rotate_servo(179)
                gpg.blinker_on(1)
                lahella()
                
                gpg.drive_cm(6)
                gpg.turn_degrees(-90)
                
                gpg.blinker_off(1)
                servo.rotate_servo(95)
            elif(risteykset[monesRisteys] == 1):
                servo.rotate_servo(14)
                gpg.blinker_on(0)
                lahella()
                
                gpg.drive_cm(6)
                gpg.turn_degrees(90)
                
                gpg.blinker_off(0)
                servo.rotate_servo(95)
            vasen = False
            oikea = False
            
            monesRisteys = monesRisteys + 1
            
        if my_linefollower.read_position() == 'center':
            gpg.forward()
        if my_linefollower.read_position() == 'left':
            gpg.left()
        if my_linefollower.read_position() == 'right':
            gpg.right()
            
    gpg.stop()
except KeyboardInterrupt as e:
    print(e)
    gpg.stop()
except Exception as e:
    print(e)
    gpg.stop()

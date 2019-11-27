from __future__ import print_function
from __future__ import division
import traceback
# the above lines are meant for Python3 compatibility.
# they force the use of Python3 functionality for print(), 
# the integer division and input()
# mind your parentheses!

import easygopigo3 as easy
import time

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
######

Position = [0,0]    #Koordinaatita määritetään leveys,korkeus järjestyksessä
Orientation = 0     #Suunta 0 = Itä/Oikea, 1 = Pohjoinen/Ylös, 2 = Länsi/Vasen, 3 = Etelä/Alas
#def orientation()
    
#def position()

def Left():
    gpg.turn_degrees(-90)

def Right():
    gpg.turn_degrees(90)

def Around():
    gpg.turn_degrees(180)
    
def PosOri():
    print("Position:",Position)
    print("Orientation:",Orientation)
    return Position

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


perus = 250
korjaus = 150
servo.rotate_servo(95)
gpg.set_speed(perus)

def Turn(suunta):
    if(suunta == 4): #Suoraan
        suunta = None

    #if(suunta is None):
        #gpg.drive_cm(3)
        #Position[1] = Position[1]+1
        
    elif(suunta == 2): #Täyskäännös
        Around()
    
    elif(suunta == 3): #Täyskäännös parkkiin
        print("Parking");
        Around()
        #PosOri()
        #input("Parked. Push enter to go")

    elif(suunta == 0): #Vasen
        Left()
        
    elif(suunta == 1): #Oikea
        Right()

def Eteen():
    vasen = False
    oikea = False
    try:
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
                vasen = False
                oikea = False
                
                gpg.stop()
                gpg.drive_cm(6) #Ajaa risteyksestä hiukan eteenpäin, tarpeellinen pakettia noutaessa.
                gpg.stop()
                arvo = 0
                return arvo
                break
                
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
                input("HJALP, I'm Lost!!!")
                gpg.drive_cm(-6)
                
                
        gpg.stop()
    except Exception:
        gpg.stop()
        traceback.print_exc()
        
def Liiku(suunta):
    vasen = False
    oikea = False
    Turn(suunta)
    lahella()
    Eteen()
while True: #Konsolitestausta varten
    Liiku(suunta = int(input("Anna suunta-arvo")))
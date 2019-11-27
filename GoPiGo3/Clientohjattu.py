# -*- coding: utf-8 -*-
from __future__ import print_function
from __future__ import division
import traceback
# the above lines are meant for Python3 compatibility.
# they force the use of Python3 functionality for print(), 
# the integer division and input()
# mind your parentheses!

import easygopigo3 as easy
import time

esteetaisyysk = 230 #raja-arvo eteen 
esteetaisyysvjo = 230 #raja-arvo vasen ja oikea


etaisyyskeski = 0
etaisyysvasen = 0
etaisyysoikea = 0

estek = 0
estev = 0
esteo = 0

vasen = False
oikea = False

position = [0,0]    #Koordinaatit määritetään leveys,korkeus järjestyksessä
orientation = 0     #Orientaatio 0 = Itä/Oikea, 1 = Pohjoinen/Ylös, 2 = Länsi/Vasen, 3 = Etelä/Alas

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

######
def lahella():
    global estek
    global estev 
    global esteo
    
    gpg.stop()
    servo.rotate_servo(40)
    time.sleep(0.2)
    etaisyysoikea = etaisyys.read_mm()

    servo.rotate_servo(95)
    time.sleep(0.2)
    etaisyyskeski = etaisyys.read_mm()

    servo.rotate_servo(150)
    time.sleep(0.2)
    etaisyysvasen = etaisyys.read_mm()

    servo.rotate_servo(95)


    if etaisyysoikea <= esteetaisyysvjo:
        esteo = 1
        print("este oikealla")
        gpg.stop()
    if etaisyyskeski <= esteetaisyysk:
        estek = 1
        gpg.stop()
        print("este keskella")
    if etaisyysvasen <= esteetaisyysvjo:
        estev = 1
        gpg.stop()
        print("este vasemmalla")


    while esteo or estek or estev == 1:
            gpg.stop()
            esteo = 0
            estek = 0
            estev = 0
            break
            #Liiku(int(input("Anna suunta-arvo: ")))
######

def Left():
    gpg.turn_degrees(-90)

######
def Right():
    gpg.turn_degrees(90)

######
def Around():
    gpg.turn_degrees(180)

######
def PosOri():
    print("Position:",position)
    print("Orientation:",orientation)
    return (position, orientation)

######
def Orientation(): # Voinee poistaa, lähinnä debuggausta varten
    if(orientation == 0): #Oikea
        print("East")
    elif(orientation == 1): #Ylös
        print("North")
    elif(orientation == 2): #Vasen
        print("West")
    elif(orientation == 3): #Alas
        print("South")
    elif(orientation>3): #Lost
        print("Lost")

######
def Turn(suunta):    #Orientaatio 0 = Itä/Oikea, 1 = Pohjoinen/Ylös, 2 = Länsi/Vasen, 3 = Etelä/Alas
    global orientation
    global position
    Orientation()
    print(suunta)
    if(orientation == 0): #Suunta Itään
        if(suunta == 0): #Suoraan
            position[0] = position[0] + 1
        
        elif(suunta == 1): #Vasen
            Left()
            position[1] = position[1] + 1
    
        elif(suunta == 2): #Täyskäännös
            Around()
            position[0] = position[0] - 1

        elif(suunta == 3): #Oikea
            Right()
            position[1] = position[1] - 1
    elif(orientation == 1): #Suunta Pohjoiseen
        if(suunta == 1): #Suoraan
            position[1] = position[1] + 1
        
        elif(suunta == 2): #Vasen
            Left()
            position[0] = position[0] - 1
    
        elif(suunta == 3): #Täyskäännös
            Around()
            position[1] = position[1] - 1

        elif(suunta == 0): #Oikea
            Right()
            position[0] = position[0] + 1
    elif(orientation == 2): #Suunta Länteen
        if(suunta == 2): #Suoraan
            position[0] = position[0] - 1
        
        elif(suunta == 3): #Vasen
            Left()
            position[1] = position[1] - 1
    
        elif(suunta == 0): #Täyskäännös
            Around()
            position[0] = position[0] + 1

        elif(suunta == 1): #Oikea
            Right()
            position[1] = position[1] + 1
    elif(orientation == 3): #Suunta Etelään
        if(suunta == 3): #Suoraan
            position[1] = position[1] - 1
        
        elif(suunta == 0): #Vasen
            Left()
            position[0] = position[0] + 1
    
        elif(suunta == 1): #Täyskäännös
            Around()
            position[1] = position[1] + 1

        elif(suunta == 2): #Oikea
            Right()
            position[0] = position[0] - 1
    orientation = suunta

######
def Parkki():
    global orientation
    if(orientation == 3):
        Left()
    elif(orientation == 2):
        Around()
    elif(orientation == 1):
        Right()
    orientation = 0
######
def Eteen():
    global vasen
    global oikea
    try:
        while True:
            paikka = my_linefollower.read(representation="bivariate")
            print(paikka)
            if(paikka[0] == 0): vasen = True
            if(paikka[len(paikka)-1] == 0): oikea = True
            
            if(vasen or oikea):
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

######
def Liiku(suunta):
    Turn(suunta)
    lahella()
    Eteen()
    if(position == [0,0]):
        Parkki()
    PosOri()
while True: #Konsolitestausta varten
    PosOri()
    Liiku(int(input("Anna suunta-arvo: ")))
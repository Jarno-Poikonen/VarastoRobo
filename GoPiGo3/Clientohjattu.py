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
            Liiku(suunta = int(input("Anna suunta-arvo: ")))


position = [0,0]    #Koordinaatita määritetään leveys,korkeus järjestyksessä
orientation = 0     #Orientaatio 0 = Itä/Oikea, 1 = Pohjoinen/Ylös, 2 = Länsi/Vasen, 3 = Etelä/Alas

def Left():
    gpg.turn_degrees(-90)

def Right():
    gpg.turn_degrees(90)

def Around():
    gpg.turn_degrees(180)
    
def PosOri():
    print("Position:",position)
    print("Orientation:",orientation)
    return position

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
def Orientation():
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
        
def Turn(suunta):    #Orientaatio 0 = Itä/Oikea, 1 = Pohjoinen/Ylös, 2 = Länsi/Vasen, 3 = Etelä/Alas
    global orientation
    Orientation()
    print(suunta)
    if(orientation == 0): #Suunta Itään
        if(suunta == 0): #Suoraan
            pass
        
        elif(suunta == 1): #Vasen
            Left()
    
        elif(suunta == 2): #Täyskäännös
            Around()

        elif(suunta == 3): #Oikea
            Right()
    elif(orientation == 1): #Suunta Pohjoiseen
        if(suunta == 1): #Suoraan
            pass
        
        elif(suunta == 2): #Vasen
            Left()
    
        elif(suunta == 3): #Täyskäännös
            Around()

        elif(suunta == 0): #Oikea
            Right()
    elif(orientation == 2): #Suunta Länteen
        if(suunta == 2): #Suoraan
            pass
        
        elif(suunta == 3): #Vasen
            Left()
    
        elif(suunta == 0): #Täyskäännös
            Around()

        elif(suunta == 1): #Oikea
            Right()
    elif(orientation == 3): #Suunta Etelään
        if(suunta == 3): #Suoraan
            pass
        
        elif(suunta == 0): #Vasen
            Left()
    
        elif(suunta == 1): #Täyskäännös
            Around()

        elif(suunta == 2): #Oikea
            Right()            
    orientation = suunta

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
                #if vasen and oikea:
                #    print("Risteys vasemmalle ja oikealle.")
                #elif vasen :
                #    print("Risteys vasemmalle.")
                #elif oikea:
                #    print("Risteys oikealle.")
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
    Turn(suunta)
    lahella()
    Eteen()
    PosOri()
while True: #Konsolitestausta varten
    Liiku(suunta = int(input("Anna suunta-arvo: ")))
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
import esteentunnistus

Este = False
vasen = False
oikea = False
arvo = 0

Aloitus = [0,1]     #Aloituspisteen asetus muuttujaan. Koordinaatit järjestyksessä x,y
position = Aloitus.copy() #Kopioidaan aloituspisteen arvo position tracking muuttujaan
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
# my_linefollower.read_position()
# my_linefollower.read_position()

perus = 250
korjaus = 150
servo.rotate_servo(95)
gpg.set_speed(perus)

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
def Position(suunta):
    if(suunta == 0): #Suoraan
        position[0] = position[0] + 1
    
    elif(suunta == 1): #Vasen
        position[1] = position[1] + 1

    elif(suunta == 2): #Täyskäännös
        position[0] = position[0] - 1

    elif(suunta == 3): #Oikea
        position[1] = position[1] - 1
######
def Turn(suunta):    #Orientaatio 0 = Itä/Oikea, 1 = Pohjoinen/Ylös, 2 = Länsi/Vasen, 3 = Etelä/Alas
    global orientation
    global position
    #print(suunta)
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
def Eteen(suunta):
    global vasen
    global oikea
    global Este
    global arvo
    Este = False
    
    try:
        while True:
            paikka = my_linefollower.read(representation="bivariate")
            #print(paikka)
            if(paikka[0] == 0): vasen = True
            if(paikka[len(paikka)-1] == 0): oikea = True
            
            if(vasen or oikea):
                vasen = False
                oikea = False
                
                gpg.stop()
                gpg.drive_cm(6) #Ajaa risteyksestä hiukan eteenpäin, tarpeellinen pakettia noutaessa.

                gpg.stop()
                arvo = 0
                Position(suunta)
                #return arvo
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
                arvo = 9
                print("HJALP, I'm Lost!!!")
                break
                #gpg.drive_cm(-6) 
                
        gpg.stop()
    except Exception:
        gpg.stop()
        traceback.print_exc()

######
def Liiku(suunta):
    global Este
    Turn(suunta)
    global arvo
    Este = esteentunnistus.lahella()
    if(Este == False):
        Eteen(suunta)
        #Position(suunta)
    else:
        arvo = 10
        print("Este havaittu suunnassa: ",suunta)
    if(position == Aloitus):
        Parkki()
    print("Arvo: ",arvo)
    return arvo
while True: #Konsolitestausta varten
    PosOri()
    Liiku(int(input("Anna suunta-arvo: ")))
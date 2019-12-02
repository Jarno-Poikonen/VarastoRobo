# -*- coding: utf-8 -*-

#esteentunnistus.lahella() palauttaa esteen statuksen (true / false) sekä tulostaa, missä esteet ovat

from __future__ import print_function
from __future__ import division
import traceback
import easygopigo3 as easy
import time

esteetaisyysk = 230 #raja-arvo eteen 
esteetaisyysvjo = 180 #raja-arvo vasen ja oikea

etaisyyskeski = 0
etaisyysvasen = 0
etaisyysoikea = 0

estek = 0
estev = 0
esteo = 0
Este = False

######

gpg = easy.EasyGoPiGo3()
etaisyys = gpg.init_distance_sensor()
servo = gpg.init_servo("SERVO1")

######
def lahella():
    global estek
    global estev 
    global esteo
    global Este
    
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

    else:
        Este = False
    while esteo or estek or estev == 1:
            gpg.stop()
            esteo = 0
            estek = 0
            estev = 0
            Este = True
    return Este
######
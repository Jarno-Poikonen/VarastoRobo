# -*- coding: utf-8 -*-

#esteentunnistus.lahella() palauttaa esteen statuksen (true / false) sekä tulostaa, missä esteet ovat

from __future__ import print_function
from __future__ import division
import traceback
import easygopigo3 as easy
import time

esteetaisyysk = 210 #raja-arvo eteen 
esteetaisyysvjo25 = 230 #raja-arvo vasen ja oikea 25 astetta
esteetaisyysvjo45 = 110 #raja-arvo vasen ja oikea 45 astetta
odotusaika = 0.03 # servon kaantamisen time sleep

etaisyyskeski = 0
etaisyysvasen25 = 0
etaisyysvasen45 = 0
etaisyysoikea25 = 0
etaisyysoikea45 = 0

estek = 0
estev25 = 0
estev45 = 0
esteo25 = 0
esteo45 = 0
Este = False

######

gpg = easy.EasyGoPiGo3()
etaisyys = gpg.init_distance_sensor()
servo = gpg.init_servo("SERVO1")

######
def lahella():
    global estek
    global estev25
    global estev45    
    global esteo25
    global esteo45
    global Este
    
    gpg.stop()
    servo.rotate_servo(50) #45 astetta oikealle
    time.sleep(odotusaika)
    etaisyysoikea45 = etaisyys.read_mm()
    
    servo.rotate_servo(75) #25 astetta oikealle
    time.sleep(odotusaika)
    etaisyysoikea25 = etaisyys.read_mm()

    servo.rotate_servo(95) #keskelle
    time.sleep(odotusaika)
    etaisyyskeski = etaisyys.read_mm()
    
    servo.rotate_servo(120) #25 astetta vasemmalle
    time.sleep(odotusaika)
    etaisyysvasen25 = etaisyys.read_mm()

    servo.rotate_servo(140) #45 astetta vasemmalle
    time.sleep(odotusaika)
    etaisyysvasen45 = etaisyys.read_mm()

    servo.rotate_servo(95) 


    if etaisyysoikea25 <= esteetaisyysvjo25:
        esteo25 = 1
        #print("este oikealla 25 astetta")
        gpg.stop()   
    if etaisyysoikea45 <= esteetaisyysvjo45:
        esteo45 = 1
        #print("este oikealla 45 astetta")
        gpg.stop()
    if etaisyyskeski <= esteetaisyysk:
        estek = 1
        gpg.stop()
        #print("este keskella")
    if etaisyysvasen25 <= esteetaisyysvjo25:
        estev25 = 1
        gpg.stop()
        #print("este vasemmalla 25 astetta")
    if etaisyysvasen45 <= esteetaisyysvjo45:
        estev45 = 1
        gpg.stop()
        #print("este vasemmalla 45 astetta")

    else:
        Este = False
    while esteo45 or esteo25 or estek or estev45 or estev25 == 1:
            gpg.stop()
            esteo45 = 0
            esteo25 = 0
            estek = 0
            estev45 = 0
            estev25 = 0
            Este = True
    return Este
######
# -*- coding: utf-8 -*-
import RPi.GPIO as GPIO
from time import sleep

GPIO.setmode(GPIO.BCM)
GPIO.setup(26, GPIO.OUT)
GPIO.output(26, GPIO.LOW)
input("Paina enter sammuttaaksesi.")
print("Asetataan signaali HIGH tilaan.")
GPIO.output(26, GPIO.HIGH)

sleep(5)

GPIO.cleanup()
print("Valmis")

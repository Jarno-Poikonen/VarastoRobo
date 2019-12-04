# -*- coding: utf-8 -*-
# All prints are commented off, these are mainly used for debugging
from __future__ import print_function
from __future__ import division
import traceback

import easygopigo3 as easy
import time
import esteentunnistus

block = False
left = False
right = False
error_val = 0

Start = [0,3]     #Setting starting point into a variable, Coordinate order x,y
position = Start.copy() #Copy Starting point value to position
orientation = 0     #Orientation 0 = East, 1 = North, 2 = West, 3 = South

gpg = easy.EasyGoPiGo3()

try:
    my_linefollower = gpg.init_line_follower()
    time.sleep(0.1)
except:
    #print('Line Follower not responding')
    time.sleep(0.2)
    exit()

base = 250
correction = 150
gpg.set_speed(base)

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
    #print("Position:",position)
    #print("Orientation:",orientation)
    return (position, orientation)

######
def Position(dir):
    if(dir == 0): #East
        position[0] = position[0] + 1
    
    elif(dir == 1): #North
        position[1] = position[1] + 1

    elif(dir == 2): #West
        position[0] = position[0] - 1

    elif(dir == 3): #South
        position[1] = position[1] - 1
######
def Turn(dir):    #Orientation 0 = East, 1 = North, 2 = West, 3 = South
    global orientation
    global position

    if(orientation == 0): #Direction East
        if(dir == 0): #Straight
            pass
        
        elif(dir == 1): #Left
            Left()
    
        elif(dir == 2): #Turn 180
            Around()

        elif(dir == 3): #Right
            Right()

    elif(orientation == 1): #Direction North
        if(dir == 1): #Straight
            pass
        
        elif(dir == 2): #Left
            Left()
    
        elif(dir == 3): #Turn 180
            Around()

        elif(dir == 0): #Right
            Right()
            
    elif(orientation == 2): #Direction West
        if(dir == 2): #Straight
            pass
        
        elif(dir == 3): #Left
            Left()
    
        elif(dir == 0): #Turn 180
            Around()

        elif(dir == 1): #Right
            Right()

    elif(orientation == 3): #Direction South
        if(dir == 3): #Straight
            pass
        
        elif(dir == 0): #Left
            Left()
    
        elif(dir == 1): #Turn 180
            Around()

        elif(dir == 2): #Right
            Right()

    orientation = dir

######
def Park():
    global orientation
    if(orientation == 3):
        Left()
    elif(orientation == 2):
        Around()
    elif(orientation == 1):
        Right()
    orientation = 0
######
def Forward(dir):
    global left
    global right
    global block
    global error_val
    block = False
    
    try:
        while True:
            line_val = my_linefollower.read(representation="bivariate")
            #print(line_val) #Prints linereader values
            if(line_val[0] == 0): left = True
            if(line_val[len(line_val)-1] == 0): right = True
            
            if(left or right):
                left = False
                right = False
                
                gpg.stop()
                gpg.drive_cm(6) #Gets tires aligned with the crossroad

                gpg.stop()
                error_val = 0
                Position(dir)
                break
                
            if my_linefollower.read_position() == 'center':
                gpg.set_speed(base)
                gpg.forward()
            if my_linefollower.read_position() == 'left':
                gpg.set_speed(correction)
                gpg.left()
            if my_linefollower.read_position() == 'right':
                gpg.set_speed(correction)
                gpg.right()
            if my_linefollower.read_position() == 'white':
                gpg.stop()
                error_val = 9
                #print("HJALP, I'm Lost!!!")
                break
                #gpg.drive_cm(-6) #This could return the robot back to the previous crossroad, by driving backwards
                
        gpg.stop()
    except Exception:
        gpg.stop()
        traceback.print_exc()

######
def Move(dir):
    global block
    global error_val
    if(dir < 4):
        Turn(dir)
        block = esteentunnistus.lahella()
        if(block == False):
            Forward(dir)
        else:
            error_val = 10
            #print("Road Blocked")
        if(position == Start):
            Park()
    #print("Error value: ",error_val)
    else:
        error_val=3
    return error_val
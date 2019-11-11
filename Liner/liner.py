# liner.py

import math

class Point:
    def __init__(self, x, y):
        self.x = float(x) # (-inf, inf)
        self.y = float(y) # (-inf, inf)

    def __add__(self, rhs):
        return Point(self.x + rhs.x, self.y + rhs.y)

    def __sub__(self, rhs):
        return Point(self.x - rhs.x, self.y - rhs.y)

    def view(self):
        print("x: " + str(self.x) + ", y: " + str(self.y))  + " (Point)"

    def toString(self):
        return "x: " + str(self.x) + ", y: " + str(self.y) + " (Point)" 

    def getOppositePoint(self):
        return Point(-self.x, -self.y)


class Movement:
    def __init__(self, deg, dist):
        self.deg = float(deg)
        self.rad = float(self.toRad(deg))
        self.dist = float(dist)

    def view(self):
        print("deg: " + str(self.deg))
        print("rad: " + str(self.rad))
        print("dist: " + str(self.dist))

    def toRad(self, deg):
        return deg / 180.0 * math.pi

    def convertToPoint(self, mode):
        if mode == "conventional":
            x = round(math.cos(self.rad) * self.dist, 14)
            y = round(math.sin(self.rad) * self.dist, 14)
        
        elif mode == "reverse": 
            x = round(math.sin(self.rad) * self.dist, 14)
            y = round(math.cos(self.rad) * self.dist, 14)

        return Point(x, y)


class Liner:
    def __init__(self, log_on):
        self.current_location = Point(0, 0)
        self.current_orientation = 0
        self.pointList = []
        self.force_reverse = False
        self.log_on = log_on
        if self.log_on:
            self.view()

    def update(self, deg, dist, mode):
        deg = (deg + self.current_orientation) % 360 # always positive
        movement = Movement(deg, dist)
        if self.force_reverse:
            point = movement.convertToPoint("reverse")
            self.force_reverse = False
        else:
            point = movement.convertToPoint(mode)
        print("DEBUG x: " + str(point.x) + "(Point)")
        print("DEBUG y: " + str(point.y) + "(Point)")
        self.current_location += point;
        self.current_orientation = deg # always positive
        if self.log_on:
            self.view()

    def save(self):
        self.pointList.append(self.current_location)
        if self.log_on:
            self.view()

    def view(self):
        print("Liner.self.current_location: " + self.current_location.toString())
        print("Liner.self.current_orientation: " + str(self.current_orientation))
        if len(self.pointList) == 0:
            print("Liner.self.pointList: []")
        else:
            print("Liner.self.pointList: [")
            for point in self.pointList:
                print("    " + point.toString())
            print("]")

    def getMovement(self, begin, end):
        listLength = len(self.pointList)
        if not listLength >= 2:
            print("warning: need at least 2 saved points to calculate movement, returning Movement(0, 0)")
            return Movement(0, 0)
        if not listLength > end:
            print("warning: 'end' is trying to access an index that doesn't exist, returning Movement(0, 0)")
            return Movement(0, 0)
        if not begin < end:
            print("warning: 'end' is smaller or equal to 'begin', returning Movement(0, 0)")
            return Movement(0, 0)
        delta_point = self.pointList[end] - self.pointList[begin]
        print("DEBUG x: " + str(delta_point.x) + ", y: " + str(delta_point.y) + " ('delta' Point)")
        opposite_point = delta_point.getOppositePoint()
        print("DEBUG x: " + str(opposite_point.x) + ", y: " + str(opposite_point.y) + " ('opposite' Point)")
        if opposite_point.x == 0 and opposite_point.y == 0:
            return Movement(0, 0)
        else:
            dist = math.sqrt(math.pow(opposite_point.x, 2) + math.pow(opposite_point.y, 2))
            angle = (math.atan2(opposite_point.y, opposite_point.x) / math.pi * 180) % 360 # always positive
            print("DEBUG angle: " + str(angle))
            deg = angle - self.current_orientation
            if deg > 180:
                deg -= 360
            elif deg < -180:
                deg += 360
            elif deg == 0 and opposite_point.x < 0:
                deg = 90
                self.force_reverse = True
            elif deg == 0 and opposite_point.x > 0:
                deg = -90
                self.force_reverse = True
            return Movement(deg, dist)

import easygopigo3 as gpg3
import time

gpg = gpg3.EasyGoPiGo3()

def square_test():
    gpg.set_speed(200)
    brc200 = 380 / 360

    for i in range(2):
        liner = Liner(True)
        liner.save()
        gpg.drive_cm(26), time.sleep(1)
        liner.update(0, 26, "reverse")
        
        gpg.orbit(90*brc200), time.sleep(1)
        gpg.drive_cm(26), time.sleep(1)
        liner.update(90, 26, "reverse")
        
        gpg.orbit(90*brc200), time.sleep(1)
        gpg.drive_cm(26), time.sleep(1)
        liner.update(90, 26, "reverse")
        liner.save()
        
        movement = liner.getMovement(0, 1)
        gpg.orbit(movement.deg), time.sleep(1)
        gpg.drive_cm(movement.dist), time.sleep(1)
        liner.update(movement.deg, movement.dist, "conventional")
        gpg.orbit(90*brc200), time.sleep(1)
        liner.update(90, 0, "reverse")
    

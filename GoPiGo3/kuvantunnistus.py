from __future__ import print_function
from __future__ import division
import traceback
import cv2
import numpy as np

TuoteID = 255

def kuvantunnistus():
    cap = cv2.VideoCapture(0)
    
    global TuoteID
    TuoteID = 255
    
    while True:
        _, frame = cap.read()
        
        crop_frame = frame[160:380, 200:640] #rajataan kuva-alue
        
        hsv_frame = cv2.cvtColor(crop_frame, cv2.COLOR_BGR2HSV) #muunnetaan BGR varit HSV vareiksi
        low_green = np.array([24, 40, 0]) #aseteaan varin alaraja-arvo 
        high_green = np.array([90, 255, 255]) #asetetaan varin ylaraja-arvo
        green_mask = cv2.inRange(hsv_frame, low_green, high_green) #etsitaan maaritelty vari kuvasta
        green = cv2.bitwise_and(crop_frame, crop_frame, mask = green_mask) #poistetaan muut paitsi haluttu vari kuvasta
        
        gray = cv2.cvtColor(green, cv2.COLOR_BGR2GRAY) #muutetaan varit harmaaksi
        blurred = cv2.GaussianBlur(gray, (5, 5), 0) #sumennetaan kuvaa
        thresh = cv2.threshold(blurred, 5, 255, cv2.THRESH_BINARY)[1] #luodaan mustavalko kuva raja-arvon ylittavista alueista
        kernel = np.ones((5, 5), np.uint8) #luodaan ykkosilla taytetty array
        mask = cv2.erode(thresh, kernel) #kutistetaan kohdetta
                                                                                                                      
        contours, _ = cv2.findContours(mask, cv2.RETR_TREE, cv2.CHAIN_APPROX_SIMPLE) #etsitaan muodot
        
        for cnt in contours:
            area = cv2.contourArea(cnt)
            approx = cv2.approxPolyDP(cnt, 0.02*cv2.arcLength(cnt, True), True)
            
            if area > 1500: #maaritetaan muotojen minimikoko
                               
                #tunnisteaan muodot
                if len(approx) == 3: #kolmio
                    TuoteID = 2 #annetaan muodolle haluttu nimi tai merkki
                elif len(approx) == 4: #nelio
                    TuoteID = 0
                elif 4 < len(approx): #ympyra
                    TuoteID = 1

        cap.release()
        break
    
    return TuoteID
kuvantunnistus()
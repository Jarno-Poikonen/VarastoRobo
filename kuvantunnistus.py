import cv2
import numpy as np

cap = cv2.VideoCapture(0) 
     
font = cv2.FONT_HERSHEY_COMPLEX

while True:
    _, frame = cap.read()
    
    crop_frame = frame[160:380, 200:640] #rajataan kuva-alue 140
    hsv_frame = cv2.cvtColor(crop_frame, cv2.COLOR_BGR2HSV) #muunnetaan BGR varit HSV vareiksi
    
    low_red = np.array([161, 100, 0]) #aseteaan varin alaraja-arvo 
    high_red = np.array([255, 255, 255]) #asetetaan varin ylaraja-arvo
    red_mask = cv2.inRange(hsv_frame, low_red, high_red) #etsitaan maaritelty vari kuvasta
    red = cv2.bitwise_and(crop_frame, crop_frame, mask = red_mask) #poistetaan muut paitsi haluttu vari kuvasta
    
    gray = cv2.cvtColor(red, cv2.COLOR_BGR2GRAY) #muutetaan varit harmaaksi
    blurred = cv2.GaussianBlur(gray, (5, 5), 0) #sumennetaan kuvaa
    thresh = cv2.threshold(gray, 5, 255, cv2.THRESH_BINARY)[1] #luodaan mustavalko kuva raja-arvon ylittavista alueista
    kernel = np.ones((5, 5), np.uint8) #luodaan ykkosilla taytetty array
    mask = cv2.erode(thresh, kernel) #kutistetaan kohdetta
    
    contours, _ = cv2.findContours(mask, cv2.RETR_TREE, cv2.CHAIN_APPROX_SIMPLE) #etsitaan muodot
    
    for cnt in contours:
        area = cv2.contourArea(cnt)
        approx = cv2.approxPolyDP(cnt, 0.02*cv2.arcLength(cnt, True), True)
        x = approx.ravel()[0]
        y = approx.ravel()[1]
        
        if area > 400: #maaritetaan muotojen minimikoko
            #cv2.drawContours(crop_frame, [approx], 0, (0, 255, 0), 2) #piirretaan muodon aariviivat kuvaan ja maaritellaan sen vari
            
            #tunnisteaan muodot
            if len(approx) == 3:
                #cv2.putText(crop_frame, "Triangle", (x, y), font, 1, (0, 255, 0)) #lisataan teksti ruudulle ja maaritellaan sen vari
                TuoteID = 2 #annetaan muodolle haluttu nimi tai merkki
                print(TuoteID)
            elif len(approx) == 4:
                #cv2.putText(crop_frame, "Rectangle", (x, y), font, 1, (0, 255, 0))
                TuoteID = 0
                print(TuoteID)
            elif len(approx) == 5:
                #cv2.putText(crop_frame, "Pentagon", (x, y), font, 1, (0, 255, 0))
                TuoteID = 3
                print(TuoteID)
            elif 7 < len(approx):
                #cv2.putText(crop_frame, "Circle", (x, y), font, 1, (0, 255, 0))
                TuoteID = 1
                print(TuoteID)
      
    #avataan kameran kuvaa nayttava ikkuna 
    #cv2.imshow("Frame", crop_frame)
    #cv2.imshow("Red", red)
    #cv2.imshow("Gray", gray)
    #cv2.imshow("Blurred", blurred)
    #cv2.imshow("Thresh", thresh)
    #cv2.imshow("Mask", mask)
    #cv2.imshow("Red mask", red_mask)
    
    #odotetaan esc nappaimen painamista scriptin lopettamiseksi
    #key = cv2.waitKey(1)
    #if key == 27:
     #   break

cap.release()
cv2.destroyAllWindows()
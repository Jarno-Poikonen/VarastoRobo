import cv2
import numpy as np

cap = cv2.VideoCapture(0) 

font = cv2.FONT_HERSHEY_COMPLEX

while True:
    _, frame = cap.read()
    
    crop_frame = frame[140:400, 70:550] #rajataan kuva-alue
    
    hsv_frame = cv2.cvtColor(crop_frame, cv2.COLOR_BGR2HSV) #muunnetaan BGR varit HSV vareiksi
    
    low_red = np.array([161, 100, 0]) #aseteaan varin alaraja-arvo
    high_red = np.array([179, 255, 255]) #asetetaan varin ylaraja-arvo
    red_mask = cv2.inRange(hsv_frame, low_red, high_red) #etsitaan maaritelty vari kuvasta
    red = cv2.bitwise_and(crop_frame, crop_frame, mask=red_mask) #poistetaan muut paitsi haluttu vari kuvasta
    kernel = np.ones((5, 5), np.uint8) #luodaan ykkosilla taytetty array
    mask = cv2.erode(red_mask, kernel) #kutistetaan kohdetta
    
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
                shape = 1 #annetaan muodolle haluttu nimi tai merkki
                print(shape)
            elif len(approx) == 4:
                #cv2.putText(crop_frame, "Rectangle", (x, y), font, 1, (0, 255, 0))
                shape = 2
                print(shape)
            elif len(approx) == 5:
                #cv2.putText(crop_frame, "Pentagon", (x, y), font, 1, (0, 255, 0))
                shape = 3
                print(shape)
            elif 7 < len(approx) < 10:
                #cv2.putText(crop_frame, "Circle", (x, y), font, 1, (0, 255, 0))
                shape = 4
                print(shape)
      
    #avataan kameran kuvaa nayttava ikkuna 
    #cv2.imshow("Frame", crop_frame)
    #cv2.imshow("Red", red)
    #cv2.imshow("Mask", mask)
    #cv2.imshow("Red mask", red_mask)
    
    #odotetaan esc nappaimen painamista scriptin lopettamiseksi
    key = cv2.waitKey(1)
    if key == 27:
        break

cap.release()
cv2.destroyAllWindows()
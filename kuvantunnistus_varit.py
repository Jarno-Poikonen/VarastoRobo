import cv2
import numpy as np

cap = cv2.VideoCapture(0)

font = cv2.FONT_HERSHEY_COMPLEX

while True:
    _, frame = cap.read()
    
    crop_frame = frame[140:400, 70:550]
    
    hsv_frame = cv2.cvtColor(crop_frame, cv2.COLOR_BGR2HSV)
    
    low_red = np.array([161, 100, 0])
    high_red = np.array([179, 255, 255])
    red_mask = cv2.inRange(hsv_frame, low_red, high_red)
    red = cv2.bitwise_and(crop_frame, crop_frame, mask=red_mask)
    
    gray = cv2.cvtColor(red, cv2.COLOR_BGR2GRAY)
    blurred = cv2.GaussianBlur(gray, (5, 5), 0)
    thresh = cv2.threshold(blurred, 20, 255, cv2.THRESH_BINARY)[1]
    kernel = np.ones((5, 5), np.uint8)
    mask = cv2.erode(thresh, kernel)
    
    contours, _ = cv2.findContours(mask, cv2.RETR_TREE, cv2.CHAIN_APPROX_SIMPLE)
    
    for cnt in contours:
        area = cv2.contourArea(cnt)
        approx = cv2.approxPolyDP(cnt, 0.02*cv2.arcLength(cnt, True), True)
        x = approx.ravel()[0]
        y = approx.ravel()[1]
        
        if area > 400:
            cv2.drawContours(crop_frame, [approx], 0, (0, 255, 0), 2)
            
            if len(approx) == 3:
                cv2.putText(crop_frame, "Triangle", (x, y), font, 1, (0, 255, 0))
                #shape = 'Triangle'
                #print(shape)
            elif len(approx) == 4:
                cv2.putText(crop_frame, "Rectangle", (x, y), font, 1, (0, 255, 0))
                #shape = "Rectangle"
                #print(shape)
            elif len(approx) == 5:
                cv2.putText(crop_frame, "Pentagon", (x, y), font, 1, (0, 255, 0))
                #shape = "Pentagon"
                #print(shape)
            elif 7 < len(approx) < 10:
                cv2.putText(crop_frame, "Circle", (x, y), font, 1, (0, 255, 0))
                #shape = "Circle"
                #print(shape)
       
    cv2.imshow("Frame", crop_frame)
    cv2.imshow("Red", red)
    #cv2.imshow("Gray", gray)
    #cv2.imshow("Blurred", blurred)
    #cv2.imshow("Thresh", thresh)
    cv2.imshow("Mask", mask)
    
    key = cv2.waitKey(1)
    if key == 27:
        break

cap.release()
cv2.destroyAllWindows()

import cv2
import numpy as np

def nothing(x):
    
    pass

cap = cv2.VideoCapture(0)

font = cv2.FONT_HERSHEY_COMPLEX

while True:
    _, frame = cap.read()
    
    gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
    blurred = cv2.GaussianBlur(gray, (5, 5), 0)
    thresh = cv2.threshold(blurred, 40, 255, cv2.THRESH_BINARY_INV)[1]
    #mask = cv2.inRange(gray, 60, 255)
    kernel = np.ones((5, 5), np.uint8)
    mask = cv2.erode(thresh, kernel)
    
    contours, _ = cv2.findContours(mask, cv2.RETR_TREE, cv2.CHAIN_APPROX_SIMPLE)
    
    for cnt in contours:
        area = cv2.contourArea(cnt)
        approx = cv2.approxPolyDP(cnt, 0.02*cv2.arcLength(cnt, True), True)
        x = approx.ravel()[0]
        y = approx.ravel()[1]
        
        if area > 400:
            cv2.drawContours(frame, [approx], 0, (0, 0, 0), 2)
            
            if len(approx) == 3:
                cv2.putText(frame, "Triangle", (x, y), font, 1, (0, 0, 0))
                #print('Triangle')
            elif len(approx) == 4:
                cv2.putText(frame, "Rectangle", (x, y), font, 1, (0, 0, 0))
                #print('Rectangle')
            elif 7 < len(approx) < 10:
                cv2.putText(frame, "Circle", (x, y), font, 1, (0, 0, 0))
                #print('Circle')
                
        cv2.imshow("Frame", frame)
        cv2.imshow("Mask", mask)
        cv2.imshow("Gray", gray)
        cv2.imshow("Blurred", blurred)
        
        key = cv2.waitKey(1)
        if key == 27:
            break

cap.release()
cv2.destroyAllWindows()
        


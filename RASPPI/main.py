# camera access in raspi
from picamera2 import Picamera2
# open cv
import cv2
# math
import numpy as np

picam2 = Picamera2()
config = picam2.create_preview_configuration(
    main = { "size": ( 2304, 1296 ) }, # 16:9 with full 75 degrees, sensor is 4608 × 2592
    controls = { "FrameRate": 30 }
)
picam2.configure( config )
picam2.start()

# limits the size of the UI
cv2.namedWindow( "Camera", cv2.WINDOW_NORMAL )
cv2.resizeWindow( "Camera", 800, 450 )
#cv2.namedWindow( "Mask", cv2.WINDOW_NORMAL )
#cv2.resizeWindow( "Mask", 800, 450 )

while True:
    frame = picam2.capture_array()
    # change color format for opencv
    frame = cv2.cvtColor( frame, cv2.COLOR_RGB2BGR )

    # detect green object
    hsv = cv2.cvtColor( frame, cv2.COLOR_BGR2HSV )
    lower = np.array( [ 40, 120, 120 ] )
    upper = np.array( [ 80, 255, 255 ] )
    mask = cv2.inRange( hsv, lower, upper )
    contours, _ = cv2.findContours( mask, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE )

    if contours:
        c = max( contours, key = cv2.contourArea )
        x, y, w, h = cv2.boundingRect( c )
        cx = x + w // 2
        cy = y + h // 2

        # draw detection
        cv2.rectangle( frame, ( x, y ), ( x + w, y + h ), ( 0, 255, 0 ), 2 )
        cv2.circle( frame, ( cx, cy ), 5, ( 0, 0, 255 ), -1 )

        print( "Center:", cx, cy )

    cv2.imshow( "Camera", frame )
    #cv2.imshow( "Mask", mask )
    if cv2.waitKey( 1 ) == ord( 'q' ):
        break

cv2.destroyAllWindows()
picam2.stop()

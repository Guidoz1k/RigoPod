# camera access in raspi
from picamera2 import Picamera2
# open cv
import cv2
# math
import numpy as np
# time
import time
# oled display
from luma.core.interface.serial import i2c
from luma.core.render import canvas
from luma.oled.device import ssd1306
# gpios
import time
from signal import pause
from gpiozero import RotaryEncoder, Button

def picam_demo():
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
    cv2.namedWindow( "Mask", cv2.WINDOW_NORMAL )
    cv2.resizeWindow( "Mask", 800, 450 )

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
        cv2.imshow( "Mask", mask )
        if cv2.waitKey( 1 ) == ord( 'q' ):
            break

    cv2.destroyAllWindows()
    picam2.stop()

def i2c_demo():
    serial = i2c( port = 1, address = 0x3C )
    device = ssd1306( serial, width = 128, height = 64 )
    print( "OLED screen initialized!" )

    with canvas( device ) as draw:
        # Draw a white outline frame around the entire screen border
        draw.rectangle( device.bounding_box, outline = "white", fill = "black" )
        
        # Write text: (X, Y coordinate, "Text String", color)
        draw.text( (15, 15), "Hello World!", fill = "white" )
        draw.text( (15, 35), "Pi Global Setup", fill = "white" )

    # Keep the text on the screen for 10 seconds before ending the program
    time.sleep( 10 )

def encoder_demo():
    # 1. Configure the GPIO pins
    # max_steps=0 allows the encoder to rotate infinitely (positive and negative)
    # wrap=False stops it from looping back to zero automatically
    encoder = RotaryEncoder( a = 27, b = 17, max_steps = 0, wrap = False )
    button = Button( pin = 22, bounce_time = 0.05 ) # 50ms debounce to prevent double-clicks

    # 2. Define what happens when actions occur
    def encoder_turned():
        # encoder.steps tracks the current absolute position
        print( f"Encoder Position: {encoder.steps}" )

    def button_pressed():
        print( "Button Clicked!" )

    def button_released():
        print( "Button Released!" )

    # 3. Assign the callback functions to the hardware events
    encoder.when_rotated = encoder_turned
    button.when_pressed = button_pressed
    button.when_released = button_released

    print( "System Ready! Turn the knob or press the button (Ctrl+C to exit)..." )

    # 4. Keep the script running cleanly in the background
    try:
        pause()
    except KeyboardInterrupt:
        print( "\nProgram stopped safely." )

encoder_demo()


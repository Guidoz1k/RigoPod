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
from PIL import Image, ImageDraw
# gpios
import time
from signal import pause
from gpiozero import RotaryEncoder, Button
# serial
import serial
# two threads running, one for opencv and the other for the communication and UI
import threading
# for variance and deviation
import statistics
# for shutting down
import subprocess
# for angle conversion
import math

#################### global variables ####################

center_x = 0
center_y = 0

#################### hardware ####################

serial2 = i2c( port = 1, address = 0x3C )
oled = ssd1306( serial2, width = 128, height = 64 )

encoder = RotaryEncoder( a = 27, b = 17, max_steps = 0, wrap = False )
button = Button( pin = 22, bounce_time = 0.05 ) # 50ms debounce to prevent double-clicks

uart = serial.Serial("/dev/ttyAMA0", 115200, timeout=0.5)

#################### input events ####################

encoder_turns = 0          # accumulated encoder steps since the main loop last looked
encoder_clicked = False    # set to True when the button is pressed

def on_cw():
    global encoder_turns
    encoder_turns += 1

def on_ccw():
    global encoder_turns
    encoder_turns -= 1

def on_press():
    global encoder_clicked
    encoder_clicked = True

encoder.when_rotated_clockwise = on_cw
encoder.when_rotated_counter_clockwise = on_ccw
button.when_pressed = on_press

#################### UART ####################

#    reading ToF:        'R' → 'R'+'ToF byte 1'+'ToF byte 0'
#    setting gimbal:     'G'+'pitch byte 1'+'pitch byte 0'+'roll byte 1'+'roll byte 0' → 'G'
#    active parallax:    'A'+'pitch byte 1'+'pitch byte 0'+'roll byte 1'+'roll byte 0' → 'A'+'ToF byte 1'+'ToF byte 0'
#    serial input error: → 'E'
#    gimbal error:       → 'S'
#    tof error:          → 'T'

def uart_message( mode, pitch = 0, roll = 0 ):
    ERROR_CODES = ( "E", "S", "T" )

    ########################## writing UART

    if mode == 'R':
        tx = b'R'
    elif mode in ( 'G', 'A' ):
        tx = ( mode.encode()
            + pitch.to_bytes( 2, "big", signed = True )
            + roll.to_bytes( 2, "big", signed = True ) )

    uart.reset_input_buffer()
    uart.write(tx)
    uart.flush()

    ########################## reading UART

    first = uart.read(1)
    if len( first ) == 0:
        return ( "TIMEOUT", None )

    ch = chr( first[ 0 ] )

    if ch in ERROR_CODES:
        return ( ch, None )

    # G mode: reply is just 'G'
    if mode == "G":
        return ( "OK", None )

    # R and A modes: two more bytes with the ToF value
    rest = uart.read( 2 )
    if len( rest ) != 2:
        return ( "TIMEOUT", None )

    tof = int.from_bytes( rest, "big" )
    return ( "OK", tof )

#################### screen ####################

def screen( line1, line2 ):
    image = Image.new(oled.mode, oled.size)   # blank 128x64 image
    draw = ImageDraw.Draw(image)

    draw.rectangle(oled.bounding_box, outline="white", fill="black")
    draw.text((15, 15), line1, fill="white")
    draw.text((15, 35), line2, fill="white")

    oled.display(image)

#################### opencv thread ####################

# from the datasheet
F_PX = 1693         # focal point of the camera
RES_V_2 = 1296 / 2  # half the vertical resolution
RES_H_2 = 2304 / 2  # half the horizontal resolution
SHOW_WINDOWS = True    # False for real measurements

def angle_conversion( cx, cy ): #convert pixel position to fov angle using the arctan and the focal distance
    global center_x
    global center_y

    center_x = int( 100 * math.degrees( math.atan( ( cx - RES_H_2 ) / F_PX ) ) )
    center_y = int( 100 * math.degrees( math.atan( ( RES_V_2 - cy ) / F_PX ) ) )
    #print( "Center:", center_x, center_y )

def picam():
    global center_x, center_y

    picam2 = Picamera2()
    config = picam2.create_preview_configuration(
        main = { "size": ( 2304, 1296 ), "format": "RGB888" }, # 16:9 with full 75 degrees, sensor is 4608 × 2592, and format ready for opencv
        controls = { "FrameDurationLimits": ( 33333, 33333 ) } # { "FrameRate": 30 } would sometimes slow down in low light conditions
    )
    picam2.configure( config )
    picam2.start()

    # limits the size of the UI
    if SHOW_WINDOWS:
        cv2.namedWindow( "Camera", cv2.WINDOW_NORMAL )
        cv2.resizeWindow( "Camera", 800, 450 )
        cv2.namedWindow( "Mask", cv2.WINDOW_NORMAL )
        cv2.resizeWindow( "Mask", 800, 450 )

    frame_count = 0
    while True:
        frame_count += 1
    
        frame = picam2.capture_array()
        # change color format for opencv NOT NEEDED ANYMORE
        # frame = cv2.cvtColor( frame, cv2.COLOR_RGB2BGR )

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
            #cv2.rectangle( frame, ( x, y ), ( x + w, y + h ), ( 0, 255, 0 ), 2 )
            #cv2.circle( frame, ( cx, cy ), 5, ( 0, 0, 255 ), -1 )

            #print( "Center:", cx - RES_H_2, RES_V_2 - cy)
            angle_conversion( cx, cy )

        if SHOW_WINDOWS and frame_count % 3 == 0:
            small_frame = cv2.resize( frame, ( 800, 450 ) )
            small_mask  = cv2.resize( mask,  ( 800, 450 ), interpolation = cv2.INTER_NEAREST )
            cv2.imshow( "Camera", small_frame )
            cv2.imshow( "Mask", small_mask )
            if cv2.waitKey( 1 ) == ord( 'q' ):
                break

    cv2.destroyAllWindows()
    picam2.stop()

#################### UI thread ####################

def clear_UI_events():
    global encoder_clicked, encoder_turns

    encoder_clicked = False
    encoder_turns = 0

def active_menu():


    clear_UI_events()

def tof_routine( iterations ):
    measurements = []

    for i in range( iterations ):
        screen( "Rangefinder data", f"{i}")
        status, tof = uart_message( "R" )
        if status == 'OK':
            measurements.append( tof )

    if( len(measurements) >= 2 ): # protects from variance crashing for undersampling
        dataset = len(measurements)
        minimum = min(measurements)
        maximum = max(measurements)
        mean = statistics.mean(measurements)
        variance = statistics.variance(measurements)
        std_dev = statistics.stdev(measurements)

        screen( "Dataset", f"{dataset:3d}" )            
        time.sleep( 3 )
        screen( "Maximum", f"{maximum:5d}" )            
        time.sleep( 3 )
        screen( "Minimum", f"{minimum:5d}" )            
        time.sleep( 3 )
        screen( "Mean", f"{mean:.3f}" )            
        time.sleep( 3 )
        screen( "Variance", f"{variance:.3f}" )            
        time.sleep( 3 )
        screen( "Standard Deviation", f"{std_dev:.3f}" )            
        time.sleep( 3 )

def tof_menu():
    samples = 2 # minimum amount of samples for statistics
    function_complete = False

    clear_UI_events()
    while function_complete == False:
        samples = ( samples + encoder_turns ) % 101
        if samples < 2:
            samples = 2
        screen( "Rangefinder samples", f"{samples}")

        if encoder_clicked == True:
            tof_routine( samples )
            function_complete = True

        clear_UI_events()
        time.sleep( 0.1 )

def gimbal_menu():
    pitch = 0
    roll = 0
    status = '0'
    function_1_complete = False
    function_2_complete = False

    clear_UI_events()
    while function_1_complete == False:
        pitch += encoder_turns
        if pitch < -45 :
            pitch = -45
        if pitch > 45:
            pitch = 45
        screen( "PITCH:", f"{pitch}")

        if encoder_clicked == True:
            clear_UI_events()
            while function_2_complete == False:
                roll += encoder_turns
                if roll < -90 :
                    roll = -90
                if roll > 90:
                    roll = 90
                screen( "ROLL:", f"{roll}")

                if encoder_clicked == True:
                    while status != 'OK':
                        status, tof = uart_message( 'G', pitch * 100, roll * 100 )
                        time.sleep( 0.2 )

                    screen( "Gimbal setup", "COMPLETE")
                    time.sleep( 3 )

                    function_2_complete = True
                
                clear_UI_events()
                time.sleep( 0.1 )

            function_1_complete = True

        clear_UI_events()
        time.sleep( 0.1 )

def test_menu():
    MENU_ITEMS = [ "Gimbal  Set", "ToF  read", "Active  Mode", "SHUTDOWN" ]
    selected = 0
    quit = False

    clear_UI_events()
    while quit != True:
        selected = ( selected + encoder_turns ) % len( MENU_ITEMS )
        screen( "Test Routines", MENU_ITEMS[ selected ] )
        screen( center_x, center_y )

        if encoder_clicked == True:
            match selected:
                case 0:
                    gimbal_menu()
                case 1:
                    tof_menu()
                case 2:
                    active_menu()
                case 3:
                    quit = True

        clear_UI_events()
        time.sleep( 0.1 )

def splash_screen():
    screen( "RigoPOD", "-= test  program =-" )
    time.sleep( 5 )
    test_menu()

def main():
    splash_screen()
    screen( "RigoPOD", "SHUTDOWN" )
    time.sleep( 1 )
    screen( "", "" )
    oled.hide()
    # Safely halt and power off the Raspberry Pi immediately
    subprocess.run( [ "sudo", "shutdown", "-h", "now" ], check = True )

#################### tests thread management ####################

# Create thread objects pointing to your functions
t1 = threading.Thread( target = picam )
t2 = threading.Thread( target = main )

# Start both threads at the same time
t1.start()
t2.start()

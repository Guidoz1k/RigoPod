# timing test for the camera loop
# measures: real frame rate from sensor timestamps + time spent in each processing stage
# run with main.py STOPPED (only one program can open the camera)
 
from picamera2 import Picamera2
import cv2
import numpy as np
import time
 
#################### test settings ####################
 
SHOW_WINDOWS = True     # run once True, once False, and compare
FRAMES       = 300      # ~10 s at 30 Hz
WARMUP       = 15       # frames ignored at start (auto exposure settling)
EXPECTED_MS  = 33.333
 
#################### camera setup (same as main.py) ####################
 
picam2 = Picamera2()
config = picam2.create_preview_configuration(
    main = { "size": ( 2304, 1296 ) },
    controls = { "FrameDurationLimits": ( 33333, 33333 ) }
)
picam2.configure( config )
picam2.start()
 
if SHOW_WINDOWS:
    cv2.namedWindow( "Camera", cv2.WINDOW_NORMAL )
    cv2.resizeWindow( "Camera", 800, 450 )
    cv2.namedWindow( "Mask", cv2.WINDOW_NORMAL )
    cv2.resizeWindow( "Mask", 800, 450 )
 
lower = np.array( [ 40, 120, 120 ] )
upper = np.array( [ 80, 255, 255 ] )
 
#################### measurement loop ####################
 
sensor_ts = []                                                  # ns, from the camera
stages = { "capture": [], "convert": [], "mask": [], "contours": [], "show": [], "total": [] }
 
for i in range( FRAMES + WARMUP ):
    t0 = time.perf_counter()
 
    # frame and ITS OWN metadata, from the same request
    request = picam2.capture_request()
    frame = request.make_array( "main" )
    md = request.get_metadata()
    request.release()
    t1 = time.perf_counter()
 
    frame = cv2.cvtColor( frame, cv2.COLOR_RGB2BGR )
    hsv = cv2.cvtColor( frame, cv2.COLOR_BGR2HSV )
    t2 = time.perf_counter()
 
    mask = cv2.inRange( hsv, lower, upper )
    t3 = time.perf_counter()
 
    contours, _ = cv2.findContours( mask, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE )
    t4 = time.perf_counter()
 
    if SHOW_WINDOWS:
        cv2.imshow( "Camera", frame )
        cv2.imshow( "Mask", mask )
        cv2.waitKey( 1 )
    t5 = time.perf_counter()
 
    if i < WARMUP:
        continue                                                # don't record warm-up frames
 
    sensor_ts.append( md[ "SensorTimestamp" ] )
    stages[ "capture" ].append( t1 - t0 )
    stages[ "convert" ].append( t2 - t1 )
    stages[ "mask" ].append( t3 - t2 )
    stages[ "contours" ].append( t4 - t3 )
    stages[ "show" ].append( t5 - t4 )
    stages[ "total" ].append( t5 - t0 )
 
picam2.stop()
cv2.destroyAllWindows()
 
#################### results ####################
 
intervals = np.diff( np.array( sensor_ts ) ) / 1e6             # ns -> ms
missed = np.sum( intervals > 1.5 * EXPECTED_MS )
 
print( f"\n===== windows {'ON' if SHOW_WINDOWS else 'OFF'}, {len( intervals )} intervals =====" )
print( "Frame interval (sensor timestamps):" )
print( f"  mean   {intervals.mean():7.3f} ms   ->  {1000 / intervals.mean():6.2f} Hz" )
print( f"  stdev  {intervals.std():7.3f} ms" )
print( f"  min    {intervals.min():7.3f} ms" )
print( f"  max    {intervals.max():7.3f} ms" )
print( f"  missed frames (> {1.5 * EXPECTED_MS:.0f} ms): {missed}" )
 
print( "\nTime per stage (Python side):" )
for name, values in stages.items():
    v = np.array( values ) * 1000                               # s -> ms
    print( f"  {name:9s} mean {v.mean():7.2f} ms   max {v.max():7.2f} ms" )
 
print( f"\nLast frame: FrameDuration {md[ 'FrameDuration' ]} us, ExposureTime {md[ 'ExposureTime' ]} us" )
 
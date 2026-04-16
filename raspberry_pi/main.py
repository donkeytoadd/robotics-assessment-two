print("Running on Raspberry Pi")
import serial
from picamera2 import Picamera2
import cv2
from ultralytics import YOLO
import time
import threading
import stream_server

enableCameraFeed = False
if(enableCameraFeed):
    stream_server.start()

# list of angles where a person was detected during a scan, this is sent to the Arduino after STOP is received
angles = []

# tracks the last time a detection was acted on, used to enforce a cooldown between captures
last_sent = 0
# seconds to wait between successive angle captures
cooldown = 1

# open serial connection to the Arduino
ser = serial.Serial('/dev/ttyACM0', 9600)
isScanning = False

# configure the camera for a small frame to maintain performance
picam2 = Picamera2()
picam2.configure(picam2.create_preview_configuration(
    main={"format": "RGB888", "size": (320, 240)}
))
picam2.start()
time.sleep(2)

# load the YOLOv8 model
model = YOLO("yolov8n.pt")

# frame counter used to throttle how often YOLO inference is run
counter = 0

# latest_frame is written by the camera thread and read by the main loop
# frame_lock ensures reads and writes don't race
latest_frame = None
frame_lock = threading.Lock()

# runs in a background thread to continuously capture frames from the camera.
# if the stream feed is enabled, also runs inference and pushes annotated frames to the server
def camera_thread():
    global latest_frame
    while True:
        frame = picam2.capture_array()
        with frame_lock:
            latest_frame = frame
        if(enableCameraFeed):
            results = model(frame, verbose=False, imgsz=256, classes=[0])
            stream_server.set_frame(results[0].plot())

t = threading.Thread(target=camera_thread, daemon=True)
t.start()

# serialises the collected angles as a comma-separated string and sends them to the Arduino.
# the Arduino will then move the turntable to each angle in sequence to dispense cards
def sendStoredAngles():
    if angles:
      message = ",".join(str(a) for a in angles)
      ser.write(f"STORED_ANGLES:{message}\n".encode())
      angles.clear()
    else:
      print("No angles to send.")

# reads one line from serial if available and handles scan lifecycle commands from the Arduino.
# SCAN/STOP control the scanning state, ANGLE responses are stored for later
def handle_serial_commands():
    global isScanning
    if ser.in_waiting > 0:
        line = ser.readline().decode().strip()
        print("CMD:", line)
        if line == "SCAN":
            isScanning = True
            print("Scan mode STARTED")
        elif line == "STOP":
            isScanning = False
            print("Scan mode STOPPED")
            time.sleep(1)
            sendStoredAngles()
        elif line.startswith("ANGLE:"):
            angle = int(line.split(":")[1])
            angles.append(angle)
            print(f"Captured angle: {angle}")
            return

# checks whether any detected person's bounding box centre falls within tolerance pixels of the horizontal centre of the frame
def is_centered_human(results, frame_width, tolerance=80):
    center_x = frame_width / 2
    for box in results[0].boxes:
        cls = int(box.cls[0])
        if model.names[cls] != "person":
            continue
        x1, y1, x2, y2 = box.xyxy[0].tolist()
        if abs((x1 + x2) / 2 - center_x) <= tolerance:
            return True
    return False

# grabs the latest camera frame and runs YOLO inference on every 3rd call
# returns True if a centred person is detected in the current frame
def detect_human():
    global counter
    with frame_lock:
        if latest_frame is None:
            return False
        frame = latest_frame.copy()

    counter += 1
    if counter % 3 != 0:
        return False

    results = model(frame, verbose=False, imgsz=256)
    if(enableCameraFeed):
        stream_server.set_frame(results[0].plot())
    return is_centered_human(results, frame.shape[1])

# sends a CAPTURE_ANGLE command to the Arduino, which responds with the turntable's current angle
def captureAngle():
    global angles
    ser.write(b"CAPTURE_ANGLE\n")

# main loop which continuously polls for serial commands and, when in scan mode,
# checks for human detections and capture the turntable angle
while True:
    handle_serial_commands()
    if isScanning:
      if detect_human():
          if time.time() - last_sent > cooldown:
              print("HUMAN DETECTED")
              captureAngle()
              last_sent = time.time()

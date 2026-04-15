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

angles = []

last_sent = 0
cooldown = 1

ser = serial.Serial('/dev/ttyACM0', 9600)
isScanning = False
picam2 = Picamera2()
picam2.configure(picam2.create_preview_configuration(
    main={"format": "RGB888", "size": (320, 240)}
))
picam2.start()
time.sleep(2)  # warm up
model = YOLO("yolov8n.pt")
counter = 0

latest_frame = None
frame_lock = threading.Lock()

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

def sendStoredAngles():
    if angles:
      message = ",".join(str(a) for a in angles)
      ser.write(f"STORED_ANGLES:{message}\n".encode())
      angles.clear()
    else:
      print("No angles to send.")

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

def captureAngle():
    global angles
    ser.write(b"CAPTURE_ANGLE\n")

while True:
    handle_serial_commands()
    if isScanning:
      if detect_human():
          if time.time() - last_sent > cooldown:
              print("HUMAN DETECTED")
              captureAngle()
              last_sent = time.time()

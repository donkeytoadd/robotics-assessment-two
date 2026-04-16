# Automated Card Shuffler & Dealer

An Arduino + Raspberry Pi project that automates shuffling and dealing a deck of playing cards. The machine scans around the table to detect and locate players, then deals cards to each player in turn.

## System Overview

The machine is built in three stacked layers:

```text
┌─────────────────────────┐
│      Card Shuffler      │  ← DC motor splits & shuffles half-decks
├─────────────────────────┤
│    Card Magazine        │  ← Holds shuffled deck; DC motor dispenses one card at a time
├─────────────────────────┤
│  Rotating Platform      │  ← Stepper motor rotates 360°; Raspberry Pi + PiCamera scans for players
└─────────────────────────┘
```

### Layer 1 — Rotating Platform (bottom)

- **Arduino Uno** drives a **28BYJ-48 stepper motor** via pins 8–11
- **Raspberry Pi** (mounted on the platform) runs a YOLOv8 person-detection model via PiCamera2
- On scan start, the platform performs one full 360° rotation; whenever a centered person is detected the Pi requests the current angle from the Arduino and stores it
- After the scan completes, all captured player angles are sent back to the Arduino so the platform can rotate to each player position during dealing

### Layer 2 — Card Magazine (middle)

- Sits directly above the platform
- A **DC motor** (connected via an H-bridge to Arduino pins 9/10) drives a wheel that ejects cards one at a time when a `DISPENSE` command is received

### Layer 3 — Card Shuffler (top)

- A split deck is loaded into two chutes
- A **DC motor** (H-bridge, Arduino pins 5/6) interleaves the two halves when a `SHUFFLE` command is received

### Serial Protocol (Arduino ↔ Raspberry Pi)

| Direction | Message | Meaning |
|-----------|---------|---------|
| Arduino → Pi | `SCAN` | Begin 360° scan / player detection |
| Arduino → Pi | `STOP` | Scan complete; send stored angles back |
| Pi → Arduino | `CAPTURE_ANGLE` | Record current platform angle |
| Arduino → Pi | `ANGLE:<n>` | Current angle in degrees |
| Pi → Arduino | `STORED_ANGLES:<a,b,c>` | Comma-separated player angles |
| Pi → Arduino | `SHUFFLE` | Run the shuffler motor |
| Pi → Arduino | `DISPENSE` | Eject one card |

---

## Hardware Requirements

| Component | Quantity | Notes |
|-----------|----------|-------|
| Arduino Uno | 1 | - |
| Raspberry Pi (3B+ or 4) | 1 | Must support PiCamera2 |
| Raspberry Pi Camera Module v2/v3 | 1 | |
| 28BYJ-48 stepper motor + ULN2003 driver | 1 | Platform rotation |
| DC motor (5–9 V) | 2 | One for shuffler, one for dispenser |
| L298N (or similar) H-bridge module | 1–2 | Motor driver for DC motors |
| Push button | 1 | Triggers scan sequence (Arduino digital pin 2) |
| Jumper wires, power supply, frame/chassis | — | |

### Pin Mapping

#### Stepper (platform rotation)



#### DC Motors (H-bridge)

| Arduino Pin | Function |
|-------------|----------|
| 5 | Shuffler — forward |
| 6 | Shuffler — reverse |
| 10 | Dispenser — forward |
| 9 | Dispenser — reverse |

> **Note:** pins 9 and 10 are shared between the stepper driver and the DC motor H-bridge in the current wiring. If both are needed simultaneously, remap one set of pins and update the `#define` values in [dc_motor_controller.h](arduino/lib/dc_motor_controller/headers/dc_motor_controller.h).

#### Button

| Arduino Pin | Function |
|-------------|----------|
| 2 (INPUT_PULLUP) | Scan trigger button |

---

## Software Requirements

### Arduino (PlatformIO)

- [PlatformIO](https://platformio.org/) (VS Code extension recommended)
- Dependencies are managed automatically via `platformio.ini`:
  - `arduino-libraries/Stepper@^1.1.3`

### Raspberry Pi

```text
Python 3.9+
picamera2
opencv-python
ultralytics   (YOLOv8)
pyserial
```

---

## Building & Flashing the Arduino

1. Open the `arduino/` folder in VS Code with the PlatformIO extension installed.
2. PlatformIO will automatically resolve the `Stepper` library dependency.
3. Connect the Arduino Uno via USB.
4. Click **Upload** in PlatformIO (or run `pio run --target upload` in the terminal).
5. Open the Serial Monitor at **9600 baud** to observe debug output.

```bash
# From the arduino/ directory
pio run --target upload
pio device monitor --baud 9600
```

---

## Setting Up the Raspberry Pi

### 1. Install dependencies

```bash
pip install picamera2 opencv-python ultralytics pyserial
```

### 2. Download the YOLOv8 model

The first run downloads `yolov8n.pt` automatically via `ultralytics`. To pre-download:

```bash
python -c "from ultralytics import YOLO; YOLO('yolov8n.pt')"
```

### 3. Verify the serial port

The Pi communicates with the Arduino over `/dev/ttyACM0`. Confirm the port:

```bash
ls /dev/ttyACM*
```

If the port differs, update line 18 of [main.py](raspberry_pi/main.py):

```python
ser = serial.Serial('/dev/ttyACM0', 9600)
```

### 4. (Optional) Enable the live camera stream

In [main.py](raspberry_pi/main.py), set:

```python
enableCameraFeed = True
```

The annotated MJPEG stream will be available at `http://<pi-ip>:8080/stream` once running.

### 5. Run

```bash
cd raspberry_pi
python main.py
```

---

## Operating the Machine

### Full Dealing Sequence

1. **Load the deck** — split the deck roughly in half and place each half into the two shuffler chutes at the top of the machine.
2. **Power on** — connect the Arduino and start `main.py` on the Raspberry Pi.
3. **Shuffle** — send `SHUFFLE` over serial (or trigger from the Pi) to interleave the two half-decks. The shuffler motor runs for 5 seconds by default. The shuffled deck drops down into the magazine.
4. **Scan for players** — press the physical button connected to Arduino pin 2. The platform will:
   - Reset to its home position (1° nudge)
   - Send `SCAN` to the Pi to begin person detection
   - Rotate 360° at 10 RPM
   - Send `STOP` when complete; the Pi transmits all detected player angles back to the Arduino
5. **Deal** — send `DISPENSE` for each card. The platform rotates to each player's stored angle before the dispenser motor pulses to eject a single card.

### Manual Serial Commands

Connect a serial terminal (9600 baud) to the Arduino or send commands from the Pi:

| Command | Effect |
|---------|--------|
| `SHUFFLE` | Run shuffler for the configured duration |
| `SHUFFLE_FULL` | Run shuffler at full PWM (255) for 5 s |
| `DISPENSE` | Eject one card (single pulse) |
| `SET_SHUFFLE_SPEED <0-255>` | Set shuffler PWM level |
| `SET_DISPENSE_SPEED <0-255>` | Set dispenser PWM level |
| `SET_SHUFFLE_DURATION <ms>` | Set how long the shuffler runs |
| `SET_DISPENSE_PULSE <ms>` | Set how long each dispense pulse lasts |

### Tuning Motor Parameters

Default values are set in [dc_motor_controller.cpp](arduino/lib/dc_motor_controller/implementations/dc_motor_controller.cpp):

```cpp
shuffleSpeed    = 200;   // PWM 0–255
dispenseSpeed   = 250;   // PWM 0–255
shuffleDuration = 5000;  // milliseconds
dispensePulse   = 300;   // milliseconds
```

These can be changed at runtime with the `SET_*` serial commands above, or adjusted at compile time.

---

## Known Limitations

**Motor torque is insufficient when fully assembled.** When all three layers are stacked (platform + magazine + shuffler), the combined weight exceeds what the current motors can reliably move. Potential remedies:

- Replace the 28BYJ-48 stepper with a higher-torque stepper (e.g. NEMA 17) and a suitable driver (e.g. A4988/DRV8825).
- Use a higher-voltage, higher-current DC motor for the shuffler and dispenser, and a matched H-bridge capable of supplying the required current.
- Power the motors from a dedicated external supply rather than relying on the Arduino's 5 V rail.
- Reduce the physical weight of the upper layers by redesigning the frame with lighter materials.
- Increase the gear ratio to trade speed for torque.

---

## Project Structure

```text
robotics-assessment-two/
├── arduino/
│   ├── src/
│   │   └── main.cpp                          # Arduino entry point
│   ├── lib/
│   │   ├── dc_motor_controller/
│   │   │   ├── headers/dc_motor_controller.h
│   │   │   └── implementations/dc_motor_controller.cpp
│   │   └── servo_controller/
│   │       ├── headers/
│   │       │   ├── stepper_controller.h
│   │       │   └── digital_button.h
│   │       └── implementations/
│   │           ├── stepper_controller.cpp
│   │           └── digital_button.cpp
│   └── platformio.ini
└── raspberry_pi/
    ├── main.py          # Person detection, serial comms, angle capture
    └── stream_server.py # Optional MJPEG stream server (port 8080)
```

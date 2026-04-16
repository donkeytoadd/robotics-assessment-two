#include <Arduino.h>
#include <headers/stepper_controller.h>
#include <headers/digital_button.h>
#include <headers/dc_motor_controller.h>

DigitalButton scanButton = DigitalButton();
StepperController stepMotor;
DCMotorController dcMotor;

void handleScanPress();
void handleSerialCommands();

String inputBuffer = "";

void setup()
{
  Serial.begin(9600);

  // initialise DC motors with pin modes and ensure they start in a stopped state
  dcMotor.begin();

  // configure the scan button pin and register the press handler
  scanButton.begin();
  scanButton.onPress(handleScanPress);
}

void loop()
{
  // check button state for a press event
  scanButton.loop();
  // check for incoming serial commands from the Raspberry Pi
  handleSerialCommands();
  // advance the stepper motor by one step toward its current target (non-blocking)
  stepMotor.TurnStep();
}

// notifies the Raspberry Pi to begin scanning, then rotates the turntable 360 degrees.
// a short delay is added before the motor starts so the Pi has time to enter scan mode
void startScan()
{
  Serial.println("SCAN");
  delay(200);
  stepMotor.SetTarget(360, []()
                      { Serial.println("STOP"); }, 10);
}

// on button press, the motor first resets to position 1 (near-zero) before starting the scan,
// ensuring each scan begins from a consistent home position
void handleScanPress()
{
  stepMotor.SetTarget(1, []()
                      { startScan(); }, 15);
}

// queue used to store the angles received from the Raspberry Pi for motor positioning
int angleQueue[50];
int queueSize = 0;
int queueIndex = 0;

// is called when the motor finishes moving to an angle, moves to the next queued angle if one exists,
// or reports ALL_ANGLES_DONE to the Pi when all positions have been visited
void onAngleComplete()
{
  Serial.println("Queue index: " + String(queueIndex) + ", Queue size: " + String(queueSize));
  if (queueIndex < queueSize)
    stepMotor.SetTarget(angleQueue[queueIndex++], onAngleComplete);
  else
    Serial.println("ALL_ANGLES_DONE");
}

// parses a list of angles from the Pi and loads them into the queue,
// then starts movement to the first angle
void handleStoredAngles(String data)
{
  queueSize = 0;
  queueIndex = 0;
  int start = 0;
  int commaIndex = -1;
  do
  {
    commaIndex = data.indexOf(',', start);
    // extract each token between commas
    String angleStr = (commaIndex == -1)
                          ? data.substring(start)
                          : data.substring(start, commaIndex);

    angleQueue[queueSize++] = angleStr.toInt();
    start = commaIndex + 1;
  } while (commaIndex != -1);

  if (queueSize > 0)
    stepMotor.SetTarget(angleQueue[queueIndex++], onAngleComplete);
}

// reads a line from serial, then routes to the appropriate handler.
// CAPTURE_ANGLE and STORED_ANGLES are stepper commands; everything else is forwarded to the DC motor controller
void handleSerialCommands()
{
  if (Serial.available() > 0)
  {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();

    if (cmd == "CAPTURE_ANGLE")
    {
      // respond with the turntable's current angle so the Pi can record it
      Serial.println("ANGLE:" + String(stepMotor.CurrentAngle()));
    }
    else if (cmd.startsWith("STORED_ANGLES:"))
    {
      String data = cmd.substring(14);
      Serial.println("Stored ANGLES: " + data);
      handleStoredAngles(data);
    }
    else
    {
      dcMotor.handleCommand(cmd);
    }
  }
}
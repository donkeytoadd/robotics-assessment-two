#include <Arduino.h>
#include <headers/stepper_controller.h>
#include <headers/digital_button.h>

DigitalButton scanButton = DigitalButton();

StepperController stepMotor;

void startScan()
{
  Serial.println("SCAN"); // tells raspberry pi to start scan mode
  delay(200);             // give Pi time to start scan mode before motor begins
  stepMotor.SetTarget(360, []()
                      { Serial.println("STOP"); }, 10);
  // delay(500); // delay to debounce button
}

void handleScanPress()
{
  stepMotor.SetTarget(1, []()
                      { startScan(); },
                      15); // reset position for scan
}

void setup()
{
  Serial.begin(9600);

  scanButton.begin();
  scanButton.onPress(handleScanPress);
}

int angleQueue[50];
int queueSize = 0;
int queueIndex = 0;
// bool startNextAngle = false;

void onAngleComplete()
{
  Serial.println("Queue index: " + String(queueIndex) + ", Queue size: " + String(queueSize));
  if (queueIndex < queueSize)
    // startNextAngle = true;
    stepMotor.SetTarget(angleQueue[queueIndex++], onAngleComplete);
  else
    Serial.println("ALL_ANGLES_DONE");
}

void handleStoredAngles(String data)
{
  queueSize = 0;
  queueIndex = 0;
  int start = 0;
  int commaIndex = -1;
  do
  {
    commaIndex = data.indexOf(',', start);
    String angleStr = (commaIndex == -1)
                          ? data.substring(start)
                          : data.substring(start, commaIndex);

    angleQueue[queueSize++] = angleStr.toInt();
    start = commaIndex + 1;
  } while (commaIndex != -1);

  if (queueSize > 0)
    stepMotor.SetTarget(angleQueue[queueIndex++], onAngleComplete);
}

void handleSerialCommands()
{
  if (Serial.available() > 0)
  {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();

    if (cmd == "CAPTURE_ANGLE")
    {
      Serial.println("ANGLE:" + String(stepMotor.CurrentAngle()));
    }
    else if (cmd.startsWith("STORED_ANGLES:"))
    {
      String data = cmd.substring(14);
      Serial.println("Stored ANGLES: " + data);
      handleStoredAngles(data);
    }
  }
}

void loop()
{
  scanButton.loop();
  handleSerialCommands();
  stepMotor.TurnStep();

  // if (startNextAngle)
  // {
  //   startNextAngle = false;
  //   stepMotor.SetTarget(angleQueue[queueIndex++], onAngleComplete);
  // }
}
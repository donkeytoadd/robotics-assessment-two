#include <Arduino.h>
#include <headers/stepper_controller.h>

StepperController stepMotor = StepperController();

void setup()
{
  delay(500);
  Serial.begin(9600);

  stepMotor.Turn(0);
  delay(2000);
  stepMotor.Turn(90);
  delay(2000);
  stepMotor.Turn(180);
  delay(2000);
}

void loop()
{
}
#pragma once
#include <Arduino.h>
#include <Stepper.h>

class StepperController
{
private:
  const int STEPS_PER_REV = 2048; // How many steps for 1 full rotation/revolution (360 degrees)
  int ClampValue(int minValue, int maxValue, int value);
  Stepper *stepper;
  long currentSteps = 0;

public:
  StepperController();
  void Turn(int angle);
  int CurrentAngle();
};
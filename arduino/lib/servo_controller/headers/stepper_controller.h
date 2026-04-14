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
  long targetSteps = 0;
  typedef void (*TurnFinishedCallBack)();
  TurnFinishedCallBack callback = nullptr;

public:
  StepperController();
  // void Turn(int angle, TurnFinishedCallBack callback = nullptr, int rpmSpeed = 15);
  void TurnStep();
  void SetTarget(int angle, TurnFinishedCallBack cb = nullptr, int rpmSpeed = 15);
  int CurrentAngle();
};
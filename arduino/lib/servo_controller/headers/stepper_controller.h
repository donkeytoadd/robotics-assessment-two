#pragma once
#include <Arduino.h>
#include <Stepper.h>

class StepperController
{
private:
  // stepper motor uses 2048 steps per full revolution in half-step mode
  const int STEPS_PER_REV = 2048;

  // clamps a value between minValue and maxValue to prevent out-of-range step counts
  int ClampValue(int minValue, int maxValue, int value);

  Stepper *stepper;
  // absolute step count from the starting position
  long currentSteps = 0;
  // step count we are currently moving toward
  long targetSteps = 0;

  // callback invoked once the motor reaches its target angle
  typedef void (*TurnFinishedCallBack)();
  TurnFinishedCallBack callback = nullptr;

public:
  StepperController();

  // advances the motor by one step toward the target, this is called every loop() iteration
  void TurnStep();

  // sets a new target angle (degrees) and optional callback to send on completion
  void SetTarget(int angle, TurnFinishedCallBack cb = nullptr, int rpmSpeed = 15);

  // returns the current position in degrees, gotten from the accumulated step count
  int CurrentAngle();
};
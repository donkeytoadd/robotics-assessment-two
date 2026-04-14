#include "./headers/stepper_controller.h"

StepperController::StepperController()
{
  const int MOTOR_PINS[] = {8, 9, 10, 11};
  stepper = new Stepper(STEPS_PER_REV, MOTOR_PINS[0], MOTOR_PINS[2], MOTOR_PINS[1], MOTOR_PINS[3]);
}

int StepperController::ClampValue(int minValue, int maxValue, int value)
{
  if (value < minValue)
    return minValue;
  if (value > maxValue)
    return maxValue;
  return value;
}

void StepperController::TurnStep()
{
  if (currentSteps == targetSteps)
    return; // nothing to do

  int direction = (targetSteps > currentSteps) ? 1 : -1;
  stepper->step(direction);
  currentSteps += direction;

  if (currentSteps == targetSteps)
  {
    Serial.println(String("Turned to ") + CurrentAngle() + " degrees");
    if (callback != nullptr)
    {
      void (*tempCallback)() = callback;
      callback = nullptr;
      tempCallback();
    }
  }
}

void StepperController::SetTarget(int angle, TurnFinishedCallBack cb, int rpmSpeed = 15)
{
  int newTarget = round(STEPS_PER_REV * (angle / 360.0));

  Serial.println("SETTING TARGET TO: " + String(angle) + " degrees, which is " + String(newTarget) + " steps");

  // below line is for debugging - can be commented out
  if (newTarget == currentSteps)
  {
    Serial.println("IMMEDIATE FINISH: Math says we are already there.");
    if (cb != nullptr)
      cb();
    return;
  }

  stepper->setSpeed(rpmSpeed);
  // int clampedAngle = ClampValue(0, 360, angle);
  targetSteps = newTarget;
  callback = cb;
}

int StepperController::CurrentAngle()
{
  return (int)round((currentSteps * 360.0) / STEPS_PER_REV);
}
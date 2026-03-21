#include "./headers/stepper_controller.h"

StepperController::StepperController()
{
  const int MOTOR_PINS[] = {8, 9, 10, 11};
  stepper = new Stepper(STEPS_PER_REV, MOTOR_PINS[0], MOTOR_PINS[2], MOTOR_PINS[1], MOTOR_PINS[3]);
  const int rpmSpeed = 15; // 10-15 is ideal for precise and reliable movements, any higher it may struggle with bigger loads.
  stepper->setSpeed(rpmSpeed);
}

int StepperController::ClampValue(int minValue, int maxValue, int value)
{
  if (value < minValue)
    return minValue;
  if (value > maxValue)
    return maxValue;
  return value;
}

void StepperController::Turn(int angle)
{
  int clampedAngle = ClampValue(0, 180, angle);
  long targetSteps = STEPS_PER_REV * (clampedAngle / 360.0);
  long stepsToMove = targetSteps - currentSteps;
  stepper->step(stepsToMove);
  currentSteps = targetSteps;
  Serial.println(String("Turned to ") + clampedAngle + " degrees");
}

int StepperController::CurrentAngle()
{
  return (int)((currentSteps * 360.0) / STEPS_PER_REV);
}
#include "./headers/stepper_controller.h"

// initialises the stepper motor on pins 8-11. pin order is interleaved (0,2,1,3) to match
// the coil wiring sequence required by the stepper motor for correct rotation direction
StepperController::StepperController()
{
  const int MOTOR_PINS[] = {8, 9, 10, 11};
  stepper = new Stepper(STEPS_PER_REV, MOTOR_PINS[0], MOTOR_PINS[2], MOTOR_PINS[1], MOTOR_PINS[3]);
}

// returns value clamped between minValue and maxValue, used to prevent the motor
// being sent out-of-range step targets
int StepperController::ClampValue(int minValue, int maxValue, int value)
{
  if (value < minValue)
    return minValue;
  if (value > maxValue)
    return maxValue;
  return value;
}

// advances the motor by one step toward the target each time it is called.
// once the target is reached, fires the stored callback and clears it so it only fires once.
// the callback is copied to a local variable before being cleared to prevent issues
// if the callback itself calls SetTarget()
void StepperController::TurnStep()
{
  // early exit check, if we are already at the target then skip making a step
  if (currentSteps == targetSteps)
    return;

  // make the step if we are not at the target steps
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

// converts an angle in degrees to a step count and begins moving toward it.
// if the motor is already at the target, the callback is fired immediately and the function returns early
void StepperController::SetTarget(int angle, TurnFinishedCallBack cb, int rpmSpeed = 15)
{
  int newTarget = round(STEPS_PER_REV * (angle / 360.0));

  Serial.println("SETTING TARGET TO: " + String(angle) + " degrees, which is " + String(newTarget) + " steps");

  // if we are already at the target position, fire the callback immediately rather than waiting
  if (newTarget == currentSteps)
  {
    Serial.println("IMMEDIATE FINISH: Math says we are already there.");
    if (cb != nullptr)
      cb();
    return;
  }

  stepper->setSpeed(rpmSpeed);
  targetSteps = newTarget;
  callback = cb;
}

// converts the current step count back to degrees for reporting over serial
int StepperController::CurrentAngle()
{
  return (int)round((currentSteps * 360.0) / STEPS_PER_REV);
}
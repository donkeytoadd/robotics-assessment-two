#include "..//headers/digital_button.h"

DigitalButton::DigitalButton()
{
  _lastState = HIGH;
  _onButtonPress = nullptr;
}

void DigitalButton::begin()
{
  pinMode(_pin, INPUT_PULLUP);
}

void DigitalButton::onPress(OnButtonPress onButtonPress)
{
  _onButtonPress = onButtonPress;
}

void DigitalButton::loop()
{
  bool currentState = digitalRead(_pin);
  if (_lastState == HIGH && currentState == LOW)
  {
    if (_onButtonPress != nullptr)
    {
      _onButtonPress();
      //_completedSpin = !_completedSpin;
    }
  }

  _lastState = currentState;
}
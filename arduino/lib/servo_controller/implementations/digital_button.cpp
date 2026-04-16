#include "..//headers/digital_button.h"

// initialises the button with no callback and assumes the pin is idle HIGH (unpressed)
DigitalButton::DigitalButton()
{
  _lastState = HIGH;
  _onButtonPress = nullptr;
}

// sets the pin to INPUT_PULLUP so the pin reads HIGH at rest and LOW when the button connects it to ground
void DigitalButton::begin()
{
  pinMode(_pin, INPUT_PULLUP);
}

// stores the callback function to call when a press is detected
void DigitalButton::onPress(OnButtonPress onButtonPress)
{
  _onButtonPress = onButtonPress;
}

// called every loop() tick and detects a change from HIGH to LOW which indicates a button press,
// then fires the registered callback if one has been set
void DigitalButton::loop()
{
  bool currentState = digitalRead(_pin);
  if (_lastState == HIGH && currentState == LOW)
  {
    if (_onButtonPress != nullptr)
    {
      _onButtonPress();
    }
  }

  _lastState = currentState;
}
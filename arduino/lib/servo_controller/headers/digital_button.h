#ifndef DIGITAL_BUTTON_H
#define DIGITAL_BUTTON_H

#include <Arduino.h>

class DigitalButton
{
public:
  typedef void (*OnButtonPress)();
  DigitalButton();

  // configures the button pin as INPUT_PULLUP so no external resistor is needed
  void begin();

  // registers the function to call when a button press is detected
  void onPress(OnButtonPress onButtonPress);

  // reads the pin state each loop() tick and triggers the callback on a change
  void loop();

private:
  // previous pin reading, used to detect the HIGH->LOW transition
  bool _lastState;
  // Digital 2 pin on Arduino Uno
  int _pin = 2;
  OnButtonPress _onButtonPress;
};

#endif
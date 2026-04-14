#ifndef DIGITAL_BUTTON_H
#define DIGITAL_BUTTON_H

#include <Arduino.h>

class DigitalButton
{
public:
  typedef void (*OnButtonPress)();
  DigitalButton();

  void begin();
  void onPress(OnButtonPress onButtonPress);
  void loop();

private:
  bool _lastState;
  int _pin = 2; // Digital 2 pin on arduino uno
  OnButtonPress _onButtonPress;
};

#endif
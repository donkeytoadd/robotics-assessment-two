#ifndef DC_MOTOR_CONTROLLER_H
#define DC_MOTOR_CONTROLLER_H

#include <Arduino.h>

// Motor pins
#define SHUFFLE_FWD 5
#define SHUFFLE_REV 6
#define DISPENSE_FWD 10
#define DISPENSE_REV 9

class DCMotorController
{
public:
    DCMotorController();

    void begin();
    void handleCommand(String cmd);

    void runShuffler();
    void runDispenser();
    void stopAll();

    void setShuffleSpeed(int speed);
    void setDispenseSpeed(int speed);
    void setShuffleDuration(unsigned long duration);
    void setDispensePulse(unsigned long pulse);

private:
    int shuffleSpeed;
    int dispenseSpeed;
    unsigned long shuffleDuration;
    unsigned long dispensePulse;
};

#endif

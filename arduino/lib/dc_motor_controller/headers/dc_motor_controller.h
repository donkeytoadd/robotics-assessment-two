#ifndef DC_MOTOR_CONTROLLER_H
#define DC_MOTOR_CONTROLLER_H

#include <Arduino.h>

// PWM-capable pins on the Arduino board used to drive the motor inputs.
// each motor needs a forward and reverse pin so direction can be controlled.
#define SHUFFLE_FWD 5
#define SHUFFLE_REV 6
#define DISPENSE_FWD 10
#define DISPENSE_REV 9

// controls two DC motors via an L293D driver: one for shuffling cards and one for dispensing them.
// speed is set using PWM (0-255) and motors run for a fixed duration before stopping automatically.
class DCMotorController
{
public:
    DCMotorController();

    // configures motor pins as outputs
    void begin();

    // dispatches a serial command string for the relevant motor action
    void handleCommand(String cmd);

    // runs the shuffle motor forward for shuffleDuration milliseconds at shuffleSpeed
    void runShuffler();

    // runs the dispense motor forward for dispenseDuration milliseconds at dispenseSpeed
    void runDispenser();

    // stops both motors immediately by writing 0 to all motor pins
    void stopAll();

    // runtime-adjustable speed setters - useful for tuning without re-uploading to the board
    void setShuffleSpeed(int speed);
    void setDispenseSpeed(int speed);
    void setShuffleDuration(unsigned long duration);
    void setDispenseDuration(unsigned long pulse);

private:
    // PWM value (0-255) for the shuffle motor
    int shuffleSpeed;
    // PWM value (0-255) for the dispense motor
    int dispenseSpeed;
    // how long (ms) the shuffle motor runs per cycle
    unsigned long shuffleDuration;
    // how long (ms) the dispense motor runs per cycle
    unsigned long dispenseDuration;
};

#endif
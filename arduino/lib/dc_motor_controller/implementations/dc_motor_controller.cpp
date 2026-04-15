#include "../headers/dc_motor_controller.h"

DCMotorController::DCMotorController()
{
    shuffleSpeed = 80;
    dispenseSpeed = 250;
    shuffleDuration = 5000;
    dispensePulse = 300;
}

void DCMotorController::begin()
{
    pinMode(SHUFFLE_FWD, OUTPUT);
    pinMode(SHUFFLE_REV, OUTPUT);
    pinMode(DISPENSE_FWD, OUTPUT);
    pinMode(DISPENSE_REV, OUTPUT);
    stopAll();
}

void DCMotorController::handleCommand(String cmd)
{
    if (cmd == "SHUFFLE")
    {
        runShuffler();
    }
    else if (cmd == "DISPENSE")
    {
        runDispenser();
    }
    else if (cmd.startsWith("SET_SHUFFLE_SPEED "))
    {
        setShuffleSpeed(cmd.substring(18).toInt());
    }
    else if (cmd.startsWith("SET_DISPENSE_SPEED "))
    {
        setDispenseSpeed(cmd.substring(19).toInt());
    }
    else if (cmd.startsWith("SET_SHUFFLE_DURATION "))
    {
        setShuffleDuration(cmd.substring(21).toInt());
    }
    else if (cmd.startsWith("SET_DISPENSE_PULSE "))
    {
        setDispensePulse(cmd.substring(19).toInt());
    }
    else
    {
        Serial.print("UNKNOWN:");
        Serial.println(cmd);
    }
}

void DCMotorController::runShuffler()
{
    Serial.println("SHUFFLING");
    analogWrite(SHUFFLE_FWD, shuffleSpeed);
    analogWrite(SHUFFLE_REV, 0);
    delay(shuffleDuration);
    stopAll();
    Serial.println("SHUFFLE_DONE");
}

void DCMotorController::runDispenser()
{
    Serial.println("DISPENSING");
    analogWrite(DISPENSE_FWD, dispenseSpeed);
    analogWrite(DISPENSE_REV, 0);
    delay(dispensePulse);
    stopAll();
    Serial.println("DISPENSE_DONE");
}

void DCMotorController::stopAll()
{
    analogWrite(SHUFFLE_FWD, 0);
    analogWrite(SHUFFLE_REV, 0);
    analogWrite(DISPENSE_FWD, 0);
    analogWrite(DISPENSE_REV, 0);
}

void DCMotorController::setShuffleSpeed(int speed)
{
    shuffleSpeed = constrain(speed, 0, 255);
    Serial.print("SHUFFLE_SPEED:");
    Serial.println(shuffleSpeed);
}

void DCMotorController::setDispenseSpeed(int speed)
{
    dispenseSpeed = constrain(speed, 0, 255);
    Serial.print("DISPENSE_SPEED:");
    Serial.println(dispenseSpeed);
}

void DCMotorController::setShuffleDuration(unsigned long duration)
{
    shuffleDuration = duration;
    Serial.print("SHUFFLE_DURATION:");
    Serial.println(shuffleDuration);
}

void DCMotorController::setDispensePulse(unsigned long pulse)
{
    dispensePulse = pulse;
    Serial.print("DISPENSE_PULSE:");
    Serial.println(dispensePulse);
}

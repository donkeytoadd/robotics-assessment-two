#include "../headers/dc_motor_controller.h"

// sets variables for dc motors, allows for speed and duration of each motor to be adjusted
DCMotorController::DCMotorController()
{
    shuffleSpeed = 250;
    dispenseSpeed = 250;
    shuffleDuration = 5000;
    dispenseDuration = 2000;
}

// configures relevant pins to behave as outputs to the motors
void DCMotorController::begin()
{
    pinMode(SHUFFLE_FWD, OUTPUT);
    pinMode(SHUFFLE_REV, OUTPUT);
    pinMode(DISPENSE_FWD, OUTPUT);
    pinMode(DISPENSE_REV, OUTPUT);
    stopAll();
}

// recieves serial command from Raspberry Pi and performs the relevant function based on the message recieved.
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
    // these are mainly for debugging purposes, to allow for speed and duration to be adjusted whilst the arduino is running
    // rather than having to change the variable and reupload each time a change is made
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
    else if (cmd.startsWith("SET_DISPENSE_DURATION "))
    {
        setDispenseDuration(cmd.substring(19).toInt());
    }
    // a test that runs using the motors full speed using digitalWrite rather than analogWrite, since issues arose surrounding
    // the speed and power of the motors when connected to a coupler, or when weight is placed on them
    else if (cmd == "SHUFFLE_FULL")
    {
        Serial.println("SHUFFLE_FULL_POWER");
        digitalWrite(SHUFFLE_FWD, HIGH);
        digitalWrite(SHUFFLE_REV, LOW);
        delay(5000);
        stopAll();
        Serial.println("SHUFFLE_FULL_DONE");
    }
    // handles any unrecognised commands
    else
    {
        Serial.print("UNKNOWN:");
        Serial.println(cmd);
    }
}

// prints a logging line to the serial monitor, and then generates a PWM signal to the shuffle motor pins with the speed at which
// we want the motor to go. it does this for the specified shuffleDuration and then stops, with a log to say the action is complete
void DCMotorController::runShuffler()
{
    Serial.println("SHUFFLING");
    analogWrite(SHUFFLE_FWD, shuffleSpeed);
    analogWrite(SHUFFLE_REV, 0);
    delay(shuffleDuration);
    stopAll();
    Serial.println("SHUFFLE_DONE");
}

// prints a logging line to the serial monitor, and then generates a PWM signal to the dispense motor pins with the speed at which
// we want the motor to go. it does this for the specified dispenseDuration and then stops, with a log to say the action is complete
void DCMotorController::runDispenser()
{
    Serial.println("DISPENSING");
    analogWrite(DISPENSE_FWD, dispenseSpeed);
    analogWrite(DISPENSE_REV, 0);
    delay(dispenseDuration);
    stopAll();
    Serial.println("DISPENSE_DONE");
}

// sends a PWM signal to both dc motors with a value of 0, meaning the motors will stop running
void DCMotorController::stopAll()
{
    analogWrite(SHUFFLE_FWD, 0);
    analogWrite(SHUFFLE_REV, 0);
    analogWrite(DISPENSE_FWD, 0);
    analogWrite(DISPENSE_REV, 0);
}

// helper functions to allow for speed and duration to be adjusted in the serial monitor rather than reuploading every new change
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

void DCMotorController::setDispenseDuration(unsigned long duration)
{
    dispenseDuration = duration;
    Serial.print("DISPENSE_DURATION:");
    Serial.println(dispenseDuration);
}
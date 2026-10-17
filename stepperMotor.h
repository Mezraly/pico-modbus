#pragma once

#include "pico/stdlib.h"
#include "Config.h"

class StepperMotor {
public:
    StepperMotor();

    void init();

    void moveOneStep(int direction);

    void moveSteps(
        int steps,
        int direction
    );

    void stop();

private:
    const uint pins[4] = {
        Pins::MOTOR_IN1,
        Pins::MOTOR_IN2,
        Pins::MOTOR_IN3,
        Pins::MOTOR_IN4
    };

    int phase = 0;
};
#include "stepperMotor.h"

static const uint8_t SEQUENCE[8][4] = {
    {1, 0, 0, 0},
    {1, 1, 0, 0},
    {0, 1, 0, 0},
    {0, 1, 1, 0},
    {0, 0, 1, 0},
    {0, 0, 1, 1},
    {0, 0, 0, 1},
    {1, 0, 0, 1}
};

StepperMotor::StepperMotor() {
}

void StepperMotor::init() {

    for (int i = 0; i < 4; i++) {

        gpio_init(pins[i]);

        gpio_set_dir(
            pins[i],
            GPIO_OUT
        );

        gpio_put(
            pins[i],
            0
        );
    }
}

void StepperMotor::moveOneStep(
    int direction
) {

    phase =
        (phase + direction + 8) % 8;

    for (int i = 0; i < 4; i++) {

        gpio_put(
            pins[i],
            SEQUENCE[phase][i]
        );
    }
}

void StepperMotor::moveSteps(
    int steps,
    int direction
) {

    for (int i = 0; i < steps; i++) {

        moveOneStep(direction);

        sleep_ms(10);
    }

    stop();
}

void StepperMotor::stop() {

    for (int i = 0; i < 4; i++) {

        gpio_put(
            pins[i],
            0
        );
    }
}
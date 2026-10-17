#include <cstdio>

#include "GarageDoor.h"
#include "Config.h"

static constexpr uint64_t STEP_INTERVAL_US = 1000;
static constexpr uint64_t LED_BLINK_INTERVAL_US = 500000;

GarageDoor::GarageDoor(
    StepperMotor& motor,
    Button& sw0,
    Button& sw1,
    Button& sw2,
    Led& led0,
    Led& led1,
    Led& led2
)
    : motor(motor),
      sw0(sw0),
      sw1(sw1),
      sw2(sw2),
      led0(led0),
      led1(led1),
      led2(led2) {

    loadSavedState();
    updateLeds();

    gpio_init(Pins::LIMIT_A);
    gpio_set_dir(Pins::LIMIT_A, GPIO_IN);
    gpio_pull_up(Pins::LIMIT_A);

    gpio_init(Pins::LIMIT_B);
    gpio_set_dir(Pins::LIMIT_B, GPIO_IN);
    gpio_pull_up(Pins::LIMIT_B);

    gpio_init(Pins::ENCODER_A);
    gpio_set_dir(Pins::ENCODER_A, GPIO_IN);

    gpio_init(Pins::ENCODER_B);
    gpio_set_dir(Pins::ENCODER_B, GPIO_IN);

    lastEncoderA =
        gpio_get(Pins::ENCODER_A);

    lastEncoderB =
        gpio_get(Pins::ENCODER_B);

    updateLeds();
}

void GarageDoor::update() {

    updateEncoder();

    updateLeds();

    bool calibrationPressed =
        sw0.isPressed() &&
        sw2.isPressed();

    if (calibrationPressed &&
        calibrationButtonReleased) {

        calibrationButtonReleased =
            false;

        motor.stop();

        movementState =
            MovementState::Idle;

        startCalibration();

        return;
    }

    if (!sw0.isPressed() &&
        !sw2.isPressed()) {

        calibrationButtonReleased =
            true;
    }

    if (sw1.wasPressed()) {

        if (calibrationState ==
            CalibrationState::NotCalibrated) {

            printf(
                "Ovi ei ole kalibroitu\n"
            );

            return;
        }

        if (movementState ==
                MovementState::Opening ||
            movementState ==
                MovementState::Closing) {

            lastMovementBeforeStop =
                movementState;

            stopMovement();

            doorState =
                DoorState::InBetween;

            movementState =
                MovementState::Stopped;

            printf("STOP\n");

            saveCurrentState();

            updateLeds();

            return;
        }

        if (movementState ==
            MovementState::Stopped) {

            if (lastMovementBeforeStop ==
                MovementState::Opening) {

                startClosing();

                return;
            }

            if (lastMovementBeforeStop ==
                MovementState::Closing) {

                startOpening();

                return;
            }
        }

        if (doorState ==
            DoorState::Open) {

            startClosing();

            return;
        }

        if (doorState ==
            DoorState::Closed) {

            startOpening();

            return;
        }
        if (doorState == DoorState::InBetween) {
            startClosing();
            return;
        }
    }

    updateMovement();
}

void GarageDoor::updateLeds() {

    if (errorState ==
        ErrorState::DoorStuck) {

        led0.off();
        led1.off();

        uint64_t now =
            time_us_64();

        if (now - lastBlinkTimeUs >=
            LED_BLINK_INTERVAL_US) {

            lastBlinkTimeUs =
                now;

            errorLedOn =
                !errorLedOn;
        }

        if (errorLedOn) {

            led2.on();
        }
        else {

            led2.off();
        }

        return;
    }

    if (calibrationState ==
        CalibrationState::NotCalibrated) {

        led0.off();
        led1.off();
        led2.on();

        return;
    }

    led2.off();

    if (doorState ==
        DoorState::Open) {

        led0.on();
        led1.off();

        return;
    }

    if (doorState ==
        DoorState::Closed) {

        led0.off();
        led1.on();

        return;
    }

    led0.off();
    led1.off();
}

void GarageDoor::updateEncoder() {

    int a =
        gpio_get(Pins::ENCODER_A);

    int b =
        gpio_get(Pins::ENCODER_B);

    if (lastEncoderA == 0 &&
        a == 1) {

        encoderChanges++;

        if (b == 0) {

            encoderPosition++;
        }
        else {

            encoderPosition--;
        }
    }

    lastEncoderA = a;
    lastEncoderB = b;
}

void GarageDoor::startOpening() {

    printf("Avataan...\n");

    doorState =
        DoorState::InBetween;

    movementState =
        MovementState::Opening;

    lastStepTimeUs =
        time_us_64();

    updateLeds();
}

void GarageDoor::startClosing() {

    printf("Suljetaan...\n");

    doorState =
        DoorState::InBetween;

    movementState =
        MovementState::Closing;

    lastStepTimeUs =
        time_us_64();

    updateLeds();
}

void GarageDoor::stopMovement() {

    motor.stop();
}

void GarageDoor::updateMovement() {

    if (movementState !=
            MovementState::Opening &&
        movementState !=
            MovementState::Closing) {

        return;
    }

    uint64_t now =
        time_us_64();

    if (now - lastStepTimeUs <
        STEP_INTERVAL_US) {

        return;
    }

    lastStepTimeUs =
        now;

    if (movementState ==
        MovementState::Opening) {

        if (!gpio_get(Pins::LIMIT_A)) {

            motor.stop();

            movementState =
                MovementState::Idle;

            doorState =
                DoorState::Open;

            errorState =
                ErrorState::Normal;

            printf("Ovi: OPEN\n");

            saveCurrentState();

            updateLeds();

            return;
        }

        motor.moveOneStep(-1);

        return;
    }

    if (movementState ==
        MovementState::Closing) {

        if (!gpio_get(Pins::LIMIT_B)) {

            motor.stop();

            movementState =
                MovementState::Idle;

            doorState =
                DoorState::Closed;

            errorState =
                ErrorState::Normal;

            printf("Ovi: CLOSED\n");

            saveCurrentState();

            updateLeds();

            return;
        }

        motor.moveOneStep(1);

        return;
    }
}

void GarageDoor::startCalibration() {

    printf(
        "\nKalibrointi alkaa...\n"
    );

    calibrationState =
        CalibrationState::NotCalibrated;

    errorState =
        ErrorState::Normal;

    movementState =
        MovementState::Idle;

    doorTravelSteps = 0;

    encoderChanges = 0;
    encoderPosition = 0;

    lastEncoderA =
        gpio_get(Pins::ENCODER_A);

    lastEncoderB =
        gpio_get(Pins::ENCODER_B);

    updateLeds();

    const int MAX_STEPS = 30000;
    const int STEP_DELAY_MS = 1;

    printf(
        "Etsitaan CLOSED-raja...\n"
    );

    bool foundLimitB = false;

    for (int i = 0;
         i < MAX_STEPS;
         i++) {

        if (!gpio_get(Pins::LIMIT_B)) {

            motor.stop();

            printf(
                "CLOSED-raja loytyi\n"
            );

            foundLimitB = true;

            break;
        }

        motor.moveOneStep(1);

        sleep_ms(
            STEP_DELAY_MS
        );

        updateEncoder();
    }

    if (!foundLimitB) {

        motor.stop();

        errorState =
            ErrorState::DoorStuck;

        calibrationState =
            CalibrationState::NotCalibrated;

        printf(
            "VIRHE: CLOSED-rajaa ei loytynyt\n"
        );

        updateLeds();

        return;
    }

    sleep_ms(500);

    printf(
        "Etsitaan OPEN-raja...\n"
    );

    bool foundLimitA = false;

    for (int i = 0;
         i < MAX_STEPS;
         i++) {

        if (!gpio_get(Pins::LIMIT_A)) {

            motor.stop();

            doorTravelSteps =
                i;

            printf(
                "OPEN-raja loytyi\n"
            );

            foundLimitA = true;

            break;
        }

        motor.moveOneStep(-1);

        sleep_ms(
            STEP_DELAY_MS
        );

        updateEncoder();
    }

    if (!foundLimitA) {

        motor.stop();

        errorState =
            ErrorState::DoorStuck;

        calibrationState =
            CalibrationState::NotCalibrated;

        printf(
            "VIRHE: OPEN-rajaa ei loytynyt\n"
        );

        updateLeds();

        return;
    }

    motor.stop();

    calibrationState =
        CalibrationState::Calibrated;

    errorState =
        ErrorState::Normal;

    doorState =
        DoorState::Open;

    movementState =
        MovementState::Idle;

    saveCurrentState();

    printf(
        "\nKalibrointi valmis\n"
    );

    printf(
        "Liikealue: %d askelta\n",
        doorTravelSteps
    );

    printf(
        "Ovi: OPEN\n\n"
    );

    updateLeds();
}

void GarageDoor::loadSavedState() {
    SavedStateData data;
    if (NVM::load(data)) {
        doorState = static_cast<DoorState>(data.doorState);
        calibrationState = static_cast<CalibrationState>(data.calibrationState);
        doorTravelSteps = data.doorTravelSteps;
        printf("Ladattu flashista: DoorState=%d, CalibrationState=%d, TravelSteps=%d\n",
               (int)doorState, (int)calibrationState, doorTravelSteps);
    } else {
        printf("Ei flash tallennusta.\n");
    }
}

void GarageDoor::saveCurrentState() {
    SavedStateData data;
    data.doorState = static_cast<uint8_t>(doorState);
    data.calibrationState = static_cast<uint8_t>(calibrationState);
    data.doorTravelSteps = doorTravelSteps;
    NVM::save(data);
    printf("Tallennettu flashiin\n");
}
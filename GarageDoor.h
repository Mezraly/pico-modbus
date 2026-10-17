#pragma once

#include <cstdint>

#include "Button.h"
#include "Led.h"
#include "stepperMotor.h"
#include "NVM.h"

enum class DoorState {
    Open,
    Closed,
    InBetween
};

enum class ErrorState {
    Normal,
    DoorStuck
};

enum class CalibrationState {
    NotCalibrated,
    Calibrated
};

enum class MovementState {
    Idle,
    Opening,
    Closing,
    Stopped
};

class GarageDoor {

public:

    GarageDoor(
        StepperMotor& motor,
        Button& sw0,
        Button& sw1,
        Button& sw2,
        Led& led0,
        Led& led1,
        Led& led2
    );

    void update();

private:

    StepperMotor& motor;

    Button& sw0;
    Button& sw1;
    Button& sw2;

    Led& led0;
    Led& led1;
    Led& led2;

    DoorState doorState =
        DoorState::InBetween;

    ErrorState errorState =
        ErrorState::Normal;

    CalibrationState calibrationState =
        CalibrationState::NotCalibrated;

    MovementState movementState =
        MovementState::Idle;

    MovementState lastMovementBeforeStop =
        MovementState::Idle;

    bool calibrationButtonReleased = true;

    int doorTravelSteps = 0;

    uint64_t lastStepTimeUs = 0;

    int lastEncoderA = 0;
    int lastEncoderB = 0;
    int encoderChanges = 0;
    int encoderPosition = 0;

    uint64_t lastBlinkTimeUs = 0;
    bool errorLedOn = false;

    void loadSavedState();
    void saveCurrentState();

    void startCalibration();

    void startOpening();

    void startClosing();

    void stopMovement();

    void updateMovement();

    void updateEncoder();

    void updateLeds();
};
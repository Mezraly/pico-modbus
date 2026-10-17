#pragma once

#include <cstdint>
#include "pico/stdlib.h"

class Button {
public:
    explicit Button(uint pin)
        : pin(pin) {

        gpio_init(pin);
        gpio_set_dir(pin, GPIO_IN);
        gpio_pull_up(pin);

        lastRawState = isPressed();
        stableState = lastRawState;
        lastChangeTime = time_us_64();
    }

    bool isPressed() const {
        return !gpio_get(pin);
    }

    bool wasPressed() {

        bool rawState = isPressed();
        uint64_t now = time_us_64();

        if (rawState != lastRawState) {
            lastRawState = rawState;
            lastChangeTime = now;
        }

        if ((now - lastChangeTime) >= DEBOUNCE_US) {

            if (stableState != rawState) {

                stableState = rawState;

                if (stableState) {
                    return true;
                }
            }
        }

        return false;
    }

private:
    uint pin;

    bool lastRawState = false;
    bool stableState = false;

    uint64_t lastChangeTime = 0;

    static constexpr uint64_t DEBOUNCE_US = 30000;
};
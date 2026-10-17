#pragma once

#include "pico/stdlib.h"

class Led {
public:
    explicit Led(uint pin)
        : pin(pin) {

        gpio_init(pin);
        gpio_set_dir(pin, GPIO_OUT);
        gpio_put(pin, 0);
    }

    void on() {
        gpio_put(pin, 1);
    }

    void off() {
        gpio_put(pin, 0);
    }

    void toggle() {
        gpio_put(pin, !gpio_get(pin));
    }

private:
    uint pin;
};
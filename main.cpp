#include <cstdio>

#include "pico/stdlib.h"
#include "pico/stdio_usb.h"

#include "Config.h"
#include "Button.h"
#include "Led.h"
#include "stepperMotor.h"
#include "GarageDoor.h"

int main() {

    stdio_init_all();

    while (!stdio_usb_connected()) {

        sleep_ms(100);
    }

    sleep_ms(500);

    printf(
        "\n=== GARAGE DOOR ===\n"
    );

    StepperMotor motor;

    motor.init();

    Button sw0(Pins::SW0);
    Button sw1(Pins::SW1);
    Button sw2(Pins::SW2);

    Led led0(Pins::LED0);
    Led led1(Pins::LED1);
    Led led2(Pins::LED2);

    GarageDoor garageDoor(
        motor,
        sw0,
        sw1,
        sw2,
        led0,
        led1,
        led2
    );

    printf("Valmis\n");

    while (true) {

        garageDoor.update();

        sleep_us(100);
    }
}
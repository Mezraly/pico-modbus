# Set minimum required version of CMake
cmake_minimum_required(VERSION 3.6)

# Set board type because we are building for PicoW
set(PICO_BOARD pico_w)

# Include build functions from Pico SDK
include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

# Set name of project and C/C++ standards
project(pico-modbus C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Initialize Pico SDK
pico_sdk_init()

add_compile_options(
        -Wall
        -Wno-format
        -Wno-unused-function
        -Wno-maybe-uninitialized
)

# MQTT sources
file(GLOB MQTT_SOURCES
        "paho.mqtt.embedded-c/MQTTPacket/src/*.c"
)

# Project source files
add_executable(${PROJECT_NAME}

        main.cpp

        # Garage door
        GarageDoor.cpp
        GarageDoor.h

        # Stepper motor
        stepperMotor.cpp
        stepperMotor.h

        # Our helper classes
        Led.h
        Button.h
        Config.h

        # UART
        uart/PicoUart.cpp
        uart/PicoUart.h
        uart/RingBuffer.cpp
        uart/RingBuffer.h

        # MQTT
        mqtt/Countdown.cpp
        mqtt/Countdown.h
        mqtt/IPStack.cpp
        mqtt/IPStack.h
        mqtt/lwipopts.h

        ${MQTT_SOURCES}

        # I2C
        i2c/PicoI2CDevice.cpp
        i2c/PicoI2CDevice.h
        i2c/PicoI2CBus.cpp
        i2c/PicoI2CBus.h

        # SPI
        spi/PicoSPIBus.cpp
        spi/PicoSPIBus.h
        spi/PicoSPIDevice.cpp
        spi/PicoSPIDevice.h
)

# Create map/bin/hex/uf2 files
pico_add_extra_outputs(${PROJECT_NAME})

target_include_directories(${PROJECT_NAME} PRIVATE

        mqtt
        uart
        i2c
        spi

        # Tarvitaan vielä nykyisille I2C/SPI helper-luokille
        display

        paho.mqtt.embedded-c/MQTTPacket/src
        paho.mqtt.embedded-c/MQTTClient/src
)

# Libraries
target_link_libraries(${PROJECT_NAME}

        pico_stdlib

        hardware_i2c
        hardware_spi
        hardware_gpio
        hardware_flash

        pico_cyw43_arch_lwip_poll
)

# USB serial ON
# UART serial OFF
pico_enable_stdio_usb(${PROJECT_NAME} 1)
pico_enable_stdio_uart(${PROJECT_NAME} 0)
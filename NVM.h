#pragma once

#include <cstdint>
#include <cstring>
#include "pico/stdlib.h"
#include "hardware/flash.h"
#include "hardware/sync.h"


#define FLASH_TARGET_OFFSET (2 * 1024 * 1024 - FLASH_SECTOR_SIZE)
#define FLASH_MAGIC_KEY     0xDEADBEEF

struct SavedStateData {
    uint32_t magic;
    uint8_t doorState;        // 0: Open, 1: Closed, 2: InBetween
    uint8_t calibrationState; // 0: NotCalibrated, 1: Calibrated
    int doorTravelSteps;
};

class NVM {
public:
    static bool load(SavedStateData& data) {
        const uint8_t* flash_target_contents = (const uint8_t *)(XIP_BASE + FLASH_TARGET_OFFSET);
        memcpy(&data, flash_target_contents, sizeof(SavedStateData));
        return (data.magic == FLASH_MAGIC_KEY);
    }

    static void save(const SavedStateData& data) {
        SavedStateData dataToSave = data;
        dataToSave.magic = FLASH_MAGIC_KEY;

        uint8_t buffer[FLASH_PAGE_SIZE];
        memset(buffer, 0xFF, sizeof(buffer));
        memcpy(buffer, &dataToSave, sizeof(SavedStateData));

        uint32_t interrupts = save_and_disable_interrupts();
        flash_range_erase(FLASH_TARGET_OFFSET, FLASH_SECTOR_SIZE);
        flash_range_program(FLASH_TARGET_OFFSET, buffer, FLASH_PAGE_SIZE);
        restore_interrupts(interrupts);
    }
};
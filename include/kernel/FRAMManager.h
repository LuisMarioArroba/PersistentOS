#ifndef FRAM_MANAGER_H
#define FRAM_MANAGER_H

#include <Arduino.h>

#include "kernel/PersistentState.h"


class FRAMManager
{

public:

    FRAMManager();


    bool begin();


    bool isAvailable() const;


    bool save(
        const PersistentState& state
    );


    bool load(
        PersistentState& state
    );


    bool clear();


private:

    bool initialized;

    bool hardwareAvailable;

    uint32_t baseAddress;


    bool writeMemory(
        uint32_t address,
        const uint8_t* data,
        size_t size
    );


    bool readMemory(
        uint32_t address,
        uint8_t* data,
        size_t size
    );

};

#endif
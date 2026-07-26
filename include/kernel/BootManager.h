#ifndef BOOT_MANAGER_H
#define BOOT_MANAGER_H

#include <Arduino.h>

#include "kernel/FRAMManager.h"
#include "kernel/PersistentState.h"


class BootManager
{

public:

    BootManager(
        FRAMManager& fram
    );


    bool begin();


    bool wasRecovery();


    bool hasPersistentMemory();


    PersistentState& getState();



private:

    FRAMManager& framManager;

    PersistentState state;

    bool recoveryDetected;

};


#endif
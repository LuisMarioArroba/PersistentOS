#ifndef BOOT_MANAGER_H
#define BOOT_MANAGER_H

#include <Arduino.h>

#include "kernel/FRAMManager.h"
#include "kernel/PersistentState.h"

/*
Inicializa el sistema y
recupera estado previo si existe durante boot
Identifica el estado persistente actual
*/
class BootManager
{

public:
    BootManager(FRAMManager& fram);
    bool begin();
    bool wasRecovery();
    PersistentState& getState();
private:
    FRAMManager& framManager;
    PersistentState state;
    bool recoveryDetected;
};

#endif
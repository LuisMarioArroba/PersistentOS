#ifndef FRAM_MANAGER_H
#define FRAM_MANAGER_H

#include <Arduino.h>

#include "kernel/PersistentState.h"

/*
Inicialización de la memoria FRAM
Guarda todo el estado persistente del sistema
Recupera el último estado guardado
Limpia la memoria persistente
*/
class FRAMManager
{

public:

    FRAMManager();
    bool begin();
    bool save(const PersistentState& state);
    bool load(PersistentState& state);
    bool clear();

private:
    bool initialized;
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
#include "kernel/FRAMManager.h"
/*
Simulated Persistent Memory
*/

static PersistentState persistentMemory;
static bool memoryAvailable = false;

FRAMManager::FRAMManager(){
    initialized = false;
    baseAddress = 0;
}

bool FRAMManager::begin(){
    initialized = true;
    Serial.println("[FRAM] Initialized");
    return true;
}

bool FRAMManager::save(const PersistentState& state){
    if(!initialized){
        return false;
    }

    memcpy(&persistentMemory,&state,sizeof(PersistentState));
    memoryAvailable = true;
    Serial.println("[FRAM] State saved");
    return true;
}

bool FRAMManager::load(PersistentState& state){
    if(!initialized){
        return false;
    }
    if(!memoryAvailable){
        Serial.println("[FRAM] No previous state");
        return false;
    }
    memcpy(&state,&persistentMemory,sizeof(PersistentState));
    Serial.println("[FRAM] State restored");
    return true;
}

bool FRAMManager::clear(){
    if(!initialized){
        return false;
    }
    memset(&persistentMemory,0,sizeof(PersistentState));
    memoryAvailable = false;
    Serial.println("[FRAM] Memory cleared");
    return true;
}

/*
Nota personal
Requerimiento equipo físico
*/

bool FRAMManager::writeMemory(uint32_t address,const uint8_t* data,size_t size){
    return true;
}

bool FRAMManager::readMemory(uint32_t address,uint8_t* data,size_t size){
    return true;
}
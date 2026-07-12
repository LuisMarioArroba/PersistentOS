#include "kernel/BootManager.h"

BootManager::BootManager(FRAMManager& fram):framManager(fram){
    recoveryDetected = false;
}

bool BootManager::begin(){
    Serial.println("[BOOT] Starting...");
    if(!framManager.begin()){
        Serial.println("[BOOT] FRAM initialization failed");
        return false;
    }

    if(framManager.load(state)){
        recoveryDetected = true;
        state.kernel.recovering = true;
        state.kernel.bootCount++;
        Serial.println("[BOOT] Previous state recovered");
    }
    else{
        memset(&state, 0,sizeof(PersistentState));
        state.kernel.bootCount = 1;
        state.kernel.recovering = false;
        Serial.println("[BOOT] Fresh system start");
    }

    framManager.save(state);
    Serial.print("[BOOT] Boot number: ");
    Serial.println(state.kernel.bootCount);
    return true;
}

bool BootManager::wasRecovery(){
    return recoveryDetected;
}

PersistentState& BootManager::getState(){
    return state;
}
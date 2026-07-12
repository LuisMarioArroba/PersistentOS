#include <Arduino.h>
#include "config/Config.h"

// Kernel
#include "kernel/Scheduler.h"
#include "kernel/Task.h"
#include "kernel/FRAMManager.h"
#include "kernel/BootManager.h"

// Hardware
#include "hardware/Sensors.h"

uint32_t systemTick = 0;
Scheduler scheduler;
FRAMManager fram;
BootManager bootManager(fram);
Sensor sensorManager;

// Tasks
void sensorTask(){
    if(sensorManager.update()){
        Serial.print("[Sensor] ");
        Serial.println(sensorManager.getValue());
    }
}

void communicationTask(){
    Serial.print("[Communication] Sending: ");
    Serial.println(sensorManager.getValue());
}

void ledTask(){
    static bool ledState=false;
    ledState=!ledState;
    digitalWrite(LED_PIN,ledState);
    Serial.print("[LED] ");
    Serial.println(ledState ? "ON":"OFF");
}

// Task registration
Task sensor ={"Sensor",sensorTask,READY,50,0};

Task communication ={"Communication",communicationTask,READY,20,0};

Task led ={"LED",ledTask,READY,5,0};

// Setup
void setup(){
    Serial.begin(115200);
    pinMode(LED_PIN, OUTPUT);

    bootManager.begin();
    scheduler.attachState(&bootManager.getState());

    if(bootManager.wasRecovery()){
        Serial.println("[BOOT] Recovery mode");
    }
    else{
        Serial.println("[BOOT] Normal startup");
    }

    if(sensorManager.begin()){
        Serial.println("[Sensor] OK");
    }
    else{
        Serial.println("[Sensor] NOT FOUND");
    }

    scheduler.addTask(&sensor);
    scheduler.addTask(&communication);
    scheduler.addTask(&led);
    Serial.println();
    Serial.println("===== PersistentOS Started =====");
}

void loop(){
    systemTick++;
    scheduler.execute();
    PersistentState& state =bootManager.getState();
    state.kernel.systemTick =systemTick;
    state.sensorBuffer.count++;

    fram.save(state);
    delay(SYSTEM_TICK_MS);
}
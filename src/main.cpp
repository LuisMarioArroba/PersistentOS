#include <Arduino.h>

#include "config/Config.h"


//====================================================
// Kernel
//====================================================

#include "kernel/Task.h"
#include "kernel/TaskManager.h"
#include "kernel/Scheduler.h"
#include "kernel/PersistentState.h"
#include "kernel/FRAMManager.h"
#include "kernel/BootManager.h"


//====================================================
// Services
//====================================================

#include "services/SensorService.h"
#include "services/SensorBuffer.h"


//====================================================
// Hardware
//====================================================

#include "hardware/Sensors.h"


//====================================================
// System Tick
//====================================================

uint32_t systemTick = 0;


//====================================================
// Kernel Objects
//====================================================

TaskManager taskManager;

Scheduler scheduler;

FRAMManager fram;

BootManager bootManager(fram);


//====================================================
// Hardware
//====================================================

Sensor sensorManager;


//====================================================
// Services
//====================================================

SensorBuffer sensorBuffer;

SensorService sensorService;


//====================================================
// Tasks
//====================================================

void sensorTask()
{
    sensorService.execute();
}



void communicationTask()
{
    Serial.print("[Communication] Sending: ");

    Serial.println(
        sensorService.getLastValue()
    );
}



void ledTask()
{
    static bool ledState = false;


    ledState = !ledState;


    digitalWrite(
        LED_PIN,
        ledState
    );

/*
    Serial.print("[LED] ");

    Serial.println(
        ledState ? "ON" : "OFF"
    );
    */
}


//====================================================
// Task Registration
//====================================================

Task sensor =
{
    "Sensor",
    sensorTask,
    READY,
    50,
    0,
    0
};



Task communication =
{
    "Communication",
    communicationTask,
    READY,
    20,
    0,
    0
};



Task led =
{
    "LED",
    ledTask,
    READY,
    5,
    0,
    0
};


//====================================================
// Setup
//====================================================

void setup()
{

    Serial.begin(115200);


    pinMode(
        LED_PIN,
        OUTPUT
    );


    //------------------------------------------------
    // Boot
    //------------------------------------------------

    bootManager.begin();



    //------------------------------------------------
    // Persistent Buffer
    //------------------------------------------------

    sensorBuffer.attach(
        &bootManager.getState().sensorBuffer
    );



    //------------------------------------------------
    // Scheduler
    //------------------------------------------------

    scheduler.attachTaskManager(
        &taskManager
    );


    scheduler.attachState(
        &bootManager.getState()
    );



    //------------------------------------------------
    // Boot Information
    //------------------------------------------------

    if(bootManager.wasRecovery())
    {

        Serial.println(
            "[BOOT] Recovery Mode"
        );

    }
    else
    {

        Serial.println(
            "[BOOT] Normal Startup"
        );

    }



    //------------------------------------------------
    // Hardware
    //------------------------------------------------

    if(sensorManager.begin())
    {

        Serial.println(
            "[Sensor] OK"
        );

    }
    else
    {

        Serial.println(
            "[Sensor] NOT FOUND"
        );

    }



    //------------------------------------------------
    // Services
    //------------------------------------------------

    sensorService.begin(
        &sensorManager,
        &sensorBuffer
    );



    //------------------------------------------------
    // Tasks
    //------------------------------------------------

    taskManager.addTask(
        &sensor
    );


    taskManager.addTask(
        &communication
    );


    taskManager.addTask(
        &led
    );



    Serial.println();


    Serial.println(
        "===== PersistentOS Started ====="
    );

}


//====================================================
// Loop
//====================================================

void loop()
{

    systemTick++;


    PersistentState& state =
        bootManager.getState();


    state.kernel.systemTick =
        systemTick;


    scheduler.execute();


    fram.save(state);


    if(systemTick % 100 == 0)
    {
        sensorService.printBufferStatus();
    }


    delay(SYSTEM_TICK_MS);

}
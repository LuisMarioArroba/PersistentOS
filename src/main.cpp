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
#include "kernel/ExecutionCheckpoint.h"
#include "kernel/ResumeManager.h"


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


ExecutionCheckpoint checkpointManager;


ResumeManager resumeManager;



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

}



//====================================================
// Task Registration
//====================================================

Task sensor =
{
    TASK_SENSOR,
    "Sensor",
    sensorTask,
    READY,
    50,
    0,
    0
};



Task communication =
{
    TASK_COMMUNICATION,
    "Communication",
    communicationTask,
    READY,
    20,
    0,
    0
};



Task led =
{
    TASK_LED,
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
    // Checkpoint System
    //------------------------------------------------

    checkpointManager.attachState(
        &bootManager.getState()
    );



    //------------------------------------------------
    // Resume System
    //------------------------------------------------

    resumeManager.begin(
        &bootManager.getState(),
        bootManager.wasRecovery()
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


    scheduler.attachCheckpoint(
        &checkpointManager
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
        &sensorBuffer,
        &resumeManager,
        &checkpointManager
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



    fram.save(
        state
    );



    if(systemTick % 100 == 0)
    {

        sensorService.printBufferStatus();

    }



    delay(
        SYSTEM_TICK_MS
    );

}
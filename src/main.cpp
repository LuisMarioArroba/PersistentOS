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
#include "kernel/CommunicationManager.h"


//====================================================
// Services
//====================================================

#include "services/SensorService.h"
#include "services/SensorBuffer.h"
#include "services/FailureManager.h"
#include "services/CommunicationService.h"
#include "services/CommunicationBuffer.h"


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

FailureManager failureManager;

bool communicationInitialized = false;

//====================================================
// Communication
//====================================================

CommunicationManager communicationManager;

CommunicationBuffer communicationBuffer;

CommunicationService communicationService;

bool systemFailed = false;

//====================================================
// Tasks
//====================================================

void sensorTask()
{
    sensorService.execute();
}



void communicationTask()
{

    communicationService.execute();

}



bool checkFailureButton()
{

    static bool lastState = HIGH;


    bool currentState =
        digitalRead(
            TEST_BUTTON_PIN
        );


    bool pressed =
        (
            lastState == HIGH &&
            currentState == LOW
        );


    lastState = currentState;


    return pressed;

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


    pinMode(
        TEST_BUTTON_PIN,
        INPUT_PULLUP
    );



    //------------------------------------------------
    // Boot
    //------------------------------------------------

    if(!bootManager.begin())
    {

        Serial.println(
            "[BOOT] Initialization failed"
        );


        while(true)
        {

        }

    }


    failureManager.begin();

    //------------------------------------------------
    // Resume System
    //------------------------------------------------

    resumeManager.begin(
        &bootManager.getState(),
        bootManager.wasRecovery()
    );



    //------------------------------------------------
    // Communication Manager
    //------------------------------------------------

    communicationInitialized =
        communicationManager.begin();


    if(communicationInitialized)
    {
        Serial.println(
            "[COMM] Manager Ready"
        );
    }
    else
    {
        Serial.println(
            "[COMM] Manager unavailable"
        );
    }

    //------------------------------------------------
    // Persistent Buffer
    //------------------------------------------------

    sensorBuffer.attach(
        &bootManager.getState().sensorBuffer
    );

    communicationBuffer.attach(
        &bootManager.getState().communicationBuffer
    );



    //------------------------------------------------
    // Checkpoint System
    //------------------------------------------------

    checkpointManager.attachState(
        &bootManager.getState()
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

    if(
        bootManager.wasRecovery()
    )
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



    if(
        bootManager.hasPersistentMemory()
    )
    {

        Serial.println(
            "[MEMORY] Persistent Storage Available"
        );

    }
    else
    {

        Serial.println(
            "[MEMORY] Volatile Mode"
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
        &checkpointManager,
        &failureManager
    );

    communicationService.begin(
        &communicationBuffer,
        &communicationManager,
        &resumeManager,
        &checkpointManager,
        &sensorService,
        &failureManager
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

        //------------------------------------------------
    // Failure Simulation
    //------------------------------------------------

    if(
    checkFailureButton()
)
{


    if(!systemFailed)
        {

            Serial.println(
                "[TEST] Failure Requested"
            );


            failureManager.triggerFailure();


            systemFailed = true;


        }
        else
        {

            Serial.println(
                "[TEST] Manual Recovery"
            );

            failureManager.clear();

            sensorService.resumeAfterFailure();


            communicationService.resumeAfterFailure();


            failureManager.clear();


            systemFailed=false;

        }

    }

    PersistentState& state =
        bootManager.getState();



    state.kernel.systemTick =
        systemTick;



    //------------------------------------------------
    // Execute Scheduler
    //------------------------------------------------

    scheduler.execute();



    //------------------------------------------------
    // Save Persistent State
    //------------------------------------------------

    if(
        bootManager.hasPersistentMemory()
    )
    {

        fram.save(
            state
        );

    }



    //------------------------------------------------
    // Buffer Debug
    //------------------------------------------------

    if(
        systemTick % 100 == 0
    )
    {

        sensorService.printBufferStatus();

    }



    delay(
        SYSTEM_TICK_MS
    );

}
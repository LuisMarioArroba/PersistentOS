#include <Arduino.h>

#include "config/Config.h"


//====================================================
// Kernel
//====================================================

#include "kernel/FRAMManager.h"
#include "kernel/BootManager.h"
#include "kernel/ExecutionCheckpoint.h"
#include "kernel/ResumeManager.h"
#include "kernel/CommunicationManager.h"
#include "kernel/EnergyManager.h"
#include "kernel/SimulationManager.h"
#include "kernel/BehaviorManager.h"
#include "kernel/EnergyPredictionManager.h"

//====================================================
// Services
//====================================================

#include "services/SensorService.h"
#include "services/SensorBuffer.h"

#include "services/CommunicationService.h"
#include "services/CommunicationBuffer.h"

#include "services/FailureManager.h"

#include "services/AlarmManager.h"

//====================================================
// Hardware
//====================================================

#include "hardware/Sensors.h"


//====================================================
// Testing
//====================================================

#include "test/TestManager.h"


//====================================================
// Persistent kernel
//====================================================

FRAMManager fram;

BootManager bootManager(
    fram
);

ExecutionCheckpoint checkpointManager;

ResumeManager resumeManager;


//====================================================
// Hardware
//====================================================

Sensor sensorManager;


//====================================================
// Buffers
//====================================================

SensorBuffer sensorBuffer;

CommunicationBuffer communicationBuffer;


//====================================================
// Managers
//====================================================

CommunicationManager communicationManager;

FailureManager failureManager;

EnergyManager energyManager;

EnergyPredictionManager energyPredictionManager;

//--------------------------------------------------
// Predicción del lado remoto: se alimenta con la
// energía que el nodo remoto reporta en cada DATA
// recibido (ver CommunicationService::handleIncomingData),
// para poder estimar --- junto con energyPredictionManager,
// el lado local --- una ventana de comunicación
// probabilística sin conocer el comportamiento real
// de la fuente de alimentación de ninguno de los dos.
//--------------------------------------------------

EnergyPredictionManager remoteEnergyPrediction;

SimulationManager simulationManager;

AlarmManager alarmManager;

BehaviorManager behaviorManager;

//====================================================
// Services
//====================================================

SensorService sensorService;

CommunicationService communicationService;


//====================================================
// Test manager
//====================================================

TestManager testManager;


//====================================================
// Setup
//====================================================

void setup()
{
    Serial.begin(
        115200
    );


    pinMode(
        LED_PIN,
        OUTPUT
    );


    pinMode(
        TEST_BUTTON_PIN,
        INPUT_PULLUP
    );


    delay(
        1000
    );


    Serial.println();

    Serial.println(
        "================================="
    );

    Serial.println(
        "      PERSISTENT OS TEST MODE"
    );

    Serial.println(
        "================================="
    );

    Serial.println();

    simulationManager.begin();

    alarmManager.begin(
        &communicationService
    );


    //--------------------------------------------------
    // Boot
    //--------------------------------------------------

    if(
        !bootManager.begin()
    )
    {
        Serial.println(
            "[BOOT] ERROR"
        );

        return;
    }


    //--------------------------------------------------
    // Resume manager
    //
    // IMPORTANT:
    //
    // Do NOT hardcode false.
    //
    // If BootManager recovered a previous persistent
    // state, recovery mode is enabled.
    //--------------------------------------------------

    resumeManager.begin(

        &bootManager.getState(),

        bootManager.wasRecovery()

    );


    //--------------------------------------------------
    // Persistent sensor buffer
    //--------------------------------------------------

    sensorBuffer.attach(

        &bootManager
            .getState()
            .sensorBuffer

    );


    //--------------------------------------------------
    // Persistent communication buffer
    //--------------------------------------------------

    communicationBuffer.attach(

        &bootManager
            .getState()
            .communicationBuffer

    );


    //--------------------------------------------------
    // Hardware
    //--------------------------------------------------

    sensorManager.begin();

    if(
        communicationManager.begin()
    )
    {

        Serial.println(
            "[SYSTEM] Communication manager ready"
        );

    }
    else
    {

        Serial.println(
            "[SYSTEM] Communication manager FAILED"
        );

    }

     //--------------------------------------------------
    // Checkpoint manager
    //--------------------------------------------------

    checkpointManager.attachState(

        &bootManager.getState()

    );

    checkpointManager.attachFRAM(
        &fram
    );

    //--------------------------------------------------
    // Failure manager
    //--------------------------------------------------

    failureManager.begin();


    //--------------------------------------------------
    // Energy manager
    //--------------------------------------------------

    energyManager.begin(
        100.0
    );
    energyPredictionManager.begin();
    remoteEnergyPrediction.begin();
    behaviorManager.begin();

    //--------------------------------------------------
    // Sensor service
    //--------------------------------------------------

    sensorService.begin(

        &sensorManager,

        &sensorBuffer,

        &resumeManager,

        &checkpointManager,

        &failureManager,

        &bootManager.getState()
    );


    //--------------------------------------------------
    // Communication service
    //--------------------------------------------------

    communicationService.begin(

        &communicationBuffer,

        &communicationManager,

        &resumeManager,

        &checkpointManager,

        &sensorService,

        &failureManager,

        &bootManager.getState(),

        &energyManager,

        &energyPredictionManager,

        &remoteEnergyPrediction,

        &behaviorManager

    );


    //--------------------------------------------------
    // Test manager
    //--------------------------------------------------

    testManager.begin(

        &checkpointManager,

        &failureManager,

        &energyManager

    );


    //--------------------------------------------------
    // System information
    //--------------------------------------------------

    Serial.println();

    Serial.println(
        "[SYSTEM] Sensor service ready"
    );

    Serial.println(
        "[SYSTEM] Communication service ready"
    );

    Serial.println(
        "[SYSTEM] Failure manager ready"
    );

    Serial.println(
        "[SYSTEM] Energy manager ready"
    );


    //--------------------------------------------------
    // Recovery information
    //--------------------------------------------------

    if(
        bootManager.wasRecovery()
    )
    {
        Serial.println();

        Serial.println(
            "[SYSTEM] ================================="
        );

        Serial.println(
            "[SYSTEM] RECOVERY BOOT DETECTED"
        );

        Serial.println(
            "[SYSTEM] ================================="
        );


        for(
            uint8_t i = 0;
            i < MAX_TASKS;
            i++
        )
        {
            Serial.print(
                "[SYSTEM] Task "
            );

            Serial.print(
                i
            );

            Serial.print(
                " checkpoint: "
            );

            Serial.print(
                bootManager
                    .getState()
                    .tasks[i]
                    .checkpoint
            );

            Serial.print(
                " | progress: "
            );

            Serial.print(
                bootManager
                    .getState()
                    .tasks[i]
                    .progress
            );

            Serial.print(
                "% | completed: "
            );

            Serial.println(
                bootManager
                    .getState()
                    .tasks[i]
                    .completed
                        ? "YES"
                        : "NO"
            );
        }

        Serial.println(
            "[SYSTEM] ================================="
        );

        Serial.println();
    }
}


//====================================================
// Main loop
//====================================================

void loop()
{

    //--------------------------------------------------
    // 1. Sensor
    //--------------------------------------------------

    sensorService.execute();


    //--------------------------------------------------
    // 2. Alarm: umbral de temperatura (Config.h),
    // prioridad sobre la telemetría de rutina vía
    // CommunicationService::requestAlarm().
    //--------------------------------------------------

    alarmManager.execute(
        sensorManager.getValue(),
        sensorManager.isConnected()
    );

    //--------------------------------------------------
    // 3. Communication state
    //--------------------------------------------------

    communicationManager.updateConnectionState();


    //--------------------------------------------------
    // 4. Communication
    //--------------------------------------------------

    communicationService.execute();


    //--------------------------------------------------
    // 5. Failure tests
    //--------------------------------------------------

    testManager.execute();

    //--------------------------------------------------
    // 6. Simulation
    //--------------------------------------------------

    simulationManager.execute();


    //--------------------------------------------------
    // 7. Energy (local, alimenta energyPredictionManager
    // para la ventana de comunicación del lado local)
    //--------------------------------------------------

    energyManager.execute();

    energyPredictionManager.observe(
        energyManager.getEnergy()
    );

    energyPredictionManager.update();

    delay(
        100
    );

}
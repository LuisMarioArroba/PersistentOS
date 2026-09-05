#include <Arduino.h>

#include <esp_system.h>

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
// Reporte (build persistentos1_reporte)
//====================================================

#include "report/ReportLogger.h"


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
// Predicción del lado remoto (ver main.cpp): se
// alimenta con la energía que el nodo remoto reporta
// en cada DATA recibido.
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
// Report: tageo de escenario (independiente del menú
// de simulación de energía de TestManager, que usa las
// teclas '1'-'4', 's'/'S' y 'm'/'M')
//
// Los 7 escenarios son los de la Sección de Resultados
// del paper. 1, 4, 5, 6 y 7 corresponden 1:1 a los
// puntos de falla que ya dispara TestManager con el
// botón físico (TEST_BUTTON_PIN); 2 y 3 dependen del
// interruptor de energía físico y deben tagearse a mano
// en el instante en que se accionan.
//====================================================

namespace
{

    //--------------------------------------------------
    // Forward declaration: definida más abajo.
    //--------------------------------------------------

    void toggleAutoScenario();


    struct ScenarioTag
    {
        char key;
        uint8_t id;
        const char* name;
    };

    const ScenarioTag SCENARIOS[7] =
    {
        { 'a', 1, "NORMAL" },
        { 'b', 2, "REMOTE_OFF" },
        { 'c', 3, "ACK_DELAYED" },
        { 'd', 4, "CUT_WAIT_ACK" },
        { 'e', 5, "CUT_BEFORE_SEND" },
        { 'f', 6, "CUT_AFTER_CREATE" },
        { 'g', 7, "CUT_RECEIVER_ACK_LOST" }
    };

    void printScenarioMenu()
    {

        Serial.println();

        Serial.println(
            "[REPORT] ================================="
        );

        Serial.println(
            "[REPORT] Tagueo de escenario (paper, Sección 4)"
        );

        Serial.println(
            "[REPORT] ================================="
        );

        for(
            uint8_t i = 0;
            i < 7;
            i++
        )
        {

            Serial.print(
                "[REPORT] "
            );

            Serial.print(
                SCENARIOS[i].key
            );

            Serial.print(
                " -> "
            );

            Serial.print(
                SCENARIOS[i].id
            );

            Serial.print(
                ". "
            );

            Serial.println(
                SCENARIOS[i].name
            );

        }

        Serial.println(
            "[REPORT] h -> mostrar este menú"
        );

        Serial.println(
            "[REPORT] r -> activar/desactivar ciclo automático "
            "(aleatorio, 1/4/5/6/7 cada 20s)"
        );

        Serial.println(
            "[REPORT] ================================="
        );

    }


    void processReportSerial()
    {

        if(
            !Serial.available()
        )
        {

            return;

        }


        //--------------------------------------------------
        // Solo consume el byte si es una de nuestras teclas
        // ('a'-'g', 'h'); cualquier otra tecla (dígitos,
        // 's'/'S', 'm'/'M') se deja intacta para que
        // TestManager::processSerial() la procese en su
        // propio turno, dentro de testManager.execute().
        //--------------------------------------------------

        char peeked =
            (char) Serial.peek();


        if(
            peeked == 'h' ||
            peeked == 'H'
        )
        {

            Serial.read();

            printScenarioMenu();

            return;

        }


        if(
            peeked == 'r' ||
            peeked == 'R'
        )
        {

            Serial.read();

            toggleAutoScenario();

            return;

        }


        for(
            uint8_t i = 0;
            i < 7;
            i++
        )
        {

            if(
                peeked == SCENARIOS[i].key
            )
            {

                Serial.read();

                ReportLogger::setScenario(
                    SCENARIOS[i].id,
                    SCENARIOS[i].name
                );

                return;

            }

        }

    }


    //--------------------------------------------------
    // Log periódico de energía/comportamiento para la
    // gráfica de trazas de energía local/remota.
    //--------------------------------------------------

    uint32_t lastEnergyLogMillis =
        0;

    const uint32_t ENERGY_LOG_INTERVAL_MS =
        2000;

    //--------------------------------------------------
    // Ciclo automático de escenarios (opcional, tecla
    // 'r'/'R' para activar/desactivar). Aleatorio puro:
    // en cada ciclo se sortea uno de los escenarios
    // automatizables, pudiendo repetirse seguido.
    //
    // Solo cubre los 5 escenarios que TestManager puede
    // disparar por software (1, 4, 5, 6, 7). Los
    // escenarios 2 (nodo remoto apagado) y 3 (confirmación
    // demorada) dependen de una acción física real -- el
    // interruptor de energía del tercer equipo, o la
    // ausencia real de conexión -- y se siguen tageando a
    // mano con 'b'/'c' mientras se provocan.
    //--------------------------------------------------

    struct AutoScenarioEntry
    {
        TestScenario testScenario;
        uint8_t reportId;
        const char* reportName;
    };

    const AutoScenarioEntry AUTO_SCENARIOS[5] =
    {
        { TEST_NORMAL,        1, "NORMAL" },
        { TEST_FAIL_WAIT_ACK, 4, "CUT_WAIT_ACK" },
        { TEST_FAIL_SEND,     5, "CUT_BEFORE_SEND" },
        { TEST_FAIL_CREATE,   6, "CUT_AFTER_CREATE" },
        { TEST_FAIL_CONFIRM,  7, "CUT_RECEIVER_ACK_LOST" }
    };

    bool autoScenarioEnabled =
        false;

    uint32_t lastAutoScenarioMillis =
        0;

    const uint32_t AUTO_SCENARIO_INTERVAL_MS =
        20000;


    void toggleAutoScenario()
    {

        autoScenarioEnabled =
            !autoScenarioEnabled;

        Serial.print(
            "[REPORT] Ciclo automático de escenarios: "
        );

        Serial.println(
            autoScenarioEnabled ? "ACTIVADO" : "DESACTIVADO"
        );

        if(
            autoScenarioEnabled
        )
        {

            //--------------------------------------------------
            // Forzar el primer sorteo de inmediato en vez de
            // esperar el intervalo completo.
            //--------------------------------------------------

            lastAutoScenarioMillis =
                millis() - AUTO_SCENARIO_INTERVAL_MS;

        }

    }


    void runAutoScenarioIfDue()
    {

        if(
            !autoScenarioEnabled
        )
        {

            return;

        }


        uint32_t now =
            millis();

        if(
            now - lastAutoScenarioMillis < AUTO_SCENARIO_INTERVAL_MS
        )
        {

            return;

        }

        lastAutoScenarioMillis =
            now;


        uint8_t index =
            (uint8_t) random(0, 5);

        const AutoScenarioEntry& chosen =
            AUTO_SCENARIOS[index];


        Serial.println();

        Serial.print(
            "[REPORT] Auto-escenario sorteado: "
        );

        Serial.print(
            chosen.reportId
        );

        Serial.print(
            ". "
        );

        Serial.println(
            chosen.reportName
        );


        testManager.forceScenario(
            chosen.testScenario
        );

        ReportLogger::setScenario(
            chosen.reportId,
            chosen.reportName
        );

    }


    //--------------------------------------------------
    // Log periódico de energía/comportamiento para la
    // gráfica de trazas de energía local/remota.
    //--------------------------------------------------

    void logEnergyIfDue()
    {

        uint32_t now =
            millis();


        if(
            now - lastEnergyLogMillis < ENERGY_LOG_INTERVAL_MS
        )
        {

            return;

        }


        lastEnergyLogMillis =
            now;

        ReportLogger::event(
            "ENERGY",
            0,
            (long)(
                energyManager.getEnergy() * 100.0f
            )
        );

    }

}


//====================================================
// Setup
//====================================================

void setup()
{
    Serial.begin(
        115200
    );


    //--------------------------------------------------
    // Semilla del generador aleatorio (ciclo automático
    // de escenarios) con el RNG por hardware del ESP32,
    // no con millis() -- que en el arranque siempre
    // ronda el mismo valor.
    //--------------------------------------------------

    randomSeed(
        esp_random()
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
        "  PERSISTENT OS - REPORT BUILD"
    );

    Serial.println(
        "================================="
    );

    Serial.println();

    ReportLogger::begin();

    ReportLogger::enable();

    printScenarioMenu();

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


    resumeManager.begin(

        &bootManager.getState(),

        bootManager.wasRecovery()

    );


    sensorBuffer.attach(

        &bootManager
            .getState()
            .sensorBuffer

    );


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

    checkpointManager.attachState(

        &bootManager.getState()

    );

    checkpointManager.attachFRAM(
        &fram
    );

    failureManager.begin();

    energyManager.begin(
        100.0
    );
    energyPredictionManager.begin();
    behaviorManager.begin();

    sensorService.begin(

        &sensorManager,

        &sensorBuffer,

        &resumeManager,

        &checkpointManager,

        &failureManager,

        &bootManager.getState()
    );

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

    testManager.begin(

        &checkpointManager,

        &failureManager,

        &energyManager

    );


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
    // Clasificación de boot para el reporte: un boot en
    // frío marca BOOT_COLD; un boot de recuperación abre
    // la ventana de latencia que se cierra sola en el
    // primer evento CONFIRM (ver ReportLogger::event()).
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

        ReportLogger::beginRecoveryTracking();

    }
    else
    {

        ReportLogger::event(
            "BOOT_COLD",
            0,
            0
        );

    }

}


//====================================================
// Main loop
//====================================================

void loop()
{

    //--------------------------------------------------
    // 0. Report: tageo manual de escenario
    //--------------------------------------------------

    processReportSerial();

    runAutoScenarioIfDue();


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
    // 5. Failure tests (botón físico + menú de energía
    //    sintética de TestManager, teclas '1'-'4'/'s'/'m')
    //--------------------------------------------------

    testManager.execute();

    //--------------------------------------------------
    // 6. Energy prediction
    //--------------------------------------------------

    energyPredictionManager.update();

    //--------------------------------------------------
    // 7. Simulation
    //--------------------------------------------------

    simulationManager.execute();

    //--------------------------------------------------
    // 8. Energy
    //--------------------------------------------------

    energyManager.execute();

    energyPredictionManager.observe(
        energyManager.getEnergy()
    );

    energyPredictionManager.update();


    //--------------------------------------------------
    // 9. Report: traza periódica de energía
    //--------------------------------------------------

    logEnergyIfDue();

    delay(
        100
    );

}

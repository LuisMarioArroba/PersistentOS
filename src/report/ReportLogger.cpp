#include "report/ReportLogger.h"

bool ReportLogger::enabled = false;

bool ReportLogger::recoveryPending = false;

uint32_t ReportLogger::recoveryStartMillis = 0;

uint8_t ReportLogger::currentScenarioId = 0;

char ReportLogger::currentScenarioName[24] = "UNSET";


//====================================================
// Begin / enable
//
// Deshabilitado por defecto: sin llamar enable(), las
// llamadas a event() desde CommunicationService.cpp y
// CommunicationManager.cpp no imprimen nada, así que el
// build normal (persistentos1) no cambia su log. Solo
// main_prueba_reporte.cpp llama a begin()+enable().
//====================================================

void ReportLogger::begin()
{

    Serial.println(
        "RPT,millis,scenario_id,scenario_name,event,packetId,value"
    );

}

void ReportLogger::enable()
{

    enabled =
        true;

}

bool ReportLogger::isEnabled()
{

    return enabled;

}


//====================================================
// Set scenario
//====================================================

void ReportLogger::setScenario(
    uint8_t scenarioId,
    const char* scenarioName
)
{

    currentScenarioId =
        scenarioId;

    strncpy(
        currentScenarioName,
        scenarioName,
        sizeof(currentScenarioName) - 1
    );

    currentScenarioName[sizeof(currentScenarioName) - 1] =
        '\0';

    event(
        "SCENARIO_SET",
        0,
        0
    );

}


//====================================================
// Recovery tracking
//====================================================

void ReportLogger::beginRecoveryTracking()
{

    recoveryPending =
        true;

    recoveryStartMillis =
        millis();

    event(
        "BOOT_RECOVERY",
        0,
        0
    );

}


//====================================================
// Event
//====================================================

void ReportLogger::event(
    const char* eventName,
    uint32_t packetId,
    long value
)
{

    if(
        !enabled
    )
    {

        return;

    }


    printLine(
        eventName,
        packetId,
        value
    );


    //--------------------------------------------------
    // Cierre automático de la latencia de recovery: el
    // primer CONFIRM tras un boot de recuperación marca
    // el fin de la ventana medida desde beginRecoveryTracking().
    //--------------------------------------------------

    if(
        recoveryPending &&
        strcmp(eventName, "CONFIRM") == 0
    )
    {

        recoveryPending =
            false;

        long latency =
            (long)(
                millis() - recoveryStartMillis
            );

        printLine(
            "RECOVERY_LATENCY_MS",
            packetId,
            latency
        );

    }

}


//====================================================
// Print line (internal)
//====================================================

void ReportLogger::printLine(
    const char* eventName,
    uint32_t packetId,
    long value
)
{

    Serial.print(
        "RPT,"
    );

    Serial.print(
        millis()
    );

    Serial.print(
        ","
    );

    Serial.print(
        currentScenarioId
    );

    Serial.print(
        ","
    );

    Serial.print(
        currentScenarioName
    );

    Serial.print(
        ","
    );

    Serial.print(
        eventName
    );

    Serial.print(
        ","
    );

    Serial.print(
        packetId
    );

    Serial.print(
        ","
    );

    Serial.println(
        value
    );

}


//====================================================
// Get scenario
//====================================================

uint8_t ReportLogger::getScenario()
{

    return currentScenarioId;

}

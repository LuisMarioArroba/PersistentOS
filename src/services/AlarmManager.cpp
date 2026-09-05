#include "services/AlarmManager.h"

#include "services/CommunicationService.h"

#include "config/Config.h"


//====================================================
// Constructor
//====================================================

AlarmManager::AlarmManager()
{

    communicationService =
        nullptr;


    currentAlarm =
        ALARM_NONE;


    alarmActive =
        false;


    lastTemperature =
        0.0f;

}


//====================================================
// Begin
//====================================================

void AlarmManager::begin(
    CommunicationService*
        communicationServicePtr
)
{

    communicationService =
        communicationServicePtr;


    currentAlarm =
        ALARM_NONE;


    alarmActive =
        false;


    Serial.println(
        "[ALARM] Manager initialized"
    );

}


//====================================================
// Execute
//====================================================

void AlarmManager::execute(
    float temperature,

    bool sensorValid
)
{

    //--------------------------------------------------
    // Sin sensor conectado no hay nada que evaluar: un
    // valor inválido (p.ej. 0.0 por defecto) no debe
    // interpretarse como una temperatura real y disparar
    // una alarma falsa. El nodo sigue comunicando de
    // todas formas, solo no participa en las alarmas.
    //--------------------------------------------------

    if(
        !sensorValid
    )
    {

        if(
            alarmActive
        )
        {

            clearAlarm();

        }

        return;

    }


    lastTemperature =
        temperature;


    //--------------------------------------------------
    // Low temperature
    //--------------------------------------------------

    if(
        temperature <
        ALARM_TEMPERATURE_MIN
    )
    {

        if(
            currentAlarm !=
            ALARM_TEMPERATURE_LOW
        )
        {

            trigger(
                ALARM_TEMPERATURE_LOW
            );

        }

        return;

    }


    //--------------------------------------------------
    // High temperature
    //--------------------------------------------------

    if(
        temperature >
        ALARM_TEMPERATURE_MAX
    )
    {

        if(
            currentAlarm !=
            ALARM_TEMPERATURE_HIGH
        )
        {

            trigger(
                ALARM_TEMPERATURE_HIGH
            );

        }

        return;

    }


    //--------------------------------------------------
    // Normal
    //--------------------------------------------------

    if(
        alarmActive
    )
    {

        clearAlarm();

    }

}


//====================================================
// Trigger
//====================================================

void AlarmManager::trigger(
    AlarmType alarm
)
{

    currentAlarm =
        alarm;


    alarmActive =
        true;


    Serial.println();

    Serial.println(
        "[ALARM] ==============================="
    );

    Serial.println(
        "[ALARM] PRIORITY ALARM"
    );

    Serial.println(
        "[ALARM] ==============================="
    );


    switch(
        alarm
    )
    {

        case ALARM_TEMPERATURE_LOW:

            Serial.print(
                "[ALARM] Temperature LOW: "
            );

            Serial.print(
                lastTemperature
            );

            Serial.println(
                " C"
            );

            break;


        case ALARM_TEMPERATURE_HIGH:

            Serial.print(
                "[ALARM] Temperature HIGH: "
            );

            Serial.print(
                lastTemperature
            );

            Serial.println(
                " C"
            );

            break;


        default:

            Serial.println(
                "[ALARM] UNKNOWN"
            );

            break;

    }


    //--------------------------------------------------
    // Immediate communication
    //--------------------------------------------------

    sendAlarm();

}


//====================================================
// Send alarm
//====================================================

void AlarmManager::sendAlarm()
{

    if(
        communicationService ==
        nullptr
    )
    {

        return;

    }


    char message[64];

    bool isLow =
        false;


    if(
        currentAlarm ==
        ALARM_TEMPERATURE_LOW
    )
    {

        isLow =
            true;

        snprintf(
            message,
            sizeof(message),
            "NODE:%d|ALARM:TEMP_LOW:%.2f",
            (int) PERSISTENT_OS_NODE_ID,
            lastTemperature
        );

    }
    else
    {

        if(
            currentAlarm ==
            ALARM_TEMPERATURE_HIGH
        )
        {

            snprintf(
                message,
                sizeof(message),
                "NODE:%d|ALARM:TEMP_HIGH:%.2f",
                (int) PERSISTENT_OS_NODE_ID,
                lastTemperature
            );

        }
        else
        {

            return;

        }

    }


    //--------------------------------------------------
    // Mismo formato de mensaje que se imprime al recibir
    // una alarma ajena (CommunicationService::handleIncomingData),
    // para que se vea igual sin importar si la alarma es
    // propia o de otro nodo.
    //--------------------------------------------------

    Serial.println();

    Serial.println(
        "[ALARM] ==============================="
    );

    Serial.print(
        "[ALARM] Nodo "
    );

    Serial.print(
        (int) PERSISTENT_OS_NODE_ID
    );

    Serial.print(
        " presento la alarma de temperatura "
    );

    Serial.print(
        isLow ? "baja " : "alta "
    );

    Serial.print(
        lastTemperature
    );

    Serial.println(
        " C"
    );

    Serial.println(
        "[ALARM] ==============================="
    );


    if(
        communicationService != nullptr
    )
    {

        communicationService->requestAlarm(
            message,
            (uint16_t) strlen(message)
        );

    }

}


//====================================================
// Clear
//====================================================

void AlarmManager::clearAlarm()
{

    Serial.println(
        "[ALARM] Conditions returned to normal"
    );


    currentAlarm =
        ALARM_NONE;


    alarmActive =
        false;

}


//====================================================
// Getters
//====================================================

bool AlarmManager::isAlarmActive() const
{

    return alarmActive;

}


AlarmType AlarmManager::getAlarm() const
{

    return currentAlarm;

}


float AlarmManager::getTemperature() const
{

    return lastTemperature;

}
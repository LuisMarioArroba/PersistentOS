#include "services/AlarmManager.h"

#include "kernel/CommunicationManager.h"

#include "config/Config.h"


//====================================================
// Constructor
//====================================================

AlarmManager::AlarmManager()
{

    communicationManager =
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
    CommunicationManager*
        communicationPtr
)
{

    communicationManager =
        communicationPtr;


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
    float temperature
)
{

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
        communicationManager ==
        nullptr
    )
    {

        return;

    }


    char message[64];


    if(
        currentAlarm ==
        ALARM_TEMPERATURE_LOW
    )
    {

        snprintf(
            message,
            sizeof(message),
            "ALARM:TEMP_LOW:%.2f",
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
                "ALARM:TEMP_HIGH:%.2f",
                lastTemperature
            );

        }
        else
        {

            return;

        }

    }


    communicationManager->sendPriority(
        (
            const uint8_t*
        )message,
        strlen(message)
    );

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
#ifndef ALARM_MANAGER_H
#define ALARM_MANAGER_H


#include <Arduino.h>


class CommunicationService;


enum AlarmType
{

    ALARM_NONE = 0,

    ALARM_TEMPERATURE_LOW,

    ALARM_TEMPERATURE_HIGH,

    ALARM_ENERGY_CRITICAL,

    ALARM_SYSTEM_FAILURE,

    ALARM_TOTAL

};


class AlarmManager
{

private:

    //--------------------------------------------------
    // Las alarmas viajan por el mismo pipeline
    // checkpointeado DATA/ACK que la telemetría de
    // rutina (vía requestAlarm()), en vez de escribirse
    // directo al puerto: así también quedan protegidas
    // por reintento e idempotencia ante un corte de
    // energía a mitad de envío.
    //--------------------------------------------------

    CommunicationService*
        communicationService;


    AlarmType currentAlarm;


    bool alarmActive;

    float lastTemperature;


private:

    void trigger(
        AlarmType alarm
    );


    void clearAlarm();


    void sendAlarm();


public:

    AlarmManager();


    void begin(
        CommunicationService*
            communicationServicePtr
    );


    void execute(
        float temperature,

        bool sensorValid
    );


    bool isAlarmActive() const;


    AlarmType getAlarm() const;


    float getTemperature() const;

};


#endif
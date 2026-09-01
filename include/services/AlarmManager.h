#ifndef ALARM_MANAGER_H
#define ALARM_MANAGER_H


#include <Arduino.h>


class CommunicationManager;


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

    CommunicationManager*
        communicationManager;


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
        CommunicationManager*
            communicationPtr
    );


    void execute(
        float temperature
    );


    bool isAlarmActive() const;


    AlarmType getAlarm() const;


    float getTemperature() const;

};


#endif
#ifndef PERSISTENT_STATE_H
#define PERSISTENT_STATE_H
#define SENSOR_BUFFER_SIZE 32
#define ENERGY_HISTORY_SIZE 50
#include <Arduino.h>
#include "Config/Constants.h"

/*
Kernel persistent information
Persistent Task Information
Sensor Persistent Buffer
Energy history

Complete Persistent OS State
    Kernel execution state
    Task scheduler state
    Sensor acquisition history
    Historical energy information (Statidist base)
*/

struct PersistentKernelState{
    uint32_t systemTick;
    uint32_t bootCount;
    bool recovering;
};

struct PersistentTaskState{
    uint8_t id;
    uint8_t state;
    uint32_t lastExecution;
    uint32_t executions;
};

struct SensorSample{
    uint32_t timestamp;
    float value;
};

struct PersistentSensorBuffer{
    SensorSample samples[SENSOR_BUFFER_SIZE];
    uint8_t head;
    uint8_t count;
};

struct EnergyRecord{
    uint32_t timestamp;
    float voltage;
};

struct PersistentEnergyHistory{
    EnergyRecord records[ENERGY_HISTORY_SIZE];
    uint8_t head;
    uint8_t count;
};

struct PersistentState
{    PersistentKernelState kernel;
    PersistentTaskState tasks[MAX_TASKS];
    PersistentSensorBuffer sensorBuffer;
    PersistentEnergyHistory energyHistory;
};

#endif
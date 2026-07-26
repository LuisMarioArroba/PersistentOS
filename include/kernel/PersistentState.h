#ifndef PERSISTENT_STATE_H
#define PERSISTENT_STATE_H
#include <Arduino.h>
#include "Config/Constants.h"
#include "Config/Config.h"
#include "kernel/TaskID.h"

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
    bool persistentAvailable;
};

struct PersistentTaskState{
    TaskID id;
    uint8_t state;
    uint32_t lastExecution;
    uint32_t executions;
    uint8_t checkpoint;
    uint8_t progress;
    bool completed;
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
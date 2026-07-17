#ifndef SENSOR_BUFFER_H
#define SENSOR_BUFFER_H


#include <Arduino.h>

#include "kernel/PersistentState.h"



class SensorBuffer
{

private:

    PersistentSensorBuffer* buffer;


public:

    SensorBuffer();


    void attach(
        PersistentSensorBuffer* persistentBuffer
    );


    void push(
        float value,
        uint32_t timestamp
    );


    bool available();


    SensorSample get(
        uint8_t index
    );


    uint8_t getCount();

};


#endif
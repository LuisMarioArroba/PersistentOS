#ifndef SENSOR_SERVICE_H
#define SENSOR_SERVICE_H


#include <Arduino.h>

#include "hardware/Sensors.h"
#include "services/SensorBuffer.h"


class SensorService
{

private:

    Sensor* sensor;

    SensorBuffer* buffer;

    float simulatedValue;


public:

    SensorService();


    void begin(
        Sensor* sensorPtr,
        SensorBuffer* bufferPtr
    );


    void execute();


    void executeSimulation();


    float getLastValue() const;


    void printBufferStatus();

};


#endif
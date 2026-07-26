#ifndef SENSOR_SERVICE_H
#define SENSOR_SERVICE_H


#include <Arduino.h>


#include "services/ResumableService.h"

#include "hardware/Sensors.h"

#include "kernel/ExecutionSteps.h"

#include "services/SensorBuffer.h"



class SensorService : public ResumableService
{

private:

    Sensor* sensor;

    SensorBuffer* buffer;

    float simulatedValue;



private:

    void saveBuffer(
        float value
    );



public:

    SensorService();



    void begin(
        Sensor* sensorPtr,
        SensorBuffer* bufferPtr,
        ResumeManager* resumePtr,
        ExecutionCheckpoint* checkpointPtr
    );



    void executeSimulation();



    float getLastValue() const;



    void printBufferStatus();



protected:

    void executeNormal() override;


    void executeResume() override;

};


#endif
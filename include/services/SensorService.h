#ifndef SENSOR_SERVICE_H
#define SENSOR_SERVICE_H


#include <Arduino.h>


#include "services/ResumeableService.h"

#include "hardware/Sensors.h"

//#include "kernel/ExecutionSteps.h"

#include "services/SensorBuffer.h"

#include "services/FailureManager.h"



class SensorService : public ResumableService
{


private:

    Sensor* sensor;

    SensorBuffer* buffer;

    FailureManager* failureManager;

    float simulatedValue;

    PersistentState* persistentState;

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
        ExecutionCheckpoint* checkpointPtr,
        FailureManager* failurePtr,
        PersistentState* persistentStatePtr
    );



    void executeSimulation();


    float getLastValue() const;


    void printBufferStatus();



protected:

    void executeNormal() override;


    void executeResume() override;


};



#endif
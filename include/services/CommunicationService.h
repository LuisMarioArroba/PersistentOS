#ifndef COMMUNICATION_SERVICE_H
#define COMMUNICATION_SERVICE_H


#include <Arduino.h>


#include "services/ResumeableService.h"

#include "services/CommunicationBuffer.h"

#include "kernel/CommunicationPacket.h"

#include "kernel/CommunicationManager.h"

#include "kernel/ExecutionSteps.h"

#include "services/SensorService.h"

#include "services/FailureManager.h"



class CommunicationService :
    public ResumableService
{


private:


    CommunicationBuffer* buffer;

    FailureManager* failureManager;

    CommunicationManager* communicationManager;

    SensorService* sensorService;

    uint32_t packetCounter;



private:


    void createPacket();


    bool preparePacket(
        CommunicationPacket* packet
    );


    bool sendPacket(
        CommunicationPacket* packet
    );


    bool waitACK(
        CommunicationPacket* packet
    );


    void confirmPacket(
        CommunicationPacket* packet
    );



public:


    CommunicationService();



    void begin(

        CommunicationBuffer* bufferPtr,

        CommunicationManager* communicationPtr,

        ResumeManager* resumePtr,

        ExecutionCheckpoint* checkpointPtr,

        SensorService* sensorPtr,

        FailureManager* failurePtr

    );



protected:


    void executeNormal() override;


    void executeResume() override;



};



#endif
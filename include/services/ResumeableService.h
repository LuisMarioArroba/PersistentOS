#ifndef RESUMABLE_SERVICE_H
#define RESUMABLE_SERVICE_H


#include <Arduino.h>


#include "kernel/ResumeManager.h"
#include "kernel/ExecutionCheckpoint.h"
#include "kernel/TaskID.h"
#include "kernel/ExecutionSteps.h"



class ResumableService
{

protected:

    ResumeManager* resumeManager;

    ExecutionCheckpoint* checkpointManager;

    TaskID taskId;


    // Estado interno de pausa
    bool interrupted;



public:

    ResumableService();


    virtual ~ResumableService() = default;



    void begin(
        ResumeManager* resumePtr,
        ExecutionCheckpoint* checkpointPtr,
        TaskID id
    );



    void execute();



    // Recuperación manual sin FRAM
    void resumeAfterFailure();



protected:


    bool shouldResume() const;


    uint8_t getCheckpoint() const;


    uint8_t getProgress() const;



    void updateCheckpoint(
        uint8_t checkpoint,
        uint8_t progress
    );



    void finishExecution();



    void pauseExecution();



    virtual void executeNormal() = 0;


    virtual void executeResume() = 0;


};



#endif
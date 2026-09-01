#ifndef EXECUTION_CHECKPOINT_H
#define EXECUTION_CHECKPOINT_H

#include <Arduino.h>

#include "kernel/PersistentState.h"
#include "kernel/TaskID.h"
#include "kernel/FRAMManager.h"
#include "kernel/ExecutionSteps.h"


class ExecutionCheckpoint
{

private:

    PersistentState* state;

    FRAMManager* framManager;


public:

    ExecutionCheckpoint();


    void attachState(
        PersistentState* persistentState
    );


    void attachFRAM(
        FRAMManager* framPtr
    );


    void startTask(
        TaskID taskId
    );


    void update(
        TaskID taskId,
        uint8_t checkpoint,
        uint8_t progress
    );


    uint8_t getCheckpoint(
        TaskID taskId
    ) const;


    uint8_t getProgress(
        TaskID taskId
    ) const;

};

#endif
#ifndef EXECUTION_CHECKPOINT_H
#define EXECUTION_CHECKPOINT_H

#include <Arduino.h>

#include "kernel/PersistentState.h"
#include "kernel/TaskID.h"

class ExecutionCheckpoint
{

private:

    PersistentState* state;

public:

    ExecutionCheckpoint();

    void attachState(
        PersistentState* persistentState
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
        TaskID  taskId
    ) const;

    uint8_t getProgress(
        TaskID  taskId
    ) const;

};

#endif
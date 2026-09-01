#ifndef RESUME_MANAGER_H
#define RESUME_MANAGER_H

#include <Arduino.h>

#include "kernel/PersistentState.h"
#include "kernel/TaskID.h"

class ResumeManager
{

private:

    PersistentState* state;

    bool recoveryMode;

    bool resumed[MAX_TASKS];

    bool recoveryTask[MAX_TASKS];


public:

    ResumeManager();

    void begin(
        PersistentState* persistentState,
        bool recovery
    );

    bool shouldResume(
        TaskID taskId
    ) const;

    uint8_t getResumeCheckpoint(
        TaskID taskId
    ) const;

    uint8_t getProgress(
        TaskID taskId
    ) const;

    void markResumed(
        TaskID taskId
    );

    bool wasResumed(
        TaskID taskId
    ) const;

    void finishTask(
        TaskID taskId
    );

};

#endif
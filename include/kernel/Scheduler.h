#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <Arduino.h>

#include "kernel/Task.h"
#include "kernel/TaskManager.h"
#include "kernel/PersistentState.h"
#include "kernel/ExecutionCheckpoint.h"
#include "config/Constants.h"

extern uint32_t systemTick;

class Scheduler
{
private:
    TaskManager* taskManager;
    ExecutionCheckpoint* checkpointManager;
    PersistentState* persistentState;

public:
    Scheduler();
    void attachTaskManager(TaskManager* manager);
    void attachState(PersistentState* state);
    void attachCheckpoint(ExecutionCheckpoint* checkpoint);
    void execute();
};
#endif
#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "Task.h"
#include "PersistentState.h"
#include "Config/Constants.h"

extern uint32_t systemTick;

class Scheduler
{
private:
    Task* tasks[MAX_TASKS];
    int count;
    PersistentState* persistentState;

public:
    Scheduler();
    void addTask(Task* task);
    void attachState(
        PersistentState* state
    );
    void execute();
};


#endif
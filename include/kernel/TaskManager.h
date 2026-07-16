#ifndef TASK_MANAGER_H
#define TASK_MANAGER_H

#include "kernel/Task.h"
#include "config/Constants.h"

class TaskManager
{
private:

    Task* tasks[MAX_TASKS];

    uint8_t taskCount;

public:

    TaskManager();

    bool addTask(Task* task);

    Task* getTask(uint8_t index);

    uint8_t getTaskCount() const;

};

#endif
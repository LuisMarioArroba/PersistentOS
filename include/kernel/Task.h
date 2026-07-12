#ifndef TASK_H
#define TASK_H

#include <Arduino.h>
typedef void (*TaskFunction)();

enum TaskState
{
    READY = 0,
    RUNNING = 1,
    WAITING = 2,
    SUSPENDED = 3
};

struct Task
{
    const char* name;
    TaskFunction run;
    TaskState state;
    uint32_t period;
    uint32_t lastExecution;
    uint8_t id;
    uint32_t executions;
};

#endif
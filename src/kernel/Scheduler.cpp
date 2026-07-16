#include "kernel/Scheduler.h"
#include "kernel/TaskManager.h"

Scheduler::Scheduler()
{
    taskManager = nullptr;
    persistentState = nullptr;
}

void Scheduler::attachTaskManager(
    TaskManager* manager
)
{
    taskManager = manager;
}

void Scheduler::attachState(
    PersistentState* state
)
{
    persistentState = state;
}

void Scheduler::execute()
{
    if(taskManager == nullptr)
    {
        return;
    }

    for(uint8_t i = 0;
        i < taskManager->getTaskCount();
        i++)
    {
        Task* task = taskManager->getTask(i);

        if(task == nullptr)
        {
            continue;
        }

        if(task->state != READY)
        {
            continue;
        }

        if((systemTick - task->lastExecution)
            < task->period)
        {
            continue;
        }

        task->state = RUNNING;

        task->run();

        task->lastExecution = systemTick;

        task->executions++;

        if(persistentState != nullptr)
        {
            persistentState->tasks[i] = { static_cast<unsigned char> (i), static_cast<unsigned char> (task->state), task->lastExecution, task->executions };
        }

        task->state = READY;
    }
}
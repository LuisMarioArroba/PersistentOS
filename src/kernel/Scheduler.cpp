#include "kernel/Scheduler.h"
#include "kernel/TaskManager.h"
#include "kernel/ExecutionSteps.h"

Scheduler::Scheduler()
{
    taskManager = nullptr;
    persistentState = nullptr;
    checkpointManager = nullptr;
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

void Scheduler::attachCheckpoint(
    ExecutionCheckpoint* checkpoint
){
    checkpointManager = checkpoint;
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

        if(checkpointManager != nullptr)
        {
            checkpointManager->update(
                task->id,
                STEP_IDLE,
                0
            );
        }

        task->run();

        task->lastExecution = systemTick;

        task->executions++;

        if(persistentState != nullptr)
        {
            persistentState->tasks[task->id].id =
                task->id;

            persistentState->tasks[task->id].state =
                task->state;

            persistentState->tasks[task->id].lastExecution =
                task->lastExecution;

            persistentState->tasks[task->id].executions =
                task->executions;
        }

        task->state = READY;
    }
}
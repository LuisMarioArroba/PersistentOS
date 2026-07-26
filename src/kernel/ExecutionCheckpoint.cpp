#include "kernel/ExecutionCheckpoint.h"

ExecutionCheckpoint::ExecutionCheckpoint()
{
    state = nullptr;
}
void ExecutionCheckpoint::startTask(
    TaskID taskId
)
{

    if(state == nullptr)
    {
        return;
    }


    if(taskId >= MAX_TASKS)
    {
        return;
    }


    state->tasks[taskId].completed = false;

    state->tasks[taskId].progress = 0;

    state->tasks[taskId].checkpoint = 0;

}

void ExecutionCheckpoint::attachState(
    PersistentState* persistentState
)
{
    state = persistentState;
}

void ExecutionCheckpoint::update(
    TaskID taskId,
    uint8_t checkpoint,
    uint8_t progress
)
{
    if(state == nullptr)
    {
        return;
    }

    if(taskId >= MAX_TASKS)
    {
        return;
    }

    state->tasks[taskId].checkpoint = checkpoint;

    state->tasks[taskId].progress = progress;
}

uint8_t ExecutionCheckpoint::getCheckpoint(
    TaskID taskId
) const
{
    if(state == nullptr)
    {
        return 0;
    }

    if(taskId >= MAX_TASKS)
    {
        return 0;
    }

    return state->tasks[taskId].checkpoint;
}

uint8_t ExecutionCheckpoint::getProgress(
    TaskID taskId
) const
{
    if(state == nullptr)
    {
        return 0;
    }

    if(taskId >= MAX_TASKS)
    {
        return 0;
    }

    return state->tasks[taskId].progress;
}
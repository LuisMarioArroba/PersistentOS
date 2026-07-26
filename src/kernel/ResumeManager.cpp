#include "kernel/ResumeManager.h"
#include "kernel/ExecutionSteps.h"

ResumeManager::ResumeManager()
{
    state = nullptr;

    recoveryMode = false;

    for(uint8_t i = 0; i < MAX_TASKS; i++)
    {
        resumed[i] = false;
    }
}

void ResumeManager::begin(
    PersistentState* persistentState,
    bool recovery
)
{
    state = persistentState;

    recoveryMode = recovery;

    for(uint8_t i = 0; i < MAX_TASKS; i++)
    {
        resumed[i] = false;
    }
}

bool ResumeManager::shouldResume(
    TaskID taskId
) const
{

    if(state == nullptr)
    {
        return false;
    }


    if(!recoveryMode)
    {
        return false;
    }


    if(taskId >= MAX_TASKS)
    {
        return false;
    }


    if(resumed[taskId])
    {
        return false;
    }



    return
        state->tasks[taskId].completed == false &&
        state->tasks[taskId].progress > 0 &&
        state->tasks[taskId].progress < 100;

}

uint8_t ResumeManager::getResumeCheckpoint(
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

uint8_t ResumeManager::getProgress(
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

void ResumeManager::markResumed(
    TaskID taskId
)
{
    if(taskId >= MAX_TASKS)
    {
        return;
    }

    resumed[taskId] = true;
}

bool ResumeManager::wasResumed(
    TaskID taskId
) const
{
    if(taskId >= MAX_TASKS)
    {
        return false;
    }

    return resumed[taskId];
}

void ResumeManager::finishTask(
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



    state->tasks[taskId].checkpoint =
        STEP_COMPLETE;


    state->tasks[taskId].progress =
        100;


    state->tasks[taskId].completed =
        true;



    resumed[taskId] = false;

}
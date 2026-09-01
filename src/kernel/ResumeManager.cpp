#include "kernel/ResumeManager.h"
#include "kernel/ExecutionSteps.h"


//====================================================
// Constructor
//====================================================

ResumeManager::ResumeManager()
{
    state =
        nullptr;

    recoveryMode =
        false;

    for(uint8_t i = 0; i < MAX_TASKS; i++)
    {
        resumed[i] =
            false;
    }
}


//====================================================
// Begin
//====================================================

void ResumeManager::begin(
    PersistentState* persistentState,
    bool recovery
)
{
    state =
        persistentState;

    recoveryMode =
        recovery;


    for(
        uint8_t i = 0;
        i < MAX_TASKS;
        i++
    )
    {
        resumed[i] =
            false;

        recoveryTask[i] =
            false;
    }


    //--------------------------------------------------
    // Capture recovery state
    //--------------------------------------------------

    if(
        recoveryMode &&
        state != nullptr
    )
    {
        for(
            uint8_t i = 0;
            i < MAX_TASKS;
            i++
        )
        {
            recoveryTask[i] =
                state->tasks[i].completed == false &&
                state->tasks[i].progress > 0 &&
                state->tasks[i].progress < 100;
             if(
                i == TASK_SENSOR
            )
            {
                recoveryTask[i] =
                    false;
            }
        }
    }


    Serial.print(
        "[ResumeManager] Recovery mode: "
    );

    Serial.println(
        recoveryMode
            ? "ENABLED"
            : "DISABLED"
    );
}


//====================================================
// Should resume
//====================================================

bool ResumeManager::shouldResume(
    TaskID taskId
) const
{
    if(
        state == nullptr
    )
    {
        return false;
    }


    if(
        !recoveryMode
    )
    {
        return false;
    }


    if(
        taskId >= MAX_TASKS
    )
    {
        return false;
    }


    if(
        resumed[taskId]
    )
    {
        return false;
    }


    return
        recoveryTask[taskId] &&
        !state->tasks[taskId].completed;
}


//====================================================
// Get resume checkpoint
//====================================================

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


    return
        state->tasks[taskId].checkpoint;
}


//====================================================
// Get progress
//====================================================

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


    return
        state->tasks[taskId].progress;
}


//====================================================
// Mark resumed
//====================================================

void ResumeManager::markResumed(
    TaskID taskId
)
{
    if(
        taskId >= MAX_TASKS
    )
    {
        return;
    }


    resumed[taskId] =
        true;

    recoveryTask[taskId] =
        false;


    Serial.print(
        "[ResumeManager] Task "
    );

    Serial.print(
        taskId
    );

    Serial.println(
        " marked as resumed"
    );
}


//====================================================
// Was resumed
//====================================================

bool ResumeManager::wasResumed(
    TaskID taskId
) const
{
    if(taskId >= MAX_TASKS)
    {
        return false;
    }


    return
        resumed[taskId];
}


//====================================================
// Finish task
//====================================================

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


    resumed[taskId] =
        false;
}

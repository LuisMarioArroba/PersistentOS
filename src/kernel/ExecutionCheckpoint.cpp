#include "kernel/ExecutionCheckpoint.h"


//====================================================
// Constructor
//====================================================

ExecutionCheckpoint::ExecutionCheckpoint()
{
    state =
        nullptr;

    framManager =
        nullptr;
}


//====================================================
// Attach persistent state
//====================================================

void ExecutionCheckpoint::attachState(
    PersistentState* persistentState
)
{
    state =
        persistentState;
}


//====================================================
// Attach FRAM
//====================================================

void ExecutionCheckpoint::attachFRAM(
    FRAMManager* framPtr
)
{
    framManager =
        framPtr;
}


//====================================================
// Start task
//====================================================

void ExecutionCheckpoint::startTask(
    TaskID taskId
)
{
    if(
        state == nullptr
    )
    {
        return;
    }


    if(
        taskId >= MAX_TASKS
    )
    {
        return;
    }


    state->tasks[taskId].completed =
        false;

    state->tasks[taskId].progress =
        0;

    state->tasks[taskId].checkpoint =
        STEP_IDLE;


    //--------------------------------------------------
    // Persist
    //--------------------------------------------------

    if(
        framManager != nullptr &&
        framManager->isAvailable()
    )
    {
        framManager->save(
            *state
        );
    }
}


//====================================================
// Update checkpoint
//====================================================

void ExecutionCheckpoint::update(
    TaskID taskId,
    uint8_t checkpoint,
    uint8_t progress
)
{
    if(
        state == nullptr
    )
    {
        return;
    }


    if(
        taskId >= MAX_TASKS
    )
    {
        return;
    }


    //--------------------------------------------------
    // Update RAM state
    //--------------------------------------------------

    state->tasks[taskId].checkpoint =
        checkpoint;

    state->tasks[taskId].progress =
        progress;

    state->tasks[taskId].completed =
        checkpoint == STEP_COMPLETE;


    //--------------------------------------------------
    // Persist state
    //--------------------------------------------------

    if(
        framManager != nullptr &&
        framManager->isAvailable()
    )
    {
        bool saved =
            framManager->save(
                *state
            );


        if(!saved)
        {
            Serial.println(
                "[Checkpoint] FRAM save failed"
            );
        }
    }
}


//====================================================
// Get checkpoint
//====================================================

uint8_t ExecutionCheckpoint::getCheckpoint(
    TaskID taskId
) const
{
    if(
        state == nullptr
    )
    {
        return 0;
    }


    if(
        taskId >= MAX_TASKS
    )
    {
        return 0;
    }


    return
        state->tasks[taskId].checkpoint;
}


//====================================================
// Get progress
//====================================================

uint8_t ExecutionCheckpoint::getProgress(
    TaskID taskId
) const
{
    if(
        state == nullptr
    )
    {
        return 0;
    }


    if(
        taskId >= MAX_TASKS
    )
    {
        return 0;
    }


    return
        state->tasks[taskId].progress;
}
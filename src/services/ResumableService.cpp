#include "services/ResumeableService.h"


//====================================================
// Constructor
//====================================================

ResumableService::ResumableService()
{
    resumeManager =
        nullptr;

    checkpointManager =
        nullptr;

    failureManager =
        nullptr;

    taskId =
        TASK_NONE;

    interrupted =
        false;
}


//====================================================
// Begin
//====================================================

void ResumableService::begin(
    ResumeManager* resumePtr,
    ExecutionCheckpoint* checkpointPtr,
    TaskID id,
    FailureManager* failurePtr
)
{
    resumeManager =
        resumePtr;

    checkpointManager =
        checkpointPtr;

    taskId =
        id;

    failureManager =
        failurePtr;

    interrupted =
        false;
}


//====================================================
// Execute
//====================================================

void ResumableService::execute()
{
    //--------------------------------------------------
    // GLOBAL FAILURE CONTROL
    //--------------------------------------------------

    if(
        failureManager != nullptr &&
        failureManager->hasFailure()
    )
    {
        if(!interrupted)
        {
            Serial.print(
                "[Service] "
            );

            Serial.print(
                taskId
            );

            Serial.println(
                " paused by failure"
            );

            pauseExecution();
        }

        return;
    }


    //--------------------------------------------------
    // RECOVERY AFTER FAILURE
    //--------------------------------------------------

    if(interrupted)
    {
        Serial.print(
            "[Service] Recovery detected - Task "
        );

        Serial.println(
            taskId
        );

        resumeAfterFailure();
    }


    //--------------------------------------------------
    // RECOVERY AFTER REBOOT
    //
    // Only occurs when ResumeManager is in recovery
    // mode and a persistent incomplete checkpoint
    // exists.
    //--------------------------------------------------

    if(shouldResume())
    {
        uint8_t checkpoint =
            getCheckpoint();


        uint8_t progress =
            getProgress();


        Serial.print(
            "[Service] Resume execution - Task "
        );

        Serial.println(
            taskId
        );


        Serial.print(
            "[Service] Resume checkpoint: "
        );

        Serial.println(
            checkpoint
        );


        Serial.print(
            "[Service] Resume progress: "
        );

        Serial.print(
            progress
        );

        Serial.println(
            "%"
        );


        //--------------------------------------------------
        // Execute exactly from the persisted checkpoint
        //--------------------------------------------------

        executeResume();

/*
        //--------------------------------------------------
        // Prevent executing the boot recovery repeatedly
        //--------------------------------------------------

        if(
            resumeManager != nullptr
        )
        {
            resumeManager->markResumed(
                taskId
            );
        }
*/

        return;
    }


    //--------------------------------------------------
    // NORMAL EXECUTION
    //
    // IMPORTANT:
    //
    // executeNormal() is responsible for continuing
    // from the checkpoint currently stored.
    //--------------------------------------------------

    executeNormal();
}


//====================================================
// Recovery after failure
//====================================================

void ResumableService::resumeAfterFailure()
{
    interrupted =
        false;


    Serial.println(
        "[Service] Recovery enabled"
    );
}


//====================================================
// Pause
//====================================================

void ResumableService::pauseExecution()
{
    interrupted =
        true;


    Serial.println(
        "[Service] Execution paused"
    );
}


//====================================================
// Should resume
//====================================================

bool ResumableService::shouldResume() const
{
    if(
        resumeManager == nullptr
    )
    {
        return false;
    }


    return
        resumeManager->shouldResume(
            taskId
        );
}


//====================================================
// Get checkpoint
//====================================================

uint8_t ResumableService::getCheckpoint() const
{
    if(
        resumeManager == nullptr
    )
    {
        return 0;
    }


    return
        resumeManager->getResumeCheckpoint(
            taskId
        );
}


//====================================================
// Get progress
//====================================================

uint8_t ResumableService::getProgress() const
{
    if(
        resumeManager == nullptr
    )
    {
        return 0;
    }


    return
        resumeManager->getProgress(
            taskId
        );
}


//====================================================
// Update checkpoint
//====================================================

void ResumableService::updateCheckpoint(
    uint8_t checkpoint,
    uint8_t progress
)
{
    if(
        checkpointManager == nullptr
    )
    {
        return;
    }


    checkpointManager->update(
        taskId,
        checkpoint,
        progress
    );
}


//====================================================
// Finish execution
//====================================================

void ResumableService::finishExecution()
{
    //--------------------------------------------------
    // Complete persistent task state
    //--------------------------------------------------

    if(
        resumeManager != nullptr
    )
    {
        resumeManager->finishTask(
            taskId
        );
    }


    //--------------------------------------------------
    // Persist completion
    //--------------------------------------------------

    if(
        checkpointManager != nullptr
    )
    {
        checkpointManager->update(
            taskId,
            STEP_COMPLETE,
            100
        );
    }


    Serial.print(
        "[Service] Task "
    );

    Serial.print(
        taskId
    );

    Serial.println(
        " completed"
    );
}


//====================================================
// Is interrupted
//====================================================

bool ResumableService::isInterrupted() const
{
    return interrupted;
}
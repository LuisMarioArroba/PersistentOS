#include "services/ResumeableService.h"

ResumableService::ResumableService()
{

    resumeManager = nullptr;

    checkpointManager = nullptr;

    taskId = TASK_NONE;


    interrupted = false;

}





void ResumableService::begin(
    ResumeManager* resumePtr,
    ExecutionCheckpoint* checkpointPtr,
    TaskID id
)
{

    resumeManager = resumePtr;

    checkpointManager = checkpointPtr;

    taskId = id;

}





void ResumableService::execute()
{


    if(interrupted)
    {

        Serial.println(
            "[Service] Waiting recovery"
        );


        return;

    }



    if(shouldResume())
    {

        Serial.println(
            "[Service] Resume execution"
        );


        executeResume();



        if(resumeManager != nullptr)
        {

            resumeManager->markResumed(
                taskId
            );

        }


    }
    else
    {

        executeNormal();

    }


}





void ResumableService::resumeAfterFailure()
{

    interrupted = false;


}





void ResumableService::pauseExecution()
{

    interrupted = true;


}





bool ResumableService::shouldResume() const
{

    if(resumeManager == nullptr)
    {
        return false;
    }


    return resumeManager->shouldResume(
        taskId
    );

}





uint8_t ResumableService::getCheckpoint() const
{

    if(resumeManager == nullptr)
    {
        return 0;
    }


    return resumeManager->getResumeCheckpoint(
        taskId
    );

}





uint8_t ResumableService::getProgress() const
{

    if(resumeManager == nullptr)
    {
        return 0;
    }


    return resumeManager->getProgress(
        taskId
    );

}





void ResumableService::updateCheckpoint(
    uint8_t checkpoint,
    uint8_t progress
)
{

    if(checkpointManager == nullptr)
    {
        return;
    }


    checkpointManager->update(
        taskId,
        checkpoint,
        progress
    );

}





void ResumableService::finishExecution()
{

    if(resumeManager != nullptr)
    {

        resumeManager->finishTask(
            taskId
        );

    }



    if(checkpointManager != nullptr)
    {

        checkpointManager->update(
            taskId,
            STEP_COMPLETE,
            100
        );

    }


}
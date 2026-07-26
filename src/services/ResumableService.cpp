#include "services/ResumableService.h"


ResumableService::ResumableService()
{
    resumeManager = nullptr;

    checkpointManager = nullptr;

    taskId = TASK_SENSOR;
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

    if(shouldResume())
    {

        executeResume();

    }
    else
    {

        if(checkpointManager != nullptr)
        {
            checkpointManager->startTask(
                taskId
            );
        }


        executeNormal();

    }

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

    if(resumeManager == nullptr)
    {
        return;
    }


    resumeManager->finishTask(
        taskId
    );

}
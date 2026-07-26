#include "services/FailureManager.h"



FailureManager::FailureManager()
{

    failureRequested = false;

}




void FailureManager::begin()
{

    failureRequested = false;

}




void FailureManager::triggerFailure()
{

    failureRequested = true;


    Serial.println(
        "[FailureManager] Failure triggered"
    );

}




bool FailureManager::hasFailure() const
{

    return failureRequested;

}




void FailureManager::clear()
{

    failureRequested = false;


    Serial.println(
        "[FailureManager] Failure cleared"
    );

}
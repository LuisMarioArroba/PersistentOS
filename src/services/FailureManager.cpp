#include "services/FailureManager.h"


//====================================================
// Constructor
//====================================================

FailureManager::FailureManager()
{

    failureRequested =
        false;

}


//====================================================
// Begin
//====================================================

void FailureManager::begin()
{

    failureRequested =
        false;

}


//====================================================
// Trigger failure
//====================================================

void FailureManager::triggerFailure()
{

    failureRequested =
        true;

    Serial.println(
        "[FailureManager] Failure triggered"
    );

}


//====================================================
// Has failure
//====================================================

bool FailureManager::hasFailure() const
{

    return failureRequested;

}


//====================================================
// Clear
//====================================================

void FailureManager::clear()
{

    failureRequested =
        false;

    Serial.println(
        "[FailureManager] Failure cleared"
    );

}
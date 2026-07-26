#ifndef FAILURE_MANAGER_H
#define FAILURE_MANAGER_H


#include <Arduino.h>


class FailureManager
{

private:

    bool failureRequested;


public:

    FailureManager();


    void begin();


    void triggerFailure();


    bool hasFailure() const;


    void clear();


};


#endif
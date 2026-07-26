#include "kernel/BootManager.h"


BootManager::BootManager(
    FRAMManager& fram
)
:
framManager(fram)
{

    recoveryDetected = false;

}



bool BootManager::begin()
{

    if(!framManager.begin())
    {
        return false;
    }



    if(framManager.load(state))
    {

        recoveryDetected = true;


        state.kernel.recovering = true;


        state.kernel.bootCount++;


    }
    else
    {

        memset(
            &state,
            0,
            sizeof(PersistentState)
        );


        state.kernel.bootCount = 1;


        state.kernel.recovering = false;



        for(uint8_t i = 0; i < MAX_TASKS; i++)
        {

            state.tasks[i].completed = false;

        }

    }



    Serial.println(
        state.kernel.bootCount
    );



    state.kernel.recovering = false;



    framManager.save(
        state
    );



    return true;

}



bool BootManager::wasRecovery()
{

    return recoveryDetected;

}



PersistentState& BootManager::getState()
{

    return state;

}
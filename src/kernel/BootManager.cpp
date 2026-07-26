#include "kernel/BootManager.h"
#include "kernel/ExecutionSteps.h"



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

    if(!framManager.isAvailable())
    {
        Serial.println(
            "[BOOT] FRAM not detected"
        );

        Serial.println(
            "[BOOT] Running volatile mode"
        );
    }
    else
    {
        Serial.println(
            "[BOOT] Persistent memory available"
        );
    }


    //--------------------------------------------------
    // Recover previous state
    //--------------------------------------------------

    if(framManager.load(state))
    {

        recoveryDetected = true;


        state.kernel.recovering = true;


        state.kernel.bootCount++;

        state.kernel.persistentAvailable =framManager.isAvailable();

        Serial.println(
            "[BOOT] Previous state recovered"
        );

    }
    else
    {

        //--------------------------------------------------
        // First execution
        //--------------------------------------------------

        memset(
            &state,
            0,
            sizeof(PersistentState)
        );


        state.kernel.bootCount = 1;


        state.kernel.recovering = false;

        state.kernel.persistentAvailable = framManager.isAvailable();

        for(uint8_t i = 0; i < MAX_TASKS; i++)
        {

            state.tasks[i].id =
                static_cast<TaskID>(i);


            state.tasks[i].checkpoint =
                STEP_IDLE;


            state.tasks[i].progress = 0;


            state.tasks[i].completed = false;

        }



        Serial.println(
            "[BOOT] Fresh startup"
        );

    }



    Serial.print(
        "[BOOT] Count: "
    );


    Serial.println(
        state.kernel.bootCount
    );



    //--------------------------------------------------
    // Persist current state
    //--------------------------------------------------

    if(framManager.isAvailable())
    {
        framManager.save(state);
    }



    /*
       No limpiar:
       
       checkpoint
       progress
       completed

       porque ResumeManager depende
       de estos valores.
    */


    state.kernel.recovering = false;



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

bool BootManager::hasPersistentMemory()
{

    return framManager.isAvailable();

}
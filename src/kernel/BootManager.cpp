#include "kernel/BootManager.h"
#include "kernel/ExecutionSteps.h"


//====================================================
// Constructor
//====================================================

BootManager::BootManager(
    FRAMManager& fram
)
:
    framManager(fram)
{
    recoveryDetected =
        false;
}


//====================================================
// Begin
//====================================================

bool BootManager::begin()
{
    //--------------------------------------------------
    // Initialize FRAM
    //--------------------------------------------------

    if(
        !framManager.begin()
    )
    {
        return false;
    }


    //--------------------------------------------------
    // FRAM status
    //--------------------------------------------------

    if(
        !framManager.isAvailable()
    )
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
    // Try to recover previous state
    //--------------------------------------------------

    if(
        framManager.load(state)
    )
    {
        recoveryDetected =
            true;


        state.kernel.recovering =
            true;


        state.kernel.bootCount++;


        state.kernel.persistentAvailable =
            framManager.isAvailable();


        Serial.println(
            "[BOOT] Previous state recovered"
        );
    }
    else
    {
        //--------------------------------------------------
        // Fresh startup
        //--------------------------------------------------

        memset(
            &state,
            0,
            sizeof(PersistentState)
        );


        state.kernel.bootCount =
            1;


        state.kernel.recovering =
            false;


        state.kernel.persistentAvailable =
            framManager.isAvailable();


        for(
            uint8_t i = 0;
            i < MAX_TASKS;
            i++
        )
        {
            state.tasks[i].id =
                static_cast<TaskID>(i);


            state.tasks[i].checkpoint =
                STEP_IDLE;


            state.tasks[i].progress =
                0;


            state.tasks[i].completed =
                false;
        }


        Serial.println(
            "[BOOT] Fresh startup"
        );
    }


    //--------------------------------------------------
    // Boot information
    //--------------------------------------------------

    Serial.print(
        "[BOOT] Count: "
    );

    Serial.println(
        state.kernel.bootCount
    );


    Serial.print(
        "[BOOT] Recovery: "
    );

    Serial.println(
        recoveryDetected
            ? "YES"
            : "NO"
    );


    //--------------------------------------------------
    // Save current state
    //--------------------------------------------------

    if(
        framManager.isAvailable()
    )
    {
        state.kernel.recovering =
            false;

        framManager.save(
            state
        );
    }
    else
    {
        state.kernel.recovering =
            false;
    }


    //--------------------------------------------------
    // Boot recovery flag is only valid during boot
    //--------------------------------------------------

    state.kernel.recovering =
        false;


    return true;
}


//====================================================
// Was recovery
//====================================================

bool BootManager::wasRecovery()
{
    return recoveryDetected;
}


//====================================================
// Get state
//====================================================

PersistentState& BootManager::getState()
{
    return state;
}


//====================================================
// Persistent memory
//====================================================

bool BootManager::hasPersistentMemory()
{
    return
        framManager.isAvailable();
}
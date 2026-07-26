#include "kernel/FRAMManager.h"



/*
    Memoria simulada temporal.

    Será reemplazada por FRAM física
    cuando exista el hardware.
*/


static PersistentState persistentMemory;

static bool memoryAvailable = false;




FRAMManager::FRAMManager()
{

    initialized = false;

    hardwareAvailable = false;

    baseAddress = 0;

}





bool FRAMManager::begin()
{

    initialized = true;


    /*
       Actualmente no existe FRAM física.

       Cuando exista el hardware:

       hardwareAvailable =
            detectFRAM();

    */


    hardwareAvailable = false;



    if(hardwareAvailable)
    {

        Serial.println(
            "[FRAM] Hardware detected"
        );

    }
    else
    {

        Serial.println(
            "[FRAM] Not found - Using volatile mode"
        );

    }



    return true;

}





bool FRAMManager::isAvailable() const
{

    return hardwareAvailable;

}






bool FRAMManager::save(
    const PersistentState& state
)
{

    if(!initialized)
    {
        return false;
    }



    if(hardwareAvailable)
    {

        /*
            Aquí irá:

            writeMemory(
                baseAddress,
                (uint8_t*)&state,
                sizeof(PersistentState)
            );

        */

    }
    else
    {

        memcpy(
            &persistentMemory,
            &state,
            sizeof(PersistentState)
        );


        memoryAvailable = true;

    }



    return true;

}





bool FRAMManager::load(
    PersistentState& state
)
{

    if(!initialized)
    {
        return false;
    }



    if(hardwareAvailable)
    {

        /*
            Aquí irá:

            readMemory(
                baseAddress,
                (uint8_t*)&state,
                sizeof(PersistentState)
            );
        */


        return true;

    }
    else
    {

        if(!memoryAvailable)
        {
            return false;
        }


        memcpy(
            &state,
            &persistentMemory,
            sizeof(PersistentState)
        );


        return true;

    }

}





bool FRAMManager::clear()
{

    if(!initialized)
    {
        return false;
    }


    memset(
        &persistentMemory,
        0,
        sizeof(PersistentState)
    );


    memoryAvailable = false;


    return true;

}





bool FRAMManager::writeMemory(
    uint32_t address,
    const uint8_t* data,
    size_t size
)
{

    return true;

}





bool FRAMManager::readMemory(
    uint32_t address,
    uint8_t* data,
    size_t size
)
{

    return true;

}
#include "kernel/FRAMManager.h"


//====================================================
// Constructor
//====================================================

FRAMManager::FRAMManager()
{

    initialized =
        false;

    hardwareAvailable =
        false;

    baseAddress =
        0;

}


//====================================================
// Begin
//====================================================

bool FRAMManager::begin()
{

    //--------------------------------------------------
    // Initialize I2C
    //--------------------------------------------------

    Wire.begin(
        FRAM_SDA_PIN,
        FRAM_SCL_PIN
    );


    Wire.setClock(
        400000
    );


    //--------------------------------------------------
    // Detect FRAM
    //--------------------------------------------------

    hardwareAvailable =
        detectFRAM();


    initialized =
        true;


    //--------------------------------------------------
    // Result
    //--------------------------------------------------

    if(
        hardwareAvailable
    )
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


//====================================================
// Detect FRAM
//====================================================

bool FRAMManager::detectFRAM()
{

    Wire.beginTransmission(
        FRAM_I2C_ADDRESS
    );


    uint8_t error =
        Wire.endTransmission();


    return
        error == 0;

}


//====================================================
// Is available
//====================================================

bool FRAMManager::isAvailable() const
{

    return
        hardwareAvailable;

}


//====================================================
// Save
//====================================================

bool FRAMManager::save(
    const PersistentState& state
)
{

    if(
        !initialized
    )
    {

        return false;

    }


    if(
        !hardwareAvailable
    )
    {

        return false;

    }


    return writeMemory(

        baseAddress,

        reinterpret_cast<const uint8_t*>(&state),

        sizeof(PersistentState)

    );

}


//====================================================
// Load
//====================================================

bool FRAMManager::load(
    PersistentState& state
)
{

    if(
        !initialized
    )
    {

        return false;

    }


    if(
        !hardwareAvailable
    )
    {

        return false;

    }


    return readMemory(

        baseAddress,

        reinterpret_cast<uint8_t*>(&state),

        sizeof(PersistentState)

    );

}


//====================================================
// Clear
//====================================================

bool FRAMManager::clear()
{

    if(
        !initialized
    )
    {

        return false;

    }


    if(
        !hardwareAvailable
    )
    {

        return false;

    }


    uint8_t zero =
        0;


    for(
        size_t i = 0;
        i < sizeof(PersistentState);
        i++
    )
    {

        if(
            !writeMemory(
                baseAddress + i,
                &zero,
                1
            )
        )
        {

            return false;

        }

    }


    return true;

}


//====================================================
// Write memory
//====================================================

bool FRAMManager::writeMemory(
    uint32_t address,
    const uint8_t* data,
    size_t size
)
{

    if(
        data == nullptr ||
        size == 0
    )
    {

        return false;

    }


    //--------------------------------------------------
    // FRAM uses a 16-bit memory address
    //--------------------------------------------------

    while(
        size > 0
    )
    {

        size_t chunkSize =
            size;


        /*
         * Keep each I2C transaction small.
         *
         * 24 bytes of data leaves room for:
         * - 2 address bytes
         * - I2C overhead
         *
         * This also avoids depending on the
         * ESP32 Wire buffer size.
         */

        if(
            chunkSize > 24
        )
        {

            chunkSize =
                24;

        }


        Wire.beginTransmission(
            FRAM_I2C_ADDRESS
        );


        Wire.write(
            (uint8_t)(
                (address >> 8) &
                0xFF
            )
        );


        Wire.write(
            (uint8_t)(
                address &
                0xFF
            )
        );


        for(
            size_t i = 0;
            i < chunkSize;
            i++
        )
        {

            Wire.write(
                data[i]
            );

        }


        uint8_t error =
            Wire.endTransmission();


        if(
            error != 0
        )
        {

            return false;

        }


        address +=
            chunkSize;


        data +=
            chunkSize;


        size -=
            chunkSize;

    }


    return true;

}


//====================================================
// Read memory
//====================================================

bool FRAMManager::readMemory(
    uint32_t address,
    uint8_t* data,
    size_t size
)
{

    if(
        data == nullptr ||
        size == 0
    )
    {

        return false;

    }


    while(
        size > 0
    )
    {

        size_t chunkSize =
            size;


        if(
            chunkSize > 24
        )
        {

            chunkSize =
                24;

        }


        //--------------------------------------------------
        // Set FRAM memory address
        //--------------------------------------------------

        Wire.beginTransmission(
            FRAM_I2C_ADDRESS
        );


        Wire.write(
            (uint8_t)(
                (address >> 8) &
                0xFF
            )
        );


        Wire.write(
            (uint8_t)(
                address &
                0xFF
            )
        );


        uint8_t error =
            Wire.endTransmission(
                false
            );


        if(
            error != 0
        )
        {

            return false;

        }


        //--------------------------------------------------
        // Read data
        //--------------------------------------------------

        size_t received =
            Wire.requestFrom(
                FRAM_I2C_ADDRESS,
                chunkSize
            );


        if(
            received != chunkSize
        )
        {

            return false;

        }


        for(
            size_t i = 0;
            i < chunkSize;
            i++
        )
        {

            if(
                !Wire.available()
            )
            {

                return false;

            }


            data[i] =
                Wire.read();

        }


        address +=
            chunkSize;


        data +=
            chunkSize;


        size -=
            chunkSize;

    }


    return true;

}
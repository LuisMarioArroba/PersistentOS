#include "services/SensorService.h"


//====================================================
// Constructor
//====================================================

SensorService::SensorService()
{
    sensor =
        nullptr;

    buffer =
        nullptr;

    simulatedValue =
        25.0;

    persistentState =
    nullptr;
}


//====================================================
// Begin
//====================================================

void SensorService::begin(
    Sensor* sensorPtr,
    SensorBuffer* bufferPtr,
    ResumeManager* resumePtr,
    ExecutionCheckpoint* checkpointPtr,
    FailureManager* failurePtr,
    PersistentState* persistentStatePtr
)
{
    sensor =
        sensorPtr;

    buffer =
        bufferPtr;

    persistentState =
        persistentStatePtr;

    ResumableService::begin(
        resumePtr,
        checkpointPtr,
        TASK_SENSOR,
        failurePtr
    );
}


//====================================================
// Normal execution
//====================================================

void SensorService::executeNormal()
{
    if(
        sensor == nullptr
    )
    {
        return;
    }


    uint8_t checkpoint =
        getCheckpoint();


    //--------------------------------------------------
    // IDLE / COMPLETE
    //--------------------------------------------------

    if(
        checkpoint == STEP_IDLE ||
        checkpoint == STEP_COMPLETE
    )
    {
        updateCheckpoint(
            STEP_SENSOR_READ,
            25
        );

        return;
    }


    //--------------------------------------------------
    // READ
    //--------------------------------------------------

    if(
        checkpoint == STEP_SENSOR_READ
    )
    {
        if(
            !sensor->update()
        )
        {
            return;
        }


        updateCheckpoint(
            STEP_SENSOR_PROCESS,
            50
        );


        return;
    }


    //--------------------------------------------------
    // PROCESS
    //--------------------------------------------------

    if(
        checkpoint == STEP_SENSOR_PROCESS
    )
    {
        float value =
            sensor->getValue();

        if(
            persistentState != nullptr
        )
        {
            persistentState->lastSensorValue =
                value;

            persistentState->lastSensorTimestamp =
                millis();
        }

        Serial.print(
            "[Sensor] "
        );

        Serial.println(
            value
        );


        updateCheckpoint(
            STEP_SENSOR_BUFFER,
            75
        );


        return;
    }


    //--------------------------------------------------
    // BUFFER
    //--------------------------------------------------

    if(
        checkpoint == STEP_SENSOR_BUFFER
    )
    {
        float value =
            sensor->getValue();


        saveBuffer(
            value
        );


        finishExecution();


        return;
    }


    //--------------------------------------------------
    // Invalid
    //--------------------------------------------------

    Serial.print(
        "[Sensor] Invalid checkpoint: "
    );

    Serial.println(
        checkpoint
    );
}


//====================================================
// Resume execution
//====================================================

void SensorService::executeResume()
{
    if(
        sensor == nullptr
    )
    {
        return;
    }


    uint8_t checkpoint =
        getCheckpoint();


    Serial.print(
        "[Sensor Resume] Checkpoint: "
    );

    Serial.println(
        checkpoint
    );


    //--------------------------------------------------
    // READ
    //--------------------------------------------------

    if(
        checkpoint == STEP_SENSOR_READ
    )
    {
        if(
            !sensor->update()
        )
        {
            return;
        }


        updateCheckpoint(
            STEP_SENSOR_PROCESS,
            50
        );


        return;
    }


    //--------------------------------------------------
    // PROCESS
    //--------------------------------------------------

    if(
        checkpoint == STEP_SENSOR_PROCESS
    )
    {
        float value =
            sensor->getValue();


        Serial.print(
            "[Sensor Resume] "
        );

        Serial.println(
            value
        );

        if(
            persistentState != nullptr
        )
        {
            persistentState->lastSensorValue =
                value;

            persistentState->lastSensorTimestamp =
                millis();
        }

        updateCheckpoint(
            STEP_SENSOR_BUFFER,
            75
        );


        return;
    }


    //--------------------------------------------------
    // BUFFER
    //--------------------------------------------------

    if(
        checkpoint == STEP_SENSOR_BUFFER
    )
    {
        float value =
            sensor->getValue();


        saveBuffer(
            value
        );


        finishExecution();


        return;
    }


    //--------------------------------------------------
    // Invalid
    //--------------------------------------------------

    Serial.print(
        "[Sensor Resume] Invalid checkpoint: "
    );

    Serial.println(
        checkpoint
    );
}


//====================================================
// Save buffer
//====================================================

void SensorService::saveBuffer(
    float value
)
{
    if(
        buffer == nullptr
    )
    {
        return;
    }


    buffer->push(
        value,
        millis()
    );


    Serial.print(
        "[Sensor Buffer] Stored: "
    );

    Serial.println(
        value
    );
}


//====================================================
// Simulation
//====================================================

void SensorService::executeSimulation()
{
    simulatedValue +=
        0.25;


    if(
        simulatedValue > 30.0
    )
    {
        simulatedValue =
            25.0;
    }


    saveBuffer(
        simulatedValue
    );


    Serial.print(
        "[Simulation] "
    );

    Serial.println(
        simulatedValue
    );


    finishExecution();
}


//====================================================
// Last value
//====================================================

float SensorService::getLastValue() const
{
    if(
        sensor == nullptr
    )
    {
        return 0.0;
    }


    return sensor->getValue();
}


//====================================================
// Buffer status
//====================================================

void SensorService::printBufferStatus()
{
    if(
        buffer == nullptr
    )
    {
        Serial.println(
            "[Buffer] NOT ATTACHED"
        );

        return;
    }


    Serial.println();

    Serial.println(
        "========== Sensor Buffer =========="
    );


    Serial.print(
        "Samples Stored : "
    );

    Serial.println(
        buffer->getCount()
    );


    Serial.println(
        "==================================="
    );
}
#include "services/SensorService.h"



SensorService::SensorService()
{

    sensor = nullptr;

    buffer = nullptr;

    failureManager = nullptr;

    simulatedValue = 25.0;

}




void SensorService::begin(
    Sensor* sensorPtr,
    SensorBuffer* bufferPtr,
    ResumeManager* resumePtr,
    ExecutionCheckpoint* checkpointPtr,
    FailureManager* failurePtr
)
{

    sensor = sensorPtr;

    buffer = bufferPtr;

    failureManager = failurePtr;


    ResumableService::begin(
        resumePtr,
        checkpointPtr,
        TASK_SENSOR
    );

}



//====================================================
// Normal execution
//====================================================

void SensorService::executeNormal()
{

    if(sensor == nullptr)
    {
        return;
    }



    //--------------------------------------------------
    // STEP 1
    //--------------------------------------------------

    updateCheckpoint(
        STEP_SENSOR_READ,
        25
    );



    if(!sensor->update())
    {
        return;
    }



    //--------------------------------------------------
    // STEP 2
    //--------------------------------------------------

    updateCheckpoint(
        STEP_SENSOR_PROCESS,
        50
    );



    if(
        failureManager != nullptr &&
        failureManager->hasFailure()
    )
    {

        Serial.println(
            "[Sensor] Simulated interruption at 50%"
        );


        Serial.print(
            "[DEBUG] Checkpoint: "
        );


        Serial.println(
            getCheckpoint()
        );


        Serial.print(
            "[DEBUG] Progress: "
        );


        Serial.println(
            getProgress()
        );



        pauseExecution();



        failureManager->clear();



        return;

    }



    float value =
        sensor->getValue();




    //--------------------------------------------------
    // STEP 3
    //--------------------------------------------------

    saveBuffer(
        value
    );



    Serial.print(
        "[Sensor] "
    );


    Serial.println(
        value
    );



    //--------------------------------------------------
    // COMPLETE
    //--------------------------------------------------

    finishExecution();

}




//====================================================
// Resume execution
//====================================================

void SensorService::executeResume()
{

    if(sensor == nullptr)
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



    switch(checkpoint)
    {


        case STEP_SENSOR_READ:
        {

            if(!sensor->update())
            {
                return;
            }


        }



        case STEP_SENSOR_PROCESS:
        {

            updateCheckpoint(
                STEP_SENSOR_PROCESS,
                50
            );


            float value =
                sensor->getValue();



            saveBuffer(
                value
            );



            Serial.print(
                "[Resume Sensor] "
            );


            Serial.println(
                value
            );


            break;

        }



        case STEP_SENSOR_BUFFER:
        {

            float value =
                sensor->getValue();



            saveBuffer(
                value
            );


            break;

        }



        default:
        {

            Serial.println(
                "[Sensor Resume] Invalid checkpoint"
            );


            return;

        }

    }



    finishExecution();

}




//====================================================
// Buffer
//====================================================

void SensorService::saveBuffer(
    float value
)
{

    updateCheckpoint(
        STEP_SENSOR_BUFFER,
        75
    );



    if(buffer != nullptr)
    {

        buffer->push(
            value,
            millis()
        );

    }

}




//====================================================
// Simulation
//====================================================

void SensorService::executeSimulation()
{

    simulatedValue += 0.25;



    if(simulatedValue > 30.0)
    {
        simulatedValue = 25.0;
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




float SensorService::getLastValue() const
{

    if(sensor == nullptr)
    {
        return 0.0;
    }


    return sensor->getValue();

}




void SensorService::printBufferStatus()
{

    if(buffer == nullptr)
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
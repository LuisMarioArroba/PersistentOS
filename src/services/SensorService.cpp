#include "services/SensorService.h"



SensorService::SensorService()
{

    sensor = nullptr;

    buffer = nullptr;

    simulatedValue = 25.0;

}



void SensorService::begin(
    Sensor* sensorPtr,
    SensorBuffer* bufferPtr
)
{

    sensor = sensorPtr;

    buffer = bufferPtr;

}



void SensorService::execute()
{

    if(sensor == nullptr)
    {
        return;
    }


    if(sensor->update())
    {

        float value =
            sensor->getValue();


        if(buffer != nullptr)
        {

            buffer->push(
                value,
                millis()
            );

        }


        Serial.print(
            "[SensorService] "
        );


        Serial.println(value);

    }

}



void SensorService::executeSimulation()
{

    simulatedValue += 0.25;


    if(simulatedValue > 30)
    {
        simulatedValue = 25.0;
    }


    if(buffer != nullptr)
    {

        buffer->push(
            simulatedValue,
            millis()
        );

    }


    Serial.print(
        "[Simulation] "
    );


    Serial.println(
        simulatedValue
    );

}



float SensorService::getLastValue() const
{

    if(sensor != nullptr)
    {
        return sensor->getValue();
    }


    return simulatedValue;

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


    Serial.println(
        "===== Sensor Buffer ====="
    );


    Serial.print(
        "Samples: "
    );


    Serial.println(
        buffer->getCount()
    );


    Serial.println(
        "========================="
    );

}
#include "services/SensorBuffer.h"



SensorBuffer::SensorBuffer()
{
    buffer = nullptr;
}



void SensorBuffer::attach(
    PersistentSensorBuffer* persistentBuffer
)
{
    buffer = persistentBuffer;
}



void SensorBuffer::push(
    float value,
    uint32_t timestamp
)
{

    if(buffer == nullptr)
    {
        return;
    }


    buffer->samples[buffer->head].value =
        value;


    buffer->samples[buffer->head].timestamp =
        timestamp;



    buffer->head =
        (buffer->head + 1) % SENSOR_BUFFER_SIZE;



    if(buffer->count < SENSOR_BUFFER_SIZE)
    {
        buffer->count++;
    }

}



bool SensorBuffer::available()
{

    if(buffer == nullptr)
    {
        return false;
    }


    return buffer->count > 0;

}



SensorSample SensorBuffer::get(
    uint8_t index
)
{

    SensorSample empty =
    {
        0,
        0
    };


    if(buffer == nullptr)
    {
        return empty;
    }


    if(index >= buffer->count)
    {
        return empty;
    }


    return buffer->samples[index];

}

uint8_t SensorBuffer::getCount()
{

    if(buffer == nullptr)
    {
        return 0;
    }


    return buffer->count;

}
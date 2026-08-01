#include "services/CommunicationBuffer.h"



CommunicationBuffer::CommunicationBuffer()
{

    buffer = nullptr;

}





void CommunicationBuffer::attach(
    PersistentCommunicationBuffer* persistentBuffer
)
{

    buffer = persistentBuffer;


    if(buffer == nullptr)
    {
        return;
    }



    /*
        Inicialización solamente si
        el buffer está vacío.

        Esto permite recuperar
        información después de un reboot.
    */

    if(buffer->count == 0)
    {

        buffer->head = 0;

        buffer->tail = 0;

    }

}





bool CommunicationBuffer::push(
    const CommunicationPacket& packet
)
{

    if(buffer == nullptr)
    {
        return false;
    }



    if(full())
    {
        return false;
    }



    //--------------------------------------------------
    // Insert packet
    //--------------------------------------------------

    buffer->packets[
        buffer->head
    ] = packet;



    buffer->head++;



    if(
        buffer->head >= COMMUNICATION_BUFFER_SIZE
    )
    {

        buffer->head = 0;

    }



    buffer->count++;



    return true;

}







bool CommunicationBuffer::pop()
{

    if(buffer == nullptr)
    {
        return false;
    }



    if(empty())
    {
        return false;
    }



    //--------------------------------------------------
    // Remove oldest packet
    //--------------------------------------------------

    buffer->packets[
        buffer->tail
    ].status = PACKET_DISCARDED;



    buffer->tail++;



    if(
        buffer->tail >= COMMUNICATION_BUFFER_SIZE
    )
    {

        buffer->tail = 0;

    }



    buffer->count--;



    return true;

}








CommunicationPacket* CommunicationBuffer::front()
{

    if(buffer == nullptr)
    {
        return nullptr;
    }



    if(empty())
    {
        return nullptr;
    }



    return
        &buffer->packets[
            buffer->tail
        ];

}








bool CommunicationBuffer::empty() const
{

    if(buffer == nullptr)
    {
        return true;
    }



    return buffer->count == 0;

}








bool CommunicationBuffer::full() const
{

    if(buffer == nullptr)
    {
        return true;
    }



    return
        buffer->count >= COMMUNICATION_BUFFER_SIZE;

}








uint8_t CommunicationBuffer::getCount() const
{

    if(buffer == nullptr)
    {
        return 0;
    }



    return buffer->count;

}








uint8_t CommunicationBuffer::capacity() const
{

    return COMMUNICATION_BUFFER_SIZE;

}








void CommunicationBuffer::clear()
{

    if(buffer == nullptr)
    {
        return;
    }



    for(
        uint8_t i = 0;
        i < COMMUNICATION_BUFFER_SIZE;
        i++
    )
    {

        buffer->packets[i].status =
            PACKET_EMPTY;

    }



    buffer->head = 0;

    buffer->tail = 0;

    buffer->count = 0;

}
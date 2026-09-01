#ifndef COMMUNICATION_BUFFER_H
#define COMMUNICATION_BUFFER_H

#include <Arduino.h>

#include "kernel/PersistentState.h"
#include "kernel/CommunicationPacket.h"


class CommunicationBuffer
{

private:

    PersistentCommunicationBuffer* buffer;


public:

    CommunicationBuffer();


    void attach(
        PersistentCommunicationBuffer* persistentBuffer
    );


    //--------------------------------------------------
    // Queue operations
    //--------------------------------------------------

    bool push(
        const CommunicationPacket& packet
    );


    bool pushPriority(
        const CommunicationPacket& packet
    );


    bool pop();


    CommunicationPacket* front();


    //--------------------------------------------------
    // Buffer information
    //--------------------------------------------------

    bool empty() const;

    bool full() const;

    uint8_t getCount() const;

    uint8_t capacity() const;


    //--------------------------------------------------
    // Packet ID
    //--------------------------------------------------

    uint32_t nextPacketID();


    //--------------------------------------------------
    // Maintenance
    //--------------------------------------------------

    void clear();

};

#endif
#ifndef COMMUNICATION_BUFFER_H
#define COMMUNICATION_BUFFER_H

#include <Arduino.h>

#include "kernel/PersistentState.h"
#include "kernel/CommunicationPacket.h"

/*
======================================================
Communication Buffer

Buffer circular persistente para almacenar paquetes
de comunicación.

Los paquetes permanecen en memoria hasta recibir
confirmación (ACK) o ser descartados.

======================================================
*/

class CommunicationBuffer
{

private:

    PersistentCommunicationBuffer* buffer;

public:

    CommunicationBuffer();

    //--------------------------------------------------
    // Attach persistent memory
    //--------------------------------------------------

    void attach(
        PersistentCommunicationBuffer* persistentBuffer
    );

    //--------------------------------------------------
    // Queue operations
    //--------------------------------------------------

    bool push(
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
    // Maintenance
    //--------------------------------------------------

    void clear();

};

#endif
#ifndef COMMUNICATION_PACKET_H
#define COMMUNICATION_PACKET_H


#include <Arduino.h>


#include "kernel/PacketStatus.h"
#include "kernel/AckStatus.h"

#include "Config/Constants.h"


//--------------------------------------------------
// Tipo de mensaje sobre el mismo protocolo DATA/ACK.
// Un paquete ALARM se crea y envía exactamente igual
// que uno DATA (mismo checkpoint, mismo ACK), solo
// cambia el prefijo de línea en el cable ("ALARM|" en
// vez de "DATA|") y que CommunicationService lo crea
// antes que cualquier telemetría de rutina pendiente.
//--------------------------------------------------

enum PacketKind : uint8_t
{
    PACKET_KIND_DATA = 0,

    PACKET_KIND_ALARM
};



struct CommunicationPacket
{


    //--------------------------------------------------
    // Identification
    //--------------------------------------------------

    uint32_t packetID;


    uint32_t timestamp;



    //--------------------------------------------------
    // Payload
    //--------------------------------------------------

    uint16_t length;


    uint8_t payload[MAX_PACKET_SIZE];



    //--------------------------------------------------
    // Communication state
    //--------------------------------------------------

    PacketStatus status;


    AckStatus ackStatus;



    //--------------------------------------------------
    // Recovery information
    //--------------------------------------------------

    uint8_t retries;


    uint16_t bytesSent;



    //--------------------------------------------------
    // Protocol information
    //--------------------------------------------------

    uint8_t protocol;



    //--------------------------------------------------
    // Security
    //--------------------------------------------------

    bool encrypted;



    //--------------------------------------------------
    // Execution checkpoint
    //--------------------------------------------------

    uint8_t checkpoint;


    //--------------------------------------------------
    // Message kind (DATA / ALARM)
    //--------------------------------------------------

    PacketKind kind;


};



#endif
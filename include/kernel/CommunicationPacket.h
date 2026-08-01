#ifndef COMMUNICATION_PACKET_H
#define COMMUNICATION_PACKET_H


#include <Arduino.h>


#include "kernel/PacketStatus.h"
#include "kernel/AckStatus.h"

#include "Config/Constants.h"



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


};



#endif
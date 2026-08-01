#ifndef PACKET_STATUS_H
#define PACKET_STATUS_H

#include <Arduino.h>

/*
======================================================
Communication Packet States

Cada paquete pasa por estos estados durante
su ciclo de vida.

Estos estados son persistentes para que,
ante una pérdida de energía, el sistema
pueda determinar exactamente dónde quedó
la transmisión.

======================================================
*/

enum PacketStatus : uint8_t
{
    //--------------------------------------------------
    // Packet created but not processed
    //--------------------------------------------------
    PACKET_EMPTY = 0,

    //--------------------------------------------------
    // Waiting to be transmitted
    //--------------------------------------------------
    PACKET_PENDING,

    //--------------------------------------------------
    // Packet selected by CommunicationService
    //--------------------------------------------------
    PACKET_PREPARING,

    //--------------------------------------------------
    // Transmission in progress
    //--------------------------------------------------
    PACKET_SENDING,

    //--------------------------------------------------
    // Waiting for ACK
    //--------------------------------------------------
    PACKET_WAITING_ACK,

    //--------------------------------------------------
    // Transmission completed successfully
    //--------------------------------------------------
    PACKET_SENT,

    //--------------------------------------------------
    // Transmission failed
    //--------------------------------------------------
    PACKET_FAILED,

    //--------------------------------------------------
    // Packet discarded
    //--------------------------------------------------
    PACKET_DISCARDED,

    //--------------------------------------------------
    // Packet ready
    //--------------------------------------------------
    PACKET_READY,

    //--------------------------------------------------
    // Packet confirm
    //--------------------------------------------------
    PACKET_CONFIRMED
};

#endif
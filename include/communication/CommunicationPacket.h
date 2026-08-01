#ifndef COMMUNICATION_PACKET_H
#define COMMUNICATION_PACKET_H

#include <Arduino.h>

struct CommunicationPacket
{
    uint32_t timestamp;

    uint16_t sequence;

    float sensorValue;

    uint16_t checksum;
};

#endif
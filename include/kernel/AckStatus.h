#ifndef ACK_STATUS_H
#define ACK_STATUS_H

#include <Arduino.h>


enum AckStatus : uint8_t
{

    ACK_NONE = 0,

    ACK_WAITING,

    ACK_RECEIVED,

    ACK_FAILED

};


#endif
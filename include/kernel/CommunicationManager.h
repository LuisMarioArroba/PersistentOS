#ifndef COMMUNICATION_MANAGER_H
#define COMMUNICATION_MANAGER_H

#include <Arduino.h>

#include "kernel/CommunicationProtocol.h"

class CommunicationManager
{

public:

    CommunicationManager();

    bool begin();

    bool send(
        const uint8_t* data,
        size_t length
    );

    bool isConnected() const;

    CommunicationProtocol getProtocol() const;

private:

    CommunicationProtocol currentProtocol;

private:

    bool initializeSerial();

    bool initializeBluetooth();

    bool initializeWiFi();

    bool sendSerial(
        const uint8_t* data,
        size_t length
    );

    bool sendBluetooth(
        const uint8_t* data,
        size_t length
    );

    bool sendWiFi(
        const uint8_t* data,
        size_t length
    );

};

#endif
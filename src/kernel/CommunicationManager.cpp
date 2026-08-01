#include "kernel/CommunicationManager.h"

#include "config/Config.h"

CommunicationManager::CommunicationManager()
{
    currentProtocol = COMM_NONE;
}

bool CommunicationManager::begin()
{

#if USE_BLUETOOTH_COMMUNICATION

    if(initializeBluetooth())
    {

        currentProtocol = COMM_BLUETOOTH;

        Serial.println(
            "[COMM] Bluetooth selected"
        );

        return true;

    }

#endif



#if USE_WIFI_COMMUNICATION

    if(initializeWiFi())
    {

        currentProtocol = COMM_WIFI;

        Serial.println(
            "[COMM] WiFi selected"
        );

        return true;

    }

#endif



#if USE_SERIAL_COMMUNICATION

    if(initializeSerial())
    {

        currentProtocol = COMM_SERIAL;

        Serial.println(
            "[COMM] Serial selected"
        );

        return true;

    }

#endif



    currentProtocol = COMM_NONE;


    Serial.println(
        "[COMM] No communication backend"
    );


    return false;

}



CommunicationProtocol
CommunicationManager::getProtocol() const
{
    return currentProtocol;
}



bool CommunicationManager::isConnected() const
{
    return currentProtocol != COMM_NONE;
}



bool CommunicationManager::send(
    const uint8_t* data,
    size_t length
)
{

    switch(currentProtocol)
    {

        case COMM_SERIAL:

            return sendSerial(
                data,
                length
            );



        case COMM_BLUETOOTH:

            return sendBluetooth(
                data,
                length
            );



        case COMM_WIFI:

            return sendWiFi(
                data,
                length
            );



        default:

            return false;

    }

}





//====================================================
// Initialization
//====================================================

bool CommunicationManager::initializeSerial()
{
    /*
        Serial ya fue inicializado
        desde main.cpp

        No se requiere ninguna
        configuración adicional.
    */

    return true;
}



bool CommunicationManager::initializeBluetooth()
{

    /*
        Futuro:

        BluetoothSerial.begin(...);

    */

    return false;

}



bool CommunicationManager::initializeWiFi()
{

    /*
        Futuro:

        WiFi.begin(...);

    */

    return false;

}





//====================================================
// Send methods
//====================================================

bool CommunicationManager::sendSerial(
    const uint8_t* data,
    size_t length
)
{

    Serial.print(
        "[COMM SERIAL] "
    );

    for(size_t i = 0; i < length; i++)
    {

        Serial.write(
            data[i]
        );

    }

    Serial.println();

    return true;

}



bool CommunicationManager::sendBluetooth(
    const uint8_t* data,
    size_t length
)
{

    /*
        Futuro:

        BluetoothSerial.write(...)
    */

    return false;

}



bool CommunicationManager::sendWiFi(
    const uint8_t* data,
    size_t length
)
{

    /*
        Futuro:

        MQTT

        HTTP

        TCP

        UDP
    */

    return false;

}
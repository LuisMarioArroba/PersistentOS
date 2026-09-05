#ifndef COMMUNICATION_MANAGER_H
#define COMMUNICATION_MANAGER_H

#include <Arduino.h>

#include "kernel/CommunicationProtocol.h"
#include "config/Config.h"
#include "Config/Constants.h"

#if USE_BLUETOOTH_COMMUNICATION
#include <BluetoothSerial.h>
#endif

class CommunicationManager
{
private:

    bool bluetoothConnected;

    //--------------------------------------------------
    // Último intento de reconexión activa tras una caída
    // del enlace (no el intento inicial de begin()/connect()
    // del arranque, que es bloqueante y ya garantiza la
    // primera conexión).
    //--------------------------------------------------

    uint32_t lastReconnectAttempt;


    //--------------------------------------------------
    // Rotación al peer secundario opcional (Config.h:
    // BLUETOOTH_SECONDARY_DEVICE_NAME). Deshabilitada
    // por completo si ese nombre está vacío: en ese caso
    // no se toca nada del comportamiento existente hacia
    // el primary.
    //--------------------------------------------------

    enum class PeerSlot : uint8_t
    {
        PRIMARY = 0,

        SECONDARY
    };

    PeerSlot activePeerSlot;

    uint32_t peerSlotStartTime;

private:

    CommunicationProtocol currentProtocol;


    //--------------------------------------------------
    // Line reassembly buffer
    //
    // Bluetooth SPP es un stream: los mensajes pueden
    // llegar partidos en varias llamadas o varios
    // mensajes pueden llegar juntos en una sola.
    // Este buffer acumula bytes hasta encontrar el
    // delimitador '\n' de un mensaje completo.
    //--------------------------------------------------

    char rxLineBuffer[MAX_LINE_SIZE];

    size_t rxLineLength;


    //--------------------------------------------------
    // Bluetooth
    //--------------------------------------------------
/*
    #if USE_BLUETOOTH_COMMUNICATION

    BluetoothSerial bluetoothSerial;

    #endif
*/

    //--------------------------------------------------
    // Initialization
    //--------------------------------------------------

    bool initializeSerial();

    bool initializeBluetooth();

    bool initializeWiFi();


    //--------------------------------------------------
    // Reconexión activa (post-arranque)
    //--------------------------------------------------

    void attemptReconnectBluetooth();


    //--------------------------------------------------
    // Rotación al peer secundario opcional. Se llama en
    // cada tick desde updateConnectionState(); es un
    // no-op si BLUETOOTH_SECONDARY_DEVICE_NAME está
    // vacío.
    //--------------------------------------------------

    void pollPeerRotation();


    //--------------------------------------------------
    // Send
    //--------------------------------------------------

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


    //--------------------------------------------------
    // Receive
    //--------------------------------------------------

    bool receiveBluetooth(
        uint8_t* buffer,
        size_t maxLength
    );

    bool receiveWiFi(
        uint8_t* buffer,
        size_t maxLength
    );


public:

    CommunicationManager();

    void updateConnectionState();

    //--------------------------------------------------
    // Initialization
    //--------------------------------------------------

    bool begin();


    //--------------------------------------------------
    // Protocol
    //--------------------------------------------------

    CommunicationProtocol getProtocol() const;


    bool isConnected() const;
    void printConnectionStatus();


    //--------------------------------------------------
    // Communication
    //--------------------------------------------------

    bool send(
        const uint8_t* data,
        size_t length
    );


    bool available();


    size_t receive(
        uint8_t* buffer,
        size_t maxLength
    );


    //--------------------------------------------------
    // Line-based receive (protocolo DATA/ACK)
    //
    // Extrae UN mensaje completo delimitado por '\n'
    // (sin el '\n') si ya está disponible en el buffer
    // interno. Devuelve false si todavía no hay un
    // mensaje completo. Debe llamarse repetidamente
    // (en loop) para drenar varios mensajes pendientes.
    //--------------------------------------------------

    bool receiveLine(
        char* outBuffer,
        size_t maxLength
    );

    bool sendPriority(
        const uint8_t* data,
        size_t length
    );

};

#endif
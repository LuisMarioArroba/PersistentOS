/*
======================================================
PersistentOS2 - Nodo emisor/receptor

Este firmware es intencionalmente independiente del
resto de la arquitectura de PersistentOS (Scheduler,
persistencia, checkpoints, etc.). Su objetivo es
probar la comunicación bidireccional DATA/ACK con
PersistentOS1 lo antes posible (ver sección 13 del
contexto del proyecto: "No necesita implementar
inicialmente toda la arquitectura PersistentOS1").

Reutiliza únicamente Config.h/Constants.h para que el
nombre Bluetooth (PersistentOS{N}) y el tamaño máximo
de línea sigan la misma convención que el nodo
principal.

Responsabilidades como RECEPTOR (de PersistentOS1):
  1. Reensamblar mensajes delimitados por '\n' desde el
     stream SPP (igual que CommunicationManager en el
     nodo principal).
  2. Reconocer mensajes DATA|packetID|payload.
  3. Detectar packetID ya procesado (idempotencia) para
     no duplicar efectos si PersistentOS1 retransmite.
  4. Responder siempre ACK|packetID.

Responsabilidades como EMISOR (hacia PersistentOS1):
  5. Generar y enviar periódicamente su propio
     DATA|packetID|payload (placeholder hasta contar
     con su propio sensor/monitor de energía).
  6. Reconocer el ACK|packetID de PersistentOS1 y
     reintentar el mismo paquete si no llega a tiempo.
======================================================
*/

#include <Arduino.h>
#include <BluetoothSerial.h>
#include <string.h>
#include <stdlib.h>

#include "config/Config.h"
#include "Config/Constants.h"


#if !USE_BLUETOOTH_COMMUNICATION
#error "PersistentOS2 requiere USE_BLUETOOTH_COMMUNICATION habilitado en Config.h"
#endif


namespace
{

BluetoothSerial bluetoothSerial;


//--------------------------------------------------
// Line reassembly buffer (stream SPP)
//--------------------------------------------------

char lineBuffer[MAX_LINE_SIZE];

size_t lineLength = 0;


//--------------------------------------------------
// Idempotency: historial circular de packetID
// ya procesados.
//--------------------------------------------------

constexpr uint8_t PROCESSED_ID_HISTORY = 8;

uint32_t processedIds[PROCESSED_ID_HISTORY];

uint8_t processedCount = 0;

uint8_t processedNextSlot = 0;


bool wasAlreadyProcessed(
    uint32_t packetId
)
{
    for(
        uint8_t i = 0;
        i < processedCount;
        i++
    )
    {
        if(
            processedIds[i] == packetId
        )
        {
            return true;
        }
    }

    return false;
}


void rememberProcessed(
    uint32_t packetId
)
{
    processedIds[processedNextSlot] =
        packetId;

    processedNextSlot =
        (processedNextSlot + 1) % PROCESSED_ID_HISTORY;

    if(
        processedCount < PROCESSED_ID_HISTORY
    )
    {
        processedCount++;
    }
}


//--------------------------------------------------
// Envío propio (PersistentOS2 -> PersistentOS1).
//
// Genera y envía periódicamente su propio DATA, y
// reintenta el mismo paquete si no llega ACK a tiempo.
// El payload es un placeholder hasta que este nodo
// tenga su propio sensor/monitor de energía.
//--------------------------------------------------

enum class OutgoingState : uint8_t
{
    IDLE,
    WAITING_ACK
};

OutgoingState outgoingState = OutgoingState::IDLE;

uint32_t outgoingPacketId = 0;

uint32_t lastSendTime = 0;

uint32_t ackWaitStart = 0;


void sendOutgoingData()
{
    //--------------------------------------------------
    // TODO: sustituir por una lectura real de energía
    // cuando este nodo tenga su propio hardware de
    // monitoreo (PowerMonitor equivalente al de
    // PersistentOS1).
    //--------------------------------------------------

    char payload[32];

    snprintf(
        payload,
        sizeof(payload),
        "ENERGY_TEST: %lu",
        (unsigned long) millis()
    );


    char frame[MAX_LINE_SIZE];

    int written =
        snprintf(
            frame,
            sizeof(frame),
            "DATA|%lu|%s\n",
            (unsigned long) outgoingPacketId,
            payload
        );

    if(
        written <= 0 ||
        (size_t) written >= sizeof(frame)
    )
    {
        Serial.println(
            "[BT] Failed to build outgoing DATA frame"
        );

        return;
    }


    Serial.print(
        "[BT] Sending own DATA, packet ID: "
    );

    Serial.println(
        outgoingPacketId
    );

    bluetoothSerial.write(
        (const uint8_t*) frame,
        (size_t) written
    );

    bluetoothSerial.flush();


    outgoingState =
        OutgoingState::WAITING_ACK;

    ackWaitStart =
        millis();
}


void handleIncomingAck(
    uint32_t ackId
)
{
    if(
        outgoingState != OutgoingState::WAITING_ACK
    )
    {
        Serial.print(
            "[BT] Ignoring ACK, nothing in flight: "
        );

        Serial.println(
            ackId
        );

        return;
    }


    if(
        ackId != outgoingPacketId
    )
    {
        Serial.print(
            "[BT] Ignoring ACK for unrelated packet ID: "
        );

        Serial.println(
            ackId
        );

        return;
    }


    Serial.print(
        "[BT] ACK received for own packet ID: "
    );

    Serial.println(
        ackId
    );

    outgoingPacketId++;

    outgoingState =
        OutgoingState::IDLE;

    lastSendTime =
        millis();
}


void pumpOutgoing()
{
    uint32_t now =
        millis();


    if(
        outgoingState == OutgoingState::IDLE
    )
    {
        if(
            now - lastSendTime >= NODE2_SEND_INTERVAL_MS
        )
        {
            sendOutgoingData();
        }

        return;
    }


    //--------------------------------------------------
    // WAITING_ACK: si se pasó el timeout, reintenta el
    // mismo paquete (mismo packetID) en vez de perderlo.
    //--------------------------------------------------

    if(
        now - ackWaitStart >= NODE2_ACK_TIMEOUT_MS
    )
    {
        Serial.println(
            "[BT] ACK timeout for own packet, retrying..."
        );

        sendOutgoingData();
    }
}


void sendAck(
    uint32_t packetId
)
{
    char ackMessage[32];

    int written =
        snprintf(
            ackMessage,
            sizeof(ackMessage),
            "ACK|%lu\n",
            (unsigned long) packetId
        );

    if(
        written <= 0
    )
    {
        return;
    }

    Serial.println(
        "[BT] Sending:"
    );

    Serial.print(
        "ACK|"
    );

    Serial.println(
        packetId
    );

    bluetoothSerial.write(
        (const uint8_t*) ackMessage,
        (size_t) written
    );

    bluetoothSerial.flush();
}


//--------------------------------------------------
// Forward declaration: definida más abajo, procesa
// el cuerpo de un mensaje DATA|<packetID>|<payload>.
//--------------------------------------------------

void handleIncomingDataLine(
    char* line
);


void handleLine(
    char* line
)
{
    Serial.println(
        "[BT] Received:"
    );

    Serial.println(
        line
    );


    //--------------------------------------------------
    // DATA|<packetID>|<payload> -> viene de PersistentOS1,
    // hay que procesarlo y responder ACK.
    //--------------------------------------------------

    if(
        strncmp(
            line,
            "DATA|",
            5
        ) == 0
    )
    {
        handleIncomingDataLine(
            line
        );

        return;
    }


    //--------------------------------------------------
    // ACK|<packetID> -> confirma un DATA que este nodo
    // envió por su cuenta.
    //--------------------------------------------------

    if(
        strncmp(
            line,
            "ACK|",
            4
        ) == 0
    )
    {
        uint32_t ackId =
            (uint32_t) strtoul(
                line + 4,
                nullptr,
                10
            );

        handleIncomingAck(
            ackId
        );

        return;
    }


    Serial.println(
        "[BT] Ignoring unrecognized message"
    );
}


void handleIncomingDataLine(
    char* line
)
{
    char* idStart =
        line + 5;

    char* separator =
        strchr(
            idStart,
            '|'
        );

    if(
        separator == nullptr
    )
    {
        Serial.println(
            "[BT] Malformed DATA message (missing payload separator)"
        );

        return;
    }

    *separator =
        '\0';

    uint32_t packetId =
        (uint32_t) strtoul(
            idStart,
            nullptr,
            10
        );

    const char* payload =
        separator + 1;

    Serial.print(
        "[BT] Packet ID: "
    );

    Serial.println(
        packetId
    );


    if(
        wasAlreadyProcessed(packetId)
    )
    {
        Serial.println(
            "[BT] Duplicate packet - re-sending ACK without reprocessing"
        );
    }
    else
    {
        Serial.print(
            "[BT] Payload: "
        );

        Serial.println(
            payload
        );

        rememberProcessed(
            packetId
        );
    }


    sendAck(
        packetId
    );
}


void pumpIncomingBytes()
{
    while(
        bluetoothSerial.available()
    )
    {
        int c =
            bluetoothSerial.read();

        if(
            c < 0
        )
        {
            break;
        }

        if(
            c == '\n'
        )
        {
            if(
                lineLength > 0 &&
                lineBuffer[lineLength - 1] == '\r'
            )
            {
                lineLength--;
            }

            lineBuffer[lineLength] =
                '\0';

            handleLine(
                lineBuffer
            );

            lineLength =
                0;
        }
        else
        {
            if(
                lineLength < (MAX_LINE_SIZE - 1)
            )
            {
                lineBuffer[lineLength] =
                    (char) c;

                lineLength++;
            }
            else
            {
                Serial.println(
                    "[BT] Line buffer overflow, discarding"
                );

                lineLength =
                    0;
            }
        }
    }
}


} // namespace


void setup()
{
    Serial.begin(
        115200
    );

    delay(
        1000
    );

    Serial.println();

    Serial.println(
        "================================="
    );

    Serial.println(
        "      PERSISTENTOS2 - RECEIVER"
    );

    Serial.println(
        "================================="
    );

    Serial.println();


    //--------------------------------------------------
    // Reintentar la inicialización indefinidamente en
    // lugar de rendirse tras un solo intento.
    //--------------------------------------------------

    bool result =
        false;

    uint32_t attempt =
        0;

    while(
        !result
    )
    {
        attempt++;

        Serial.print(
            "[BT] Bluetooth begin attempt #"
        );

        Serial.println(
            attempt
        );

        result =
            bluetoothSerial.begin(
                BLUETOOTH_DEVICE_NAME
            );

        if(
            !result
        )
        {
            Serial.println(
                "[BT] Bluetooth initialization failed, retrying..."
            );

            bluetoothSerial.end();

            delay(
                BLUETOOTH_INIT_RETRY_DELAY_MS
            );
        }
    }

    Serial.print(
        "[BT] Device name: "
    );

    Serial.println(
        BLUETOOTH_DEVICE_NAME
    );

    Serial.println(
        "[BT] Waiting for PersistentOS1..."
    );
}


void loop()
{
    pumpIncomingBytes();

    pumpOutgoing();

    delay(
        10
    );
}

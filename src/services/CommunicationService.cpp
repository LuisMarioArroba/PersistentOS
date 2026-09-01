#include "services/CommunicationService.h"
#include "report/ReportLogger.h"

#include <string.h>
#include <stdlib.h>


//====================================================
// Constructor
//====================================================

CommunicationService::CommunicationService()
{
    buffer =
        nullptr;

    communicationManager =
        nullptr;

    sensorService =
        nullptr;

    receivedCount =
        0;

    receivedNextSlot =
        0;
}


//====================================================
// Begin
//====================================================

void CommunicationService::begin(
    CommunicationBuffer* bufferPtr,
    CommunicationManager* communicationPtr,
    ResumeManager* resumePtr,
    ExecutionCheckpoint* checkpointPtr,
    SensorService* sensorPtr,
    FailureManager* failurePtr,
    PersistentState* persistentStatePtr
)
{
    buffer =
        bufferPtr;

    communicationManager =
        communicationPtr;

    sensorService =
        sensorPtr;

    persistentState =
        persistentStatePtr;

    ResumableService::begin(
        resumePtr,
        checkpointPtr,
        TASK_COMMUNICATION,
        failurePtr
    );
}


//====================================================
// Normal execution
//====================================================

void CommunicationService::executeNormal()
{
    if(
        buffer == nullptr ||
        communicationManager == nullptr
    )
    {
        return;
    }


    //--------------------------------------------------
    // Siempre escuchar, sin importar el checkpoint del
    // envío propio: este nodo es emisor Y receptor.
    //--------------------------------------------------

    pollIncoming();


    uint8_t checkpoint =
        getCheckpoint();


    //--------------------------------------------------
    // IDLE / COMPLETE
    //--------------------------------------------------

    if(
        checkpoint == STEP_IDLE ||
        checkpoint == STEP_COMPLETE
    )
    {
        updateCheckpoint(
            STEP_COMMUNICATION_CREATE,
            20
        );


        Serial.println(
            "[COMM] Starting packet creation"
        );


        return;
    }


    //--------------------------------------------------
    // CREATE
    //--------------------------------------------------

    if(
        checkpoint == STEP_COMMUNICATION_CREATE
    )
    {
        createPacket();


        Serial.println(
            "[COMM] CREATE completed"
        );


        updateCheckpoint(
            STEP_COMMUNICATION_PREPARE,
            40
        );


        return;
    }


    //--------------------------------------------------
    // PREPARE
    //--------------------------------------------------

    if(
        checkpoint == STEP_COMMUNICATION_PREPARE
    )
    {
        CommunicationPacket* packet =
            buffer->front();


        if(
            packet == nullptr
        )
        {
            Serial.println(
                "[COMM] No packet to prepare"
            );

            return;
        }


        if(
            !preparePacket(packet)
        )
        {
            return;
        }


        Serial.println(
            "[COMM] PREPARE completed"
        );


        updateCheckpoint(
            STEP_COMMUNICATION_SEND,
            60
        );


        return;
    }


    //--------------------------------------------------
    // SEND
    //--------------------------------------------------

    if(
        checkpoint == STEP_COMMUNICATION_SEND
    )
    {
        CommunicationPacket* packet =
            buffer->front();


        if(
            packet == nullptr
        )
        {
            Serial.println(
                "[COMM] No packet to send"
            );

            return;
        }


        if(
            !sendPacket(packet)
        )
        {
            Serial.println(
                "[COMM] Send failed"
            );

            return;
        }


        Serial.println(
            "[COMM] SEND completed"
        );


        updateCheckpoint(
            STEP_COMMUNICATION_WAIT_ACK,
            80
        );


        return;
    }


    //--------------------------------------------------
    // WAIT ACK
    //--------------------------------------------------

    if(
        checkpoint == STEP_COMMUNICATION_WAIT_ACK
    )
    {
        CommunicationPacket* packet =
            buffer->front();


        if(
            packet == nullptr
        )
        {
            Serial.println(
                "[COMM] No packet waiting ACK"
            );

            return;
        }


        if(
            !waitACK(packet)
        )
        {
            Serial.println(
                "[COMM] ACK failed"
            );

            return;
        }


        Serial.println(
            "[COMM] ACK received"
        );


        updateCheckpoint(
            STEP_COMMUNICATION_CONFIRM,
            95
        );


        return;
    }


    //--------------------------------------------------
    // CONFIRM
    //--------------------------------------------------

    if(
        checkpoint == STEP_COMMUNICATION_CONFIRM
    )
    {
        CommunicationPacket* packet =
            buffer->front();


        if(
            packet == nullptr
        )
        {
            Serial.println(
                "[COMM] No packet to confirm"
            );

            return;
        }


        confirmPacket(
            packet
        );


        finishExecution();


        return;
    }


    //--------------------------------------------------
    // INVALID
    //--------------------------------------------------

    Serial.print(
        "[COMM] Invalid checkpoint: "
    );

    Serial.println(
        checkpoint
    );
}


//====================================================
// Resume execution
//====================================================

void CommunicationService::executeResume()
{
    if(
        buffer == nullptr ||
        communicationManager == nullptr
    )
    {
        Serial.println(
            "[COMM Resume] Dependencies not available"
        );

        return;
    }


    //--------------------------------------------------
    // Siempre escuchar, sin importar el checkpoint del
    // envío propio: este nodo es emisor Y receptor.
    //--------------------------------------------------

    pollIncoming();


    //--------------------------------------------------
    // IMPORTANT:
    // Read checkpoint BEFORE accessing buffer.
    //--------------------------------------------------

    uint8_t checkpoint =
        getCheckpoint();


    Serial.print(
        "[COMM Resume] Checkpoint: "
    );

    Serial.println(
        checkpoint
    );


    //--------------------------------------------------
    // CREATE
    //--------------------------------------------------

    if(
        checkpoint == STEP_COMMUNICATION_CREATE
    )
    {
        Serial.println(
            "[COMM Resume] Continuing from CREATE"
        );


        createPacket();


        updateCheckpoint(
            STEP_COMMUNICATION_PREPARE,
            40
        );


        return;
    }


    //--------------------------------------------------
    // PREPARE
    //--------------------------------------------------

    if(
        checkpoint == STEP_COMMUNICATION_PREPARE
    )
    {
        CommunicationPacket* packet =
            buffer->front();


        if(
            packet == nullptr
        )
        {
            Serial.println(
                "[COMM Resume] No packet at PREPARE"
            );

            return;
        }


        Serial.print(
            "[COMM Resume] PREPARE packet ID: "
        );

        Serial.println(
            packet->packetID
        );


        if(
            !preparePacket(packet)
        )
        {
            return;
        }


        updateCheckpoint(
            STEP_COMMUNICATION_SEND,
            60
        );


        return;
    }


    //--------------------------------------------------
    // SEND
    //--------------------------------------------------

    if(
        checkpoint == STEP_COMMUNICATION_SEND
    )
    {
        CommunicationPacket* packet =
            buffer->front();


        if(
            packet == nullptr
        )
        {
            Serial.println(
                "[COMM Resume] No packet at SEND"
            );

            return;
        }


        Serial.print(
            "[COMM Resume] SEND packet ID: "
        );

        Serial.println(
            packet->packetID
        );


        if(
            !sendPacket(packet)
        )
        {
            Serial.println(
                "[COMM Resume] SEND failed"
            );

            return;
        }


        updateCheckpoint(
            STEP_COMMUNICATION_WAIT_ACK,
            80
        );


        return;
    }


    //--------------------------------------------------
    // WAIT ACK
    //
    // IMPORTANT:
    //
    // There is NO CREATE.
    // There is NO PREPARE.
    // There is NO SEND.
    //
    // We continue directly here.
    //--------------------------------------------------

    if(
        checkpoint == STEP_COMMUNICATION_WAIT_ACK
    )
    {
        CommunicationPacket* packet =
            buffer->front();


        if(
            packet == nullptr
        )
        {
            Serial.println(
                "[COMM Resume] No packet at WAIT_ACK"
            );

            return;
        }


        Serial.print(
            "[COMM Resume] WAIT_ACK packet ID: "
        );

        Serial.println(
            packet->packetID
        );


        //--------------------------------------------------
        // ACK already received before reboot
        //--------------------------------------------------

        if(
            packet->ackStatus ==
            ACK_RECEIVED
        )
        {
            Serial.println(
                "[COMM Resume] ACK already received"
            );


            updateCheckpoint(
                STEP_COMMUNICATION_CONFIRM,
                95
            );


            return;
        }


        //--------------------------------------------------
        // Continue waiting
        //--------------------------------------------------

        if(
            !waitACK(packet)
        )
        {
            Serial.println(
                "[COMM Resume] ACK still pending"
            );

            return;
        }


        Serial.println(
            "[COMM Resume] ACK received"
        );


        updateCheckpoint(
            STEP_COMMUNICATION_CONFIRM,
            95
        );


        return;
    }


    //--------------------------------------------------
    // CONFIRM
    //--------------------------------------------------

    if(
        checkpoint == STEP_COMMUNICATION_CONFIRM
    )
    {
        CommunicationPacket* packet =
            buffer->front();


        if(
            packet == nullptr
        )
        {
            Serial.println(
                "[COMM Resume] No packet at CONFIRM"
            );

            return;
        }


        Serial.print(
            "[COMM Resume] CONFIRM packet ID: "
        );

        Serial.println(
            packet->packetID
        );


        confirmPacket(
            packet
        );


        finishExecution();


        return;
    }


    //--------------------------------------------------
    // Invalid
    //--------------------------------------------------

    Serial.print(
        "[COMM Resume] Invalid checkpoint: "
    );

    Serial.println(
        checkpoint
    );
}


//====================================================
// Create packet
//====================================================

void CommunicationService::createPacket()
{
    CommunicationPacket packet;


    packet.packetID =
        buffer->nextPacketID();


    packet.timestamp =
        millis();


    packet.status =
        PACKET_PENDING;


    packet.retries =
        0;


    packet.ackStatus =
        ACK_NONE;


    packet.bytesSent =
        0;


    packet.encrypted =
        false;


    packet.checkpoint =
        STEP_COMMUNICATION_CREATE;


    ReportLogger::event(
        "CREATE",
        packet.packetID,
        0
    );


    //--------------------------------------------------
    // Get sensor value
    //--------------------------------------------------

    float value =
    0.0;

    if(
        persistentState != nullptr
    )
    {
        value =
            persistentState->lastSensorValue;
    }


    //--------------------------------------------------
    // Create message
    //--------------------------------------------------

    char message[32];


    snprintf(
        message,
        sizeof(message),
        "TEMP: %.2f",
        value
    );


    packet.length =
        strlen(message);


    memcpy(
        packet.payload,
        message,
        packet.length
    );


    //--------------------------------------------------
    // Store packet
    //--------------------------------------------------

    buffer->push(
        packet
    );


    Serial.print(
        "[COMM] Packet created ID: "
    );

    Serial.println(
        packet.packetID
    );


    Serial.print(
        "[COMM] Payload: "
    );

    Serial.println(
        message
    );
}


//====================================================
// Prepare
//====================================================

bool CommunicationService::preparePacket(
    CommunicationPacket* packet
)
{
    if(
        packet == nullptr
    )
    {
        return false;
    }


    packet->status =
        PACKET_READY;


    packet->checkpoint =
        STEP_COMMUNICATION_PREPARE;


    Serial.print(
        "[COMM] Preparing packet ID: "
    );

    Serial.println(
        packet->packetID
    );


    return true;
}


//====================================================
// Send
//====================================================

bool CommunicationService::sendPacket(
    CommunicationPacket* packet
)
{
    if(
        packet == nullptr
    )
    {
        return false;
    }


    packet->status =
        PACKET_WAITING_ACK;


    packet->ackStatus =
        ACK_WAITING;


    packet->retries++;


    //--------------------------------------------------
    // Build protocol frame: DATA|<packetID>|<payload>\n
    //--------------------------------------------------

    char frame[MAX_LINE_SIZE];


    int prefixWritten =
        snprintf(
            frame,
            sizeof(frame),
            "DATA|%lu|",
            (unsigned long) packet->packetID
        );


    if(
        prefixWritten < 0 ||
        (size_t) prefixWritten >= sizeof(frame)
    )
    {
        Serial.println(
            "[COMM] Failed to build DATA frame (prefix)"
        );

        packet->ackStatus =
            ACK_FAILED;

        return false;
    }


    size_t prefixLength =
        (size_t) prefixWritten;


    size_t payloadLength =
        packet->length;


    //--------------------------------------------------
    // Reserve 2 bytes: '\n' + safety
    //--------------------------------------------------

    if(
        prefixLength + payloadLength + 1 >
        sizeof(frame)
    )
    {
        Serial.println(
            "[COMM] Payload too large for protocol frame"
        );

        packet->ackStatus =
            ACK_FAILED;

        return false;
    }


    memcpy(
        frame + prefixLength,
        packet->payload,
        payloadLength
    );


    frame[prefixLength + payloadLength] =
        '\n';


    size_t frameLength =
        prefixLength + payloadLength + 1;


    Serial.print(
        "[COMM BT] Sending: "
    );

    Serial.write(
        (const uint8_t*) frame,
        frameLength - 1
    );

    Serial.println();


    bool result =
        communicationManager->send(
            (const uint8_t*) frame,
            frameLength
        );


    if(result)
    {
        //--------------------------------------------------
        // Marca el momento del envío (inicial o reintento).
        // waitACK() usa este valor para decidir cuándo
        // reintentar el mismo packetID.
        //--------------------------------------------------

        packet->timestamp =
            millis();

        packet->checkpoint =
            STEP_COMMUNICATION_WAIT_ACK;


        //--------------------------------------------------
        // retries == 1 es el envío inicial (incrementado
        // arriba en cada llamada); retries > 1 es un
        // reintento real sobre el mismo packetID.
        //--------------------------------------------------

        ReportLogger::event(
            packet->retries > 1
                ? "RETRY"
                : "SEND",
            packet->packetID,
            packet->retries
        );
    }
    else
    {
        packet->ackStatus =
            ACK_FAILED;
    }


    return result;
}


//====================================================
// Wait ACK
//====================================================

bool CommunicationService::waitACK(
    CommunicationPacket* packet
)
{
    if(
        packet == nullptr
    )
    {
        return false;
    }


    //--------------------------------------------------
    // El drenado real del puerto BT y el emparejamiento
    // ACK<->packetID ocurren en pollIncoming() /
    // handleIncomingAck(), que se ejecutan en cada tick
    // sin importar el checkpoint actual (así este nodo
    // puede seguir recibiendo DATA remoto al mismo
    // tiempo que espera su propio ACK). Aquí solo se
    // consulta el resultado.
    //--------------------------------------------------

    if(
        packet->ackStatus == ACK_RECEIVED
    )
    {
        return true;
    }


    //--------------------------------------------------
    // ACK not received yet. Si ya pasó COMM_ACK_TIMEOUT_MS
    // desde el último envío, reintenta el mismo packetID en
    // vez de quedarse esperando indefinidamente (esto es lo
    // que dejaba a PersistentOS1 trabado en WAIT_ACK para
    // siempre tras un reboot: se saltaba directo a WAIT_ACK
    // sin reenviar, y el paquete original nunca llegó a
    // PersistentOS2 o su ACK se perdió con la reconexión).
    //
    // Mismo patrón que NODE2_ACK_TIMEOUT_MS en main_node2.cpp.
    //--------------------------------------------------

    uint32_t now =
        millis();

    if(
        now - packet->timestamp >= COMM_ACK_TIMEOUT_MS
    )
    {
        Serial.print(
            "[COMM] ACK timeout for packet ID: "
        );

        Serial.print(
            packet->packetID
        );

        Serial.println(
            ", retrying..."
        );

        sendPacket(
            packet
        );
    }


    packet->ackStatus =
        ACK_WAITING;

    return false;
}


//====================================================
// Confirm
//====================================================

void CommunicationService::confirmPacket(
    CommunicationPacket* packet
)
{
    if(
        packet == nullptr
    )
    {
        return;
    }


    uint32_t id =
        packet->packetID;


    //--------------------------------------------------
    // Validate ACK
    //--------------------------------------------------

    if(
        packet->ackStatus !=
        ACK_RECEIVED
    )
    {
        Serial.println(
            "[COMM] ACK missing"
        );


        packet->status =
            PACKET_WAITING_ACK;


        return;
    }


    //--------------------------------------------------
    // Confirm
    //--------------------------------------------------

    packet->status =
        PACKET_CONFIRMED;


    packet->checkpoint =
        STEP_COMMUNICATION_CONFIRM;


    ReportLogger::event(
        "CONFIRM",
        id,
        packet->retries
    );

    Serial.print(
        "[COMM] Packet confirmed: "
    );

    Serial.println(
        id
    );


    //--------------------------------------------------
    // Remove packet
    //--------------------------------------------------

    buffer->pop();
}


//====================================================
// Poll incoming (DATA remoto + ACK propio)
//====================================================

void CommunicationService::pollIncoming()
{
    if(
        communicationManager == nullptr
    )
    {
        return;
    }


    char line[MAX_LINE_SIZE];


    while(
        communicationManager->receiveLine(
            line,
            sizeof(line)
        )
    )
    {
        //--------------------------------------------------
        // DATA|<packetID>|<payload>  -> viene del nodo remoto
        //--------------------------------------------------

        if(
            strncmp(
                line,
                "DATA|",
                5
            ) == 0
        )
        {
            handleIncomingData(
                line
            );

            continue;
        }


        //--------------------------------------------------
        // ACK|<packetID>  -> confirma algo que este nodo envió
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

            continue;
        }


        Serial.print(
            "[COMM RX] Ignoring unrecognized message: "
        );

        Serial.println(
            line
        );
    }
}


//====================================================
// Handle incoming DATA (energy/estado del nodo remoto)
//====================================================

void CommunicationService::handleIncomingData(
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
            "[COMM RX] Malformed DATA message (missing payload separator)"
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


    if(
        wasAlreadyReceived(packetId)
    )
    {
        Serial.println(
            "[COMM RX] Duplicate packet - re-sending ACK without reprocessing"
        );
    }
    else
    {
        //--------------------------------------------------
        // Por ahora solo se registra en el log. Cuando el
        // comportamiento de energía esté implementado en
        // ambos nodos, este es el punto donde se debería
        // integrar el dato remoto (FRAM / EnergyManager /
        // EnergyPredictionManager, según corresponda).
        //--------------------------------------------------

        Serial.print(
            "[COMM RX] Remote data (packet "
        );

        Serial.print(
            packetId
        );

        Serial.print(
            "): "
        );

        Serial.println(
            payload
        );

        rememberReceived(
            packetId
        );
    }


    sendAckFor(
        packetId
    );
}


//====================================================
// Handle incoming ACK (confirma un DATA propio)
//====================================================

void CommunicationService::handleIncomingAck(
    uint32_t ackId
)
{
    if(
        buffer == nullptr
    )
    {
        return;
    }


    CommunicationPacket* packet =
        buffer->front();


    if(
        packet == nullptr
    )
    {
        Serial.print(
            "[COMM] Ignoring ACK, no packet in flight: "
        );

        Serial.println(
            ackId
        );

        return;
    }


    if(
        ackId != packet->packetID
    )
    {
        Serial.print(
            "[COMM] Ignoring ACK for unrelated packet ID: "
        );

        Serial.println(
            ackId
        );

        return;
    }


    packet->ackStatus =
        ACK_RECEIVED;

    packet->status =
        PACKET_SENT;

    packet->checkpoint =
        STEP_COMMUNICATION_CONFIRM;

    Serial.print(
        "[COMM] ACK received for packet ID: "
    );

    Serial.println(
        ackId
    );
}


//====================================================
// Send ACK for a received DATA packet
//====================================================

void CommunicationService::sendAckFor(
    uint32_t packetId
)
{
    if(
        communicationManager == nullptr
    )
    {
        return;
    }


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

    Serial.print(
        "[COMM TX] ACK|"
    );

    Serial.println(
        packetId
    );

    communicationManager->send(
        (const uint8_t*) ackMessage,
        (size_t) written
    );
}


//====================================================
// Idempotencia de DATA entrante
//====================================================

bool CommunicationService::wasAlreadyReceived(
    uint32_t packetId
)
{
    for(
        uint8_t i = 0;
        i < receivedCount;
        i++
    )
    {
        if(
            receivedIds[i] == packetId
        )
        {
            return true;
        }
    }

    return false;
}


void CommunicationService::rememberReceived(
    uint32_t packetId
)
{
    receivedIds[receivedNextSlot] =
        packetId;

    receivedNextSlot =
        (receivedNextSlot + 1) % 8;

    if(
        receivedCount < 8
    )
    {
        receivedCount++;
    }
}

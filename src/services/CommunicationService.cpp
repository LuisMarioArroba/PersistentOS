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

    energyManager =
        nullptr;

    localEnergyPrediction =
        nullptr;

    remoteEnergyPrediction =
        nullptr;

    behaviorManager =
        nullptr;

    receivedCount =
        0;

    receivedNextSlot =
        0;

    alarmPending =
        false;

    alarmPayloadLength =
        0;

    for(
        uint8_t i = 0;
        i < MAX_KNOWN_NODES;
        i++
    )
    {
        knownNodes[i].nodeId = 0;
        knownNodes[i].temperature = 0.0f;
        knownNodes[i].valid = false;
        knownNodes[i].lastSeen = 0;
    }

    for(
        uint8_t i = 0;
        i < MAX_RELAYED_ALARMS;
        i++
    )
    {
        relayedAlarms[i].originNodeId = 0;
        relayedAlarms[i].alarmType = 0;
        relayedAlarms[i].timestamp = 0;
    }
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
    PersistentState* persistentStatePtr,
    EnergyManager* energyManagerPtr,
    EnergyPredictionManager* localEnergyPredictionPtr,
    EnergyPredictionManager* remoteEnergyPredictionPtr,
    BehaviorManager* behaviorManagerPtr
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

    energyManager =
        energyManagerPtr;

    localEnergyPrediction =
        localEnergyPredictionPtr;

    remoteEnergyPrediction =
        remoteEnergyPredictionPtr;

    behaviorManager =
        behaviorManagerPtr;

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
        if(
            !createPacket()
        )
        {
            //--------------------------------------------------
            // No hay alarma pendiente y la ventana de energía
            // no es favorable: se queda en CREATE y lo vuelve
            // a intentar en el siguiente tick, sin bloquear el
            // resto del sistema.
            //--------------------------------------------------

            return;
        }


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


        if(
            !createPacket()
        )
        {
            //--------------------------------------------------
            // Igual que en executeNormal(): sin alarma pendiente
            // y con ventana desfavorable, se queda en CREATE y
            // lo reintenta en el siguiente tick.
            //--------------------------------------------------

            return;
        }


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

bool CommunicationService::createPacket()
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


    //--------------------------------------------------
    // Alarma pendiente: tiene prioridad sobre cualquier
    // telemetría de rutina y se crea sin pasar por la
    // ventana de comunicación favorable (una alarma se
    // intenta siempre, de inmediato).
    //--------------------------------------------------

    if(
        alarmPending
    )
    {
        packet.kind =
            PACKET_KIND_ALARM;

        packet.length =
            alarmPayloadLength;

        memcpy(
            packet.payload,
            alarmPayload,
            alarmPayloadLength
        );

        alarmPending =
            false;

        ReportLogger::event(
            "CREATE_ALARM",
            packet.packetID,
            0
        );

        buffer->push(
            packet
        );

        Serial.print(
            "[COMM] PRIORITY alarm packet created ID: "
        );

        Serial.println(
            packet.packetID
        );

        Serial.print(
            "[COMM] Payload: "
        );

        Serial.write(
            (const uint8_t*) alarmPayload,
            alarmPayloadLength
        );

        Serial.println();

        return true;
    }


    //--------------------------------------------------
    // Telemetría de rutina: se detiene si la ventana de
    // comunicación no es favorable en este momento, en
    // vez de insistir gastando energía en un intento con
    // pocas probabilidades de completarse.
    //--------------------------------------------------

    if(
        !isCommunicationWindowFavorable()
    )
    {
        return false;
    }


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

    bool sensorValid =
        false;

    if(
        persistentState != nullptr
    )
    {
        value =
            persistentState->lastSensorValue;

        sensorValid =
            persistentState->lastSensorValid;
    }


    float localEnergy =
        0.0f;

    if(
        energyManager != nullptr
    )
    {
        localEnergy =
            energyManager->getEnergy();
    }


    //--------------------------------------------------
    // Actualizar la propia entrada en la tabla de nodos
    // conocidos antes de anunciarla.
    //--------------------------------------------------

    updateKnownNode(
        (uint8_t) PERSISTENT_OS_NODE_ID,
        value,
        sensorValid
    );


    //--------------------------------------------------
    // Create message: identidad de este nodo, temperatura
    // (o "NA" si el sensor no está conectado, p.ej.
    // PersistentOS2 sin DS18B20 todavía), energía local, y
    // un resumen de la última temperatura conocida de los
    // demás nodos (propio + recibida transitivamente), para
    // que la información se propague en cada salto y no
    // solo entre pares directos.
    //--------------------------------------------------

    char knownSummary[32];

    buildKnownSummary(
        knownSummary,
        sizeof(knownSummary)
    );


    char message[MAX_PACKET_SIZE];


    if(
        sensorValid
    )
    {
        snprintf(
            message,
            sizeof(message),
            "NODE:%d|TEMP:%.2f|ENERGY:%.2f|K:%s",
            (int) PERSISTENT_OS_NODE_ID,
            value,
            localEnergy,
            knownSummary
        );
    }
    else
    {
        snprintf(
            message,
            sizeof(message),
            "NODE:%d|TEMP:NA|ENERGY:%.2f|K:%s",
            (int) PERSISTENT_OS_NODE_ID,
            localEnergy,
            knownSummary
        );
    }


    packet.kind =
        PACKET_KIND_DATA;

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


    return true;
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
    // o ALARM|<packetID>|<payload>\n según packet->kind.
    //--------------------------------------------------

    char frame[MAX_LINE_SIZE];


    const char* prefixTag =
        (packet->kind == PACKET_KIND_ALARM)
            ? "ALARM"
            : "DATA";


    int prefixWritten =
        snprintf(
            frame,
            sizeof(frame),
            "%s|%lu|",
            prefixTag,
            (unsigned long) packet->packetID
        );


    if(
        prefixWritten < 0 ||
        (size_t) prefixWritten >= sizeof(frame)
    )
    {
        Serial.println(
            "[COMM] Failed to build protocol frame (prefix)"
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
                line,
                false
            );

            continue;
        }


        //--------------------------------------------------
        // ALARM|<packetID>|<payload>  -> alerta de prioridad
        // del nodo remoto (mismo protocolo, mismo ACK).
        //--------------------------------------------------

        if(
            strncmp(
                line,
                "ALARM|",
                6
            ) == 0
        )
        {
            handleIncomingData(
                line,
                true
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
    char* line,

    bool isAlarm
)
{
    char* idStart =
        line +
        (isAlarm ? 6 : 5);

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
            "[COMM RX] Malformed message (missing payload separator)"
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

        sendAckFor(
            packetId
        );

        return;
    }


    //--------------------------------------------------
    // Identidad del remitente: NODE:<id> siempre va al
    // principio del payload, tanto en DATA como en ALARM.
    //--------------------------------------------------

    uint8_t senderNodeId =
        0;

    const char* nodeTag =
        strstr(
            payload,
            "NODE:"
        );

    if(
        nodeTag != nullptr
    )
    {
        senderNodeId =
            (uint8_t) strtoul(
                nodeTag + 5,
                nullptr,
                10
            );
    }


    if(
        isAlarm
    )
    {
        //--------------------------------------------------
        // Formato: NODE:<id>|ALARM:TEMP_LOW:<valor>  o
        //          NODE:<id>|ALARM:TEMP_HIGH:<valor>
        //--------------------------------------------------

        uint8_t alarmType =
            0;

        float alarmTemperature =
            0.0f;

        const char* lowTag =
            strstr(
                payload,
                "ALARM:TEMP_LOW:"
            );

        const char* highTag =
            strstr(
                payload,
                "ALARM:TEMP_HIGH:"
            );

        if(
            lowTag != nullptr
        )
        {
            alarmType = 1;

            alarmTemperature =
                strtof(
                    lowTag + 16,
                    nullptr
                );
        }
        else if(
            highTag != nullptr
        )
        {
            alarmType = 2;

            alarmTemperature =
                strtof(
                    highTag + 17,
                    nullptr
                );
        }


        Serial.println();

        Serial.println(
            "[ALARM] ==============================="
        );

        Serial.print(
            "[ALARM] Nodo "
        );

        Serial.print(
            senderNodeId
        );

        Serial.print(
            " presento la alarma de temperatura "
        );

        Serial.print(
            alarmType == 1 ? "baja " : "alta "
        );

        Serial.print(
            alarmTemperature
        );

        Serial.println(
            " C"
        );

        Serial.println(
            "[ALARM] ==============================="
        );


        if(
            senderNodeId != 0
        )
        {
            updateKnownNode(
                senderNodeId,
                alarmTemperature,
                true
            );
        }


        //--------------------------------------------------
        // Relevo: si la alarma es de OTRO nodo (no de este
        // mismo) y todavía no se reenvió, se vuelve a
        // encolar como prioritaria para que llegue al
        // siguiente salto (primary o secondary), en vez de
        // quedarse solo en este par directo.
        //--------------------------------------------------

        if(
            senderNodeId != 0 &&
            senderNodeId != (uint8_t) PERSISTENT_OS_NODE_ID &&
            alarmType != 0 &&
            !wasAlreadyRelayed(senderNodeId, alarmType)
        )
        {
            rememberRelayed(
                senderNodeId,
                alarmType
            );

            Serial.println(
                "[ALARM] Relevando hacia el siguiente salto"
            );

            requestAlarm(
                payload,
                (uint16_t) strlen(payload)
            );
        }


        rememberReceived(
            packetId
        );

        sendAckFor(
            packetId
        );

        return;
    }


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


    //--------------------------------------------------
    // Extraer TEMP:<valor> (o NA) del remitente directo y
    // actualizar su entrada en la tabla de nodos conocidos.
    //--------------------------------------------------

    if(
        senderNodeId != 0
    )
    {
        const char* tempTag =
            strstr(
                payload,
                "TEMP:"
            );

        if(
            tempTag != nullptr &&
            strncmp(
                tempTag + 5,
                "NA",
                2
            ) != 0
        )
        {
            updateKnownNode(
                senderNodeId,
                strtof(
                    tempTag + 5,
                    nullptr
                ),
                true
            );
        }
        else
        {
            updateKnownNode(
                senderNodeId,
                0.0f,
                false
            );
        }
    }


    //--------------------------------------------------
    // Fusionar el resumen K:<id>:<temp>,... con lo que este
    // nodo ya sabe -- así la información de un tercer nodo
    // llega aunque nunca se haya conectado directamente con
    // él, siempre que algún salto la haya traído.
    //--------------------------------------------------

    const char* knownTag =
        strstr(
            payload,
            "K:"
        );

    if(
        knownTag != nullptr
    )
    {
        parseKnownSummary(
            knownTag + 2
        );
    }


    //--------------------------------------------------
    // Extraer ENERGY:<valor> del payload (si viene) y
    // alimentar la predicción del lado remoto y el
    // clasificador de comportamiento con el par
    // (energía local, energía remota). Ninguno de los
    // dos nodos conoce el perfil real que el tercer
    // equipo aplica: esto solo usa lo observado.
    //--------------------------------------------------

    const char* energyTag =
        strstr(
            payload,
            "ENERGY:"
        );

    if(
        energyTag != nullptr
    )
    {
        float remoteEnergy =
            strtof(
                energyTag + 7,
                nullptr
            );

        if(
            remoteEnergyPrediction != nullptr
        )
        {
            remoteEnergyPrediction->observe(
                remoteEnergy
            );
        }

        if(
            behaviorManager != nullptr &&
            energyManager != nullptr
        )
        {
            behaviorManager->observe(
                energyManager->getEnergy(),
                remoteEnergy,
                millis()
            );
        }
    }


    rememberReceived(
        packetId
    );


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


//====================================================
// Predicción de ventana de comunicación
//====================================================
//
// Ni este nodo ni el remoto conocen el comportamiento
// real de su fuente de alimentación (el tercer equipo
// que la controla); esta función solo mira lo que cada
// uno observó: la energía local propia y la última
// energía remota conocida, junto con su tendencia
// reciente. No es una predicción exacta del intervalo,
// es un heurístico probabilístico de primera
// aproximación, tal como se documenta en el paper.
//====================================================

bool CommunicationService::isCommunicationWindowFavorable()
{
    //--------------------------------------------------
    // Sin alguno de los dos predictores no hay base para
    // decidir: se deja pasar en vez de bloquear el envío.
    //--------------------------------------------------

    if(
        energyManager == nullptr ||
        localEnergyPrediction == nullptr ||
        remoteEnergyPrediction == nullptr
    )
    {
        return true;
    }


    float localValue =
        energyManager->getEnergy();

    float localTrend =
        localEnergyPrediction->getTrend();

    bool localLow =
        localValue < COMM_MIN_FAVORABLE_ENERGY;

    bool localFalling =
        localTrend < -COMM_TREND_FALLING_THRESHOLD;

    if(
        localLow &&
        localFalling
    )
    {
        Serial.print(
            "[COMM] Window unfavorable: local energy low and falling ("
        );

        Serial.print(
            localValue
        );

        Serial.println(
            "%)"
        );

        return false;
    }


    //--------------------------------------------------
    // Sin observaciones remotas todavía (arranque, o el
    // otro nodo nunca ha enviado nada): no hay nada que
    // evaluar del lado remoto, se deja pasar.
    //--------------------------------------------------

    if(
        remoteEnergyPrediction->getSampleCount() == 0
    )
    {
        return true;
    }


    float remoteValue =
        remoteEnergyPrediction->getCurrentValue();

    float remoteTrend =
        remoteEnergyPrediction->getTrend();

    bool remoteLow =
        remoteValue < COMM_MIN_FAVORABLE_ENERGY;

    bool remoteFalling =
        remoteTrend < -COMM_TREND_FALLING_THRESHOLD;

    if(
        remoteLow &&
        remoteFalling
    )
    {
        Serial.print(
            "[COMM] Window unfavorable: last known remote energy low and falling ("
        );

        Serial.print(
            remoteValue
        );

        Serial.println(
            "%)"
        );

        return false;
    }


    return true;
}


//====================================================
// Solicitud de alarma (desde AlarmManager)
//====================================================

void CommunicationService::requestAlarm(
    const char* payload,
    uint16_t length
)
{
    if(
        payload == nullptr
    )
    {
        return;
    }


    if(
        length >= sizeof(alarmPayload)
    )
    {
        length =
            sizeof(alarmPayload) - 1;
    }


    memcpy(
        alarmPayload,
        payload,
        length
    );


    alarmPayloadLength =
        length;

    alarmPending =
        true;


    Serial.print(
        "[COMM] Alarm requested, will be created next: "
    );

    Serial.write(
        (const uint8_t*) alarmPayload,
        alarmPayloadLength
    );

    Serial.println();
}


//====================================================
// Tabla de nodos conocidos
//====================================================

void CommunicationService::updateKnownNode(
    uint8_t nodeId,
    float temperature,
    bool valid
)
{
    if(
        nodeId == 0
    )
    {
        return;
    }


    int8_t freeSlot =
        -1;

    for(
        uint8_t i = 0;
        i < MAX_KNOWN_NODES;
        i++
    )
    {
        if(
            knownNodes[i].nodeId == nodeId
        )
        {
            knownNodes[i].temperature = temperature;
            knownNodes[i].valid = valid;
            knownNodes[i].lastSeen = millis();

            return;
        }

        if(
            freeSlot < 0 &&
            knownNodes[i].nodeId == 0
        )
        {
            freeSlot = (int8_t) i;
        }
    }


    if(
        freeSlot >= 0
    )
    {
        knownNodes[freeSlot].nodeId = nodeId;
        knownNodes[freeSlot].temperature = temperature;
        knownNodes[freeSlot].valid = valid;
        knownNodes[freeSlot].lastSeen = millis();

        Serial.print(
            "[NODES] Nodo "
        );

        Serial.print(
            nodeId
        );

        Serial.println(
            " agregado a la tabla de nodos conocidos"
        );
    }

    //--------------------------------------------------
    // Tabla llena y nodo desconocido: se descarta en
    // silencio -- MAX_KNOWN_NODES ya cubre la topología
    // de este trabajo (hasta 3 nodos).
    //--------------------------------------------------
}


void CommunicationService::buildKnownSummary(
    char* buffer,
    size_t bufferSize
)
{
    if(
        buffer == nullptr ||
        bufferSize == 0
    )
    {
        return;
    }

    buffer[0] = '\0';

    size_t used = 0;

    bool first = true;

    for(
        uint8_t i = 0;
        i < MAX_KNOWN_NODES;
        i++
    )
    {
        if(
            knownNodes[i].nodeId == 0 ||
            knownNodes[i].nodeId == (uint8_t) PERSISTENT_OS_NODE_ID
        )
        {
            //--------------------------------------------------
            // Slot vacío, o la propia entrada (ya va en el
            // campo NODE:/TEMP: principal del mensaje, no
            // hace falta repetirla en el resumen).
            //--------------------------------------------------

            continue;
        }

        char entry[16];

        if(
            knownNodes[i].valid
        )
        {
            snprintf(
                entry,
                sizeof(entry),
                "%d:%.1f",
                knownNodes[i].nodeId,
                knownNodes[i].temperature
            );
        }
        else
        {
            snprintf(
                entry,
                sizeof(entry),
                "%d:NA",
                knownNodes[i].nodeId
            );
        }

        size_t entryLen =
            strlen(entry);

        //--------------------------------------------------
        // +1 por la coma separadora si no es la primera.
        //--------------------------------------------------

        size_t needed =
            entryLen + (first ? 0 : 1);

        if(
            used + needed >= bufferSize
        )
        {
            //--------------------------------------------------
            // No entra más en el presupuesto de payload; se
            // corta el resumen aquí en vez de desbordar.
            //--------------------------------------------------

            break;
        }

        if(
            !first
        )
        {
            buffer[used] = ',';
            used++;
            buffer[used] = '\0';
        }

        strcat(
            buffer,
            entry
        );

        used += entryLen;

        first = false;
    }
}


void CommunicationService::parseKnownSummary(
    const char* summary
)
{
    if(
        summary == nullptr
    )
    {
        return;
    }

    char copy[32];

    strncpy(
        copy,
        summary,
        sizeof(copy) - 1
    );

    copy[sizeof(copy) - 1] = '\0';


    char* cursor =
        copy;

    while(
        cursor != nullptr &&
        *cursor != '\0'
    )
    {
        char* comma =
            strchr(
                cursor,
                ','
            );

        if(
            comma != nullptr
        )
        {
            *comma = '\0';
        }

        char* colon =
            strchr(
                cursor,
                ':'
            );

        if(
            colon != nullptr
        )
        {
            *colon = '\0';

            uint8_t entryNodeId =
                (uint8_t) strtoul(
                    cursor,
                    nullptr,
                    10
                );

            const char* entryValue =
                colon + 1;

            if(
                entryNodeId != 0 &&
                entryNodeId != (uint8_t) PERSISTENT_OS_NODE_ID
            )
            {
                if(
                    strncmp(
                        entryValue,
                        "NA",
                        2
                    ) != 0
                )
                {
                    updateKnownNode(
                        entryNodeId,
                        strtof(
                            entryValue,
                            nullptr
                        ),
                        true
                    );
                }
            }
        }

        cursor =
            (comma != nullptr) ? (comma + 1) : nullptr;
    }
}


//====================================================
// Relevo de alarmas ajenas
//====================================================

bool CommunicationService::wasAlreadyRelayed(
    uint8_t originNodeId,
    uint8_t alarmType
)
{
    uint32_t now =
        millis();

    for(
        uint8_t i = 0;
        i < MAX_RELAYED_ALARMS;
        i++
    )
    {
        if(
            relayedAlarms[i].originNodeId == originNodeId &&
            relayedAlarms[i].alarmType == alarmType
        )
        {
            //--------------------------------------------------
            // Se considera "la misma" alarma solo dentro de
            // una ventana de tiempo; pasado ese lapso, una
            // nueva ocurrencia del mismo tipo desde el mismo
            // nodo sí se vuelve a relevar.
            //--------------------------------------------------

            if(
                now - relayedAlarms[i].timestamp < 60000
            )
            {
                return true;
            }

            return false;
        }
    }

    return false;
}


void CommunicationService::rememberRelayed(
    uint8_t originNodeId,
    uint8_t alarmType
)
{
    for(
        uint8_t i = 0;
        i < MAX_RELAYED_ALARMS;
        i++
    )
    {
        if(
            relayedAlarms[i].originNodeId == originNodeId &&
            relayedAlarms[i].alarmType == alarmType
        )
        {
            relayedAlarms[i].timestamp = millis();

            return;
        }
    }

    for(
        uint8_t i = 0;
        i < MAX_RELAYED_ALARMS;
        i++
    )
    {
        if(
            relayedAlarms[i].originNodeId == 0
        )
        {
            relayedAlarms[i].originNodeId = originNodeId;
            relayedAlarms[i].alarmType = alarmType;
            relayedAlarms[i].timestamp = millis();

            return;
        }
    }

    //--------------------------------------------------
    // Tabla llena: se sobrescribe el slot más antiguo en
    // vez de descartar el registro nuevo.
    //--------------------------------------------------

    uint8_t oldestIndex = 0;

    for(
        uint8_t i = 1;
        i < MAX_RELAYED_ALARMS;
        i++
    )
    {
        if(
            relayedAlarms[i].timestamp < relayedAlarms[oldestIndex].timestamp
        )
        {
            oldestIndex = i;
        }
    }

    relayedAlarms[oldestIndex].originNodeId = originNodeId;
    relayedAlarms[oldestIndex].alarmType = alarmType;
    relayedAlarms[oldestIndex].timestamp = millis();
}

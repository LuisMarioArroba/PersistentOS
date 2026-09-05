/*
======================================================
[DEPRECADO] PersistentOS2 - firmware mínimo original

Este archivo YA NO SE COMPILA (platformio.ini excluye
node2/ de todos los entornos). PersistentOS2 ahora usa
el mismo src/main.cpp que PersistentOS1 -- mismo
micro-núcleo (BootManager, FRAM, SensorService,
CommunicationService, EnergyManager, AlarmManager),
parametrizado por -DPERSISTENT_OS_NODE_ID=2, con rol
esclavo hacia su primary (ver Config.h:
BLUETOOTH_PRIMARY_IS_MASTER).

Se conserva este archivo solo como referencia histórica
del primer prototipo (firmware standalone, sin FRAM ni
checkpoints), previo a que PersistentOS2 tuviera su
propio módulo FRAM conectado.
======================================================
*/

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
// Energía local (simulación en software, no
// persistente): igual que en PersistentOS1, esto es
// independiente del interruptor físico del tercer
// equipo. Le permite a este nodo reportar un valor de
// energía real (no un placeholder fijo) al nodo remoto.
//--------------------------------------------------

float localEnergy = 100.0f;

uint32_t lastEnergySimTime = 0;


void updateLocalEnergy()
{
    uint32_t now =
        millis();

    if(
        now - lastEnergySimTime < NODE2_ENERGY_SIM_INTERVAL_MS
    )
    {
        return;
    }

    lastEnergySimTime =
        now;

    //--------------------------------------------------
    // Consumo constante con recuperación intermitente
    // (aprox. 1 de cada 3 ciclos), para que la energía
    // local de este nodo también fluctúe con el tiempo,
    // en vez de decaer monótonamente.
    //--------------------------------------------------

    if(
        (now / NODE2_ENERGY_SIM_INTERVAL_MS) % 3 == 0
    )
    {
        localEnergy +=
            NODE2_ENERGY_RECOVERY_AMOUNT;
    }
    else
    {
        localEnergy -=
            NODE2_ENERGY_CONSUMPTION_PER_CYCLE;
    }

    if(
        localEnergy > 100.0f
    )
    {
        localEnergy = 100.0f;
    }

    if(
        localEnergy < 0.0f
    )
    {
        localEnergy = 0.0f;
    }
}


//--------------------------------------------------
// Última energía remota conocida (de PersistentOS1) y
// su tendencia. Este nodo tampoco conoce el
// comportamiento real de la fuente de alimentación de
// ninguno de los dos: solo lo observado en cada DATA
// recibido.
//--------------------------------------------------

bool hasRemoteEnergy = false;

float remoteEnergyValue = 0.0f;

float remoteEnergyTrend = 0.0f;


void observeRemoteEnergy(
    float value
)
{
    if(
        hasRemoteEnergy
    )
    {
        remoteEnergyTrend =
            value - remoteEnergyValue;
    }

    remoteEnergyValue =
        value;

    hasRemoteEnergy =
        true;
}


//--------------------------------------------------
// Ventana de comunicación favorable: heurístico
// probabilístico de primera aproximación (Config.h),
// igual criterio que usa PersistentOS1.
//--------------------------------------------------

bool isCommunicationWindowFavorable()
{
    bool localLow =
        localEnergy < COMM_MIN_FAVORABLE_ENERGY;

    if(
        localLow
    )
    {
        Serial.println(
            "[BT] Window unfavorable: local energy low"
        );

        return false;
    }

    if(
        !hasRemoteEnergy
    )
    {
        return true;
    }

    bool remoteLow =
        remoteEnergyValue < COMM_MIN_FAVORABLE_ENERGY;

    bool remoteFalling =
        remoteEnergyTrend < -COMM_TREND_FALLING_THRESHOLD;

    if(
        remoteLow &&
        remoteFalling
    )
    {
        Serial.println(
            "[BT] Window unfavorable: last known remote energy low and falling"
        );

        return false;
    }

    return true;
}


//--------------------------------------------------
// Envío propio (PersistentOS2 -> PersistentOS1).
//
// Genera y envía periódicamente su propio DATA (con la
// energía local real, no un placeholder), y reintenta
// el mismo paquete si no llega ACK a tiempo.
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
    char payload[32];

    snprintf(
        payload,
        sizeof(payload),
        "ENERGY:%.2f",
        localEnergy
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
            now - lastSendTime >= NODE2_SEND_INTERVAL_MS &&
            isCommunicationWindowFavorable()
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
// el cuerpo de un mensaje DATA|<packetID>|<payload> o
// ALARM|<packetID>|<payload>.
//--------------------------------------------------

void handleIncomingDataLine(
    char* line,

    bool isAlarm
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
            line,
            false
        );

        return;
    }


    //--------------------------------------------------
    // ALARM|<packetID>|<payload> -> alerta de prioridad de
    // PersistentOS1 (mismo protocolo, mismo ACK).
    //--------------------------------------------------

    if(
        strncmp(
            line,
            "ALARM|",
            6
        ) == 0
    )
    {
        handleIncomingDataLine(
            line,
            true
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
            "[BT] Malformed message (missing payload separator)"
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

        sendAck(
            packetId
        );

        return;
    }


    if(
        isAlarm
    )
    {
        Serial.println();

        Serial.println(
            "[ALARM RX] ==============================="
        );

        Serial.print(
            "[ALARM RX] PersistentOS1 reports: "
        );

        Serial.println(
            payload
        );

        Serial.println(
            "[ALARM RX] ==============================="
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
    }


    //--------------------------------------------------
    // Extraer ENERGY:<valor> del payload remoto (si
    // viene) para conocer la última energía observada
    // de PersistentOS1, insumo de isCommunicationWindowFavorable().
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
        observeRemoteEnergy(
            strtof(
                energyTag + 7,
                nullptr
            )
        );
    }


    rememberProcessed(
        packetId
    );


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

    updateLocalEnergy();

    pumpOutgoing();

    delay(
        10
    );
}

#include "kernel/CommunicationManager.h"
#include "config/Config.h"
#include "report/ReportLogger.h"

#include <string.h>

#if USE_BLUETOOTH_COMMUNICATION
#include <BluetoothSerial.h>
#endif

#if USE_WIFI_COMMUNICATION
#include <WiFi.h>
#endif

//====================================================
// Bluetooth instance
//====================================================

#if USE_BLUETOOTH_COMMUNICATION

    BluetoothSerial bluetoothSerial;

#endif

//====================================================
// Constructor
//====================================================

CommunicationManager::CommunicationManager()
{


currentProtocol =
    COMM_NONE;
bluetoothConnected =
    false;
lastReconnectAttempt =
    0;
rxLineLength =
    0;
activePeerSlot =
    PeerSlot::PRIMARY;
peerSlotStartTime =
    0;


}

bool CommunicationManager::sendPriority(
    const uint8_t* data,
    size_t length
)
{

    if(
        data == nullptr ||
        length == 0
    )
    {

        return false;

    }


    Serial.print(
        "[COMM PRIORITY] "
    );


    for(
        size_t i = 0;
        i < length;
        i++
    )
    {

        Serial.write(
            data[i]
        );

    }


    Serial.println();


    //--------------------------------------------------
    // Current backend
    //--------------------------------------------------

    return send(
        data,
        length
    );

}

//====================================================
// Begin
//====================================================

bool CommunicationManager::begin()
{


//--------------------------------------------------
// Bluetooth
//--------------------------------------------------

#if USE_BLUETOOTH_COMMUNICATION

if(
    initializeBluetooth()
)
{

    currentProtocol =
        COMM_BLUETOOTH;


    Serial.println(
        "[COMM] Bluetooth selected"
    );


    return true;

}

#endif


//--------------------------------------------------
// WiFi
//--------------------------------------------------

#if USE_WIFI_COMMUNICATION

if(
    initializeWiFi()
)
{

    currentProtocol =
        COMM_WIFI;


    Serial.println(
        "[COMM] WiFi selected"
    );


    return true;

}

#endif


//--------------------------------------------------
// Serial
//--------------------------------------------------

#if USE_SERIAL_COMMUNICATION

if(
    initializeSerial()
)
{

    currentProtocol =
        COMM_SERIAL;


    Serial.println(
        "[COMM] Serial selected"
    );


    return true;

}

#endif


//--------------------------------------------------
// No backend
//--------------------------------------------------

currentProtocol =
    COMM_NONE;


Serial.println(
    "[COMM] No communication backend"
);


return false;


}

//====================================================
// Get protocol
//====================================================

CommunicationProtocol
CommunicationManager::getProtocol() const
{


return currentProtocol;


}

//====================================================
// Is connected
//====================================================

bool CommunicationManager::isConnected() const
{


switch(
    currentProtocol
)
{

    case COMM_BLUETOOTH:

    #if USE_BLUETOOTH_COMMUNICATION

        return
            bluetoothSerial.hasClient();

    #else

        return false;

    #endif


    case COMM_WIFI:

        #if USE_WIFI_COMMUNICATION

        return true;

        #else

        return false;

        #endif


    case COMM_SERIAL:

        #if USE_SERIAL_COMMUNICATION

        return true;

        #else

        return false;

        #endif


    default:

        return false;

}


}

//====================================================
// Send
//====================================================

bool CommunicationManager::send(


const uint8_t* data,

size_t length


)
{


if(
    data == nullptr ||
    length == 0
)
{

    return false;

}


switch(
    currentProtocol
)
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
// Available
//====================================================

bool CommunicationManager::available()
{


switch(
    currentProtocol
)
{

    case COMM_BLUETOOTH:

        #if USE_BLUETOOTH_COMMUNICATION

        return
            bluetoothSerial.available() > 0;

        #else

        return false;

        #endif


    case COMM_WIFI:

        return false;


    case COMM_SERIAL:

        return false;


    default:

        return false;

}


}

//====================================================
// Receive
//====================================================

size_t CommunicationManager::receive(


uint8_t* buffer,

size_t maxLength


)
{


if(
    buffer == nullptr ||
    maxLength == 0
)
{

    return 0;

}


switch(
    currentProtocol
)
{

    case COMM_BLUETOOTH:

        return receiveBluetooth(
            buffer,
            maxLength
        );


    case COMM_WIFI:

        return receiveWiFi(
            buffer,
            maxLength
        );


    default:

        return 0;

}


}

//====================================================
// Initialize Serial
//====================================================

bool CommunicationManager::initializeSerial()
{


#if USE_SERIAL_COMMUNICATION

/*
    Serial solamente para
    monitoreo/debug.

    No representa el canal
    principal de comunicación.
*/

return true;

#else

return false;

#endif


}

//====================================================
// Initialize Bluetooth
//====================================================

bool CommunicationManager::initializeBluetooth()
{


#if USE_BLUETOOTH_COMMUNICATION

Serial.println(
    "[COMM] Initializing Bluetooth..."
);


//--------------------------------------------------
// Reintentar la inicialización indefinidamente en
// lugar de rendirse tras un solo intento. Solo cubre
// el arranque del stack Bluetooth (advertising SPP);
// la espera de un cliente real se maneja de forma
// asíncrona en updateConnectionState().
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
        "[COMM] Bluetooth begin attempt #"
    );

    Serial.println(
        attempt
    );

    result =
        bluetoothSerial.begin(
            BLUETOOTH_DEVICE_NAME,
            BLUETOOTH_PRIMARY_IS_MASTER    // maestro: busca y se
                                            // conecta. Esclavo: se
                                            // anuncia y espera.
        );

    if(
        !result
    )
    {

        Serial.println(
            "[COMM] Bluetooth initialization failed, retrying..."
        );

        bluetoothSerial.end();

        delay(
            BLUETOOTH_INIT_RETRY_DELAY_MS
        );

    }

}


Serial.println(
    "[COMM] Bluetooth initialized"
);


Serial.print(
    "[COMM] Device name: "
);


Serial.println(
    BLUETOOTH_DEVICE_NAME
);


#if BLUETOOTH_PRIMARY_IS_MASTER

//--------------------------------------------------
// Rol maestro: búsqueda/conexión activa al primary.
// connect() hace descubrimiento SDP por nombre y
// bloquea unos segundos por intento; se reintenta
// indefinidamente hasta lograr la conexión real,
// en lugar de intentarlo una sola vez.
//--------------------------------------------------

Serial.print(
    "[COMM] Searching for "
);

Serial.print(
    BLUETOOTH_REMOTE_DEVICE_NAME
);

Serial.println(
    "..."
);


bool connected =
    false;

uint32_t connectAttempt =
    0;

while(
    !connected
)
{

    connectAttempt++;

    Serial.print(
        "[COMM] Bluetooth connect attempt #"
    );

    Serial.println(
        connectAttempt
    );

    connected =
        bluetoothSerial.connect(
            BLUETOOTH_REMOTE_DEVICE_NAME
        );

    if(
        !connected
    )
    {

        Serial.println(
            "[COMM] Connection to remote node failed, retrying..."
        );

        delay(
            BLUETOOTH_CONNECT_RETRY_DELAY_MS
        );

    }

}


Serial.print(
    "[COMM] Connected to "
);

Serial.println(
    BLUETOOTH_REMOTE_DEVICE_NAME
);

#else

//--------------------------------------------------
// Rol esclavo: no se llama connect(), solo se queda
// anunciándose. BluetoothSerial en este modo acepta a
// CUALQUIER maestro que lo encuentre por nombre -- no
// solo al configurado como "primary" en Config.h. La
// identidad real de quien se conectó se descubre después,
// a nivel de aplicación, por el campo NODE:<id> de cada
// mensaje (ver CommunicationService::handleIncomingData),
// no aquí. "primary" solo documenta la relación esperada
// por defecto, no restringe quién puede enlazarse.
//--------------------------------------------------

Serial.print(
    "[COMM] Advertising as "
);

Serial.print(
    BLUETOOTH_DEVICE_NAME
);

Serial.println(
    ", waiting for any master to connect..."
);

#endif


return true;

#else

return false;

#endif


}

//====================================================
// Initialize WiFi
//====================================================

bool CommunicationManager::initializeWiFi()
{


#if USE_WIFI_COMMUNICATION

Serial.println(
    "[COMM] Initializing WiFi..."
);


WiFi.mode(
    WIFI_AP
);


bool result =
    WiFi.softAP(
        WIFI_SSID,
        WIFI_PASSWORD
    );


if(
    !result
)
{

    Serial.println(
        "[COMM] WiFi initialization failed"
    );


    return false;

}


Serial.println(
    "[COMM] WiFi Access Point started"
);


Serial.print(
    "[COMM] SSID: "
);


Serial.println(
    WIFI_SSID
);


Serial.print(
    "[COMM] IP: "
);


Serial.println(
    WiFi.softAPIP()
);


return true;

#else

return false;

#endif


}

//====================================================
// Send Serial
//====================================================

bool CommunicationManager::sendSerial(


const uint8_t* data,

size_t length


)
{


#if USE_SERIAL_COMMUNICATION

Serial.print(
    "[COMM SERIAL] "
);


for(
    size_t i = 0;
    i < length;
    i++
)
{

    Serial.write(
        data[i]
    );

}


Serial.println();


return true;

#else

return false;

#endif


}

//====================================================
// Send Bluetooth
//====================================================

bool CommunicationManager::sendBluetooth(
    const uint8_t* data,
    size_t length
)
{

#if USE_BLUETOOTH_COMMUNICATION

    bool connected =
        bluetoothSerial.hasClient();


    Serial.print(
        "[COMM BT] hasClient(): "
    );

    Serial.println(
        connected
            ? "TRUE"
            : "FALSE"
    );


    if(
        !connected
    )
    {

        Serial.println(
            "[COMM BT] No SPP client connected"
        );

        return false;

    }


    size_t written =
        bluetoothSerial.write(
            data,
            length
        );


    bluetoothSerial.flush();


    Serial.print(
        "[COMM BT] Bytes written: "
    );

    Serial.print(
        written
    );

    Serial.print(
        "/"
    );

    Serial.println(
        length
    );


    return written == length;

#else

    return false;

#endif

}

//====================================================
// Send WiFi
//====================================================

bool CommunicationManager::sendWiFi(


const uint8_t* data,

size_t length


)
{


#if USE_WIFI_COMMUNICATION

/*
    Reservado para:

    TCP
    UDP
    HTTP
    MQTT
*/

return false;

#else

return false;

#endif


}

//====================================================
// Receive Bluetooth
//====================================================

bool CommunicationManager::receiveBluetooth(


uint8_t* buffer,

size_t maxLength


)
{


#if USE_BLUETOOTH_COMMUNICATION

if(
    buffer == nullptr ||
    maxLength == 0
)
{

    return false;

}


if(
    !bluetoothSerial.available()
)
{

    return false;

}


size_t received =
    bluetoothSerial.readBytes(
        buffer,
        maxLength
    );


return
    received > 0;

#else

return false;

#endif


}

//====================================================
// Receive Line (protocolo DATA/ACK)
//====================================================

bool CommunicationManager::receiveLine(

    char* outBuffer,

    size_t maxLength

)
{

if(
    outBuffer == nullptr ||
    maxLength == 0
)
{

    return false;

}


#if USE_BLUETOOTH_COMMUNICATION

if(
    currentProtocol !=
    COMM_BLUETOOTH
)
{

    return false;

}


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


    //--------------------------------------------------
    // End of message
    //--------------------------------------------------

    if(
        c == '\n'
    )
    {

        //--------------------------------------------------
        // Trim trailing '\r' if present (CRLF)
        //--------------------------------------------------

        if(
            rxLineLength > 0 &&
            rxLineBuffer[rxLineLength - 1] == '\r'
        )
        {

            rxLineLength--;

        }


        rxLineBuffer[rxLineLength] =
            '\0';


        size_t copyLength =
            rxLineLength < (maxLength - 1)
                ? rxLineLength
                : (maxLength - 1);


        memcpy(
            outBuffer,
            rxLineBuffer,
            copyLength
        );

        outBuffer[copyLength] =
            '\0';


        rxLineLength =
            0;


        return true;

    }


    //--------------------------------------------------
    // Accumulate byte
    //--------------------------------------------------

    if(
        rxLineLength < (MAX_LINE_SIZE - 1)
    )
    {

        rxLineBuffer[rxLineLength] =
            (char) c;

        rxLineLength++;

    }
    else
    {

        //--------------------------------------------------
        // Overflow protection: discard malformed/oversized
        // line and start over.
        //--------------------------------------------------

        Serial.println(
            "[COMM BT] Line buffer overflow, discarding"
        );

        rxLineLength =
            0;

    }

}


return false;

#else

return false;

#endif

}

//====================================================
// Receive WiFi
//====================================================

bool CommunicationManager::receiveWiFi(


uint8_t* buffer,

size_t maxLength


)
{


#if USE_WIFI_COMMUNICATION

/*
    Implementación posterior
    de recepción WiFi.
*/

return false;

#else

return false;

#endif


}

//====================================================
// Print connection status
//====================================================

void CommunicationManager::printConnectionStatus()
{

    Serial.print(
        "[COMM] Protocol: "
    );


    switch(
        currentProtocol
    )
    {

        case COMM_BLUETOOTH:

            Serial.print(
                "BLUETOOTH"
            );


#if USE_BLUETOOTH_COMMUNICATION

            Serial.print(
                " | Client: "
            );


            if(
                bluetoothSerial.hasClient()
            )
            {

                Serial.println(
                    "CONNECTED"
                );

            }
            else
            {

                Serial.println(
                    "NOT CONNECTED"
                );

            }

#else

            Serial.println();

#endif

            break;


        case COMM_WIFI:

            Serial.println(
                "WIFI"
            );

            break;


        case COMM_SERIAL:

            Serial.println(
                "SERIAL"
            );

            break;


        default:

            Serial.println(
                "NONE"
            );

            break;

    }

}
void CommunicationManager::updateConnectionState()
{

#if USE_BLUETOOTH_COMMUNICATION

    if(
        currentProtocol !=
        COMM_BLUETOOTH
    )
    {
        return;
    }


    bool connected =
        bluetoothSerial.hasClient();


    if(
        connected &&
        !bluetoothConnected
    )
    {

        bluetoothConnected =
            true;


        Serial.println(
            "[COMM BT] CLIENT CONNECTED"
        );

        ReportLogger::event(
            "BT_CONNECTED",
            0,
            0
        );

    }


    if(
        !connected &&
        bluetoothConnected
    )
    {

        bluetoothConnected =
            false;


        Serial.println(
            "[COMM BT] CLIENT DISCONNECTED"
        );

        ReportLogger::event(
            "BT_DISCONNECTED",
            0,
            0
        );

    }


    //--------------------------------------------------
    // Sigue sin cliente: reintenta la conexión activa cada
    // BLUETOOTH_RECONNECT_COOLDOWN_MS en vez de quedarse
    // esperando pasivamente. Esto es lo que faltaba para el
    // caso en que es PersistentOS2 (el esclavo) el que se
    // reinicia: al volver, se vuelve a anunciar y a esperar
    // normalmente, pero solo PersistentOS1 (el maestro, el
    // único que hace connect()) puede reiniciar el enlace
    // desde su lado.
    //--------------------------------------------------

#if BLUETOOTH_PRIMARY_IS_MASTER

    if(
        !connected &&
        activePeerSlot == PeerSlot::PRIMARY
    )
    {

        uint32_t now =
            millis();

        if(
            now - lastReconnectAttempt >= BLUETOOTH_RECONNECT_COOLDOWN_MS
        )
        {

            lastReconnectAttempt =
                now;

            attemptReconnectBluetooth();

        }

    }

#endif


    //--------------------------------------------------
    // Rotación al peer secundario opcional. No-op si
    // BLUETOOTH_SECONDARY_DEVICE_NAME está vacío.
    //--------------------------------------------------

    pollPeerRotation();

#endif

}


//====================================================
// Attempt reconnect (Bluetooth)
//====================================================

void CommunicationManager::attemptReconnectBluetooth()
{

#if USE_BLUETOOTH_COMMUNICATION

    Serial.print(
        "[COMM BT] Searching for "
    );

    Serial.print(
        BLUETOOTH_REMOTE_DEVICE_NAME
    );

    Serial.println(
        " (reconnect)..."
    );


    bool connected =
        bluetoothSerial.connect(
            BLUETOOTH_REMOTE_DEVICE_NAME
        );


    if(
        connected
    )
    {

        bluetoothConnected =
            true;


        Serial.println(
            "[COMM BT] Reconnected"
        );

        ReportLogger::event(
            "BT_RECONNECTED",
            0,
            0
        );

    }
    else
    {

        Serial.println(
            "[COMM BT] Reconnect attempt failed, will retry"
        );

    }

#endif

}


//====================================================
// Peer rotation (secondary opcional)
//====================================================
//
// No es una "cadena" simultánea (BluetoothSerial clásico
// solo sostiene una sesión SPP a la vez): es una rotación
// -- se desconecta del primary, intenta UNA vez encontrar
// al secondary, y si lo logra se queda comunicando con él
// un rato antes de volver al primary. Si no lo encuentra,
// vuelve al primary de inmediato. El resto del sistema
// (CommunicationService) no distingue con quién está
// hablando: solo usa lo que esté conectado en bluetoothSerial
// en cada momento.
//====================================================

void CommunicationManager::pollPeerRotation()
{

#if USE_BLUETOOTH_COMMUNICATION

    if(
        strlen(BLUETOOTH_SECONDARY_DEVICE_NAME) == 0
    )
    {
        //--------------------------------------------------
        // Deshabilitado (comportamiento por defecto sin un
        // tercer nodo real): no toca nada del enlace hacia
        // el primary.
        //--------------------------------------------------

        return;
    }


    uint32_t now =
        millis();


    if(
        activePeerSlot == PeerSlot::PRIMARY
    )
    {
        //--------------------------------------------------
        // Solo tiene sentido rotar lejos del primary si
        // realmente está conectado a él ahora mismo; si
        // todavía lo está buscando, no hay nada de qué
        // "salir".
        //--------------------------------------------------

        if(
            !bluetoothConnected
        )
        {
            peerSlotStartTime =
                now;

            return;
        }


        if(
            now - peerSlotStartTime < BLUETOOTH_PRIMARY_WINDOW_MS
        )
        {
            return;
        }


        Serial.print(
            "[COMM BT] Leaving primary, searching for optional secondary: "
        );

        Serial.println(
            BLUETOOTH_SECONDARY_DEVICE_NAME
        );

        bluetoothSerial.disconnect();


#if !BLUETOOTH_PRIMARY_IS_MASTER

        //--------------------------------------------------
        // Un nodo esclavo hacia su primary necesita
        // volverse maestro temporalmente para poder marcar
        // al secondary (nadie más lo va a llamar por él).
        //--------------------------------------------------

        bluetoothSerial.end();

        bluetoothSerial.begin(
            BLUETOOTH_DEVICE_NAME,
            true
        );

#endif


        bool found =
            bluetoothSerial.connect(
                BLUETOOTH_SECONDARY_DEVICE_NAME
            );


        if(
            found
        )
        {

            Serial.print(
                "[COMM BT] Connected to secondary: "
            );

            Serial.println(
                BLUETOOTH_SECONDARY_DEVICE_NAME
            );

            ReportLogger::event(
                "BT_SECONDARY_CONNECTED",
                0,
                0
            );

            activePeerSlot =
                PeerSlot::SECONDARY;

            bluetoothConnected =
                true;

            peerSlotStartTime =
                now;

            return;

        }


        Serial.println(
            "[COMM BT] Secondary not found, returning to primary"
        );


#if !BLUETOOTH_PRIMARY_IS_MASTER

        bluetoothSerial.end();

        bluetoothSerial.begin(
            BLUETOOTH_DEVICE_NAME,
            false
        );

#endif


        activePeerSlot =
            PeerSlot::PRIMARY;

        bluetoothConnected =
            false;

        peerSlotStartTime =
            now;


        //--------------------------------------------------
        // Rol maestro: forzar un reintento inmediato hacia
        // el primary en el próximo tick, en vez de esperar
        // el cooldown normal de reconexión.
        //--------------------------------------------------

        lastReconnectAttempt =
            0;

        return;

    }


    //--------------------------------------------------
    // activePeerSlot == SECONDARY
    //--------------------------------------------------

    if(
        !bluetoothConnected ||
        now - peerSlotStartTime >= BLUETOOTH_SECONDARY_WINDOW_MS
    )
    {

        Serial.println(
            "[COMM BT] Leaving secondary, returning to primary"
        );

        bluetoothSerial.disconnect();


#if !BLUETOOTH_PRIMARY_IS_MASTER

        bluetoothSerial.end();

        bluetoothSerial.begin(
            BLUETOOTH_DEVICE_NAME,
            false
        );

#endif


        activePeerSlot =
            PeerSlot::PRIMARY;

        bluetoothConnected =
            false;

        peerSlotStartTime =
            now;

        lastReconnectAttempt =
            0;

    }

#endif

}
#ifndef CONFIG_H
#define CONFIG_H


//====================================================
// GPIO Configuration
//====================================================

#define LED_PIN             2
#define ONE_WIRE_BUS        4
#define TEST_BUTTON_PIN     15


//====================================================
// Scheduler Configuration
//====================================================

#define MAX_TASKS           10
#define SYSTEM_TICK_MS      100


//====================================================
// Persistent Memory Configuration
//====================================================

#define SENSOR_BUFFER_SIZE      32
#define ENERGY_HISTORY_SIZE     50


//====================================================
// Communication Configuration
//====================================================

// Serial queda solamente para monitoreo
#define USE_SERIAL_COMMUNICATION       true

// Bluetooth clásico SPP
#define USE_BLUETOOTH_COMMUNICATION    true

// WiFi preparado para una etapa posterior
#define USE_WIFI_COMMUNICATION         false


//====================================================
// Bluetooth Configuration
//====================================================

// El ID de nodo se define normalmente desde platformio.ini
// (build_flags = -DPERSISTENT_OS_NODE_ID=N) para cada entorno.
// Si no se define, se asume nodo 1 por defecto.
#ifndef PERSISTENT_OS_NODE_ID
#define PERSISTENT_OS_NODE_ID          1
#endif

#define PERSISTENT_OS_STR_HELPER(x)    #x
#define PERSISTENT_OS_STR(x)           PERSISTENT_OS_STR_HELPER(x)

// Genera "PersistentOS1", "PersistentOS2", etc.
#define BLUETOOTH_DEVICE_NAME          "PersistentOS" PERSISTENT_OS_STR(PERSISTENT_OS_NODE_ID)

// Tiempo de espera entre reintentos de inicialización de Bluetooth
// cuando bluetoothSerial.begin() falla. La inicialización se
// reintenta indefinidamente hasta tener éxito.
#define BLUETOOTH_INIT_RETRY_DELAY_MS  1000

//====================================================
// Rol y peers por nodo (mismo firmware/kernel para
// todos, parametrizado por PERSISTENT_OS_NODE_ID en
// tiempo de compilación).
//
// - PRIMARY es obligatorio: el nodo no funciona sin
//   poder enlazarse con él.
// - SECONDARY es opcional: el nodo intenta encontrarlo
//   periódicamente, pero si no aparece, sigue
//   funcionando normalmente con el primary.
//
// Node 1 y Node 3 son maestros (buscan y se conectan
// activamente a su primary); Node 2 es esclavo (espera
// a que Node 1 lo conecte). Node 3 no es "solo un
// esclavo" -- tiene la misma capacidad de iniciativa
// que Node 1, solo que apunta a un primary distinto.
//====================================================

#if PERSISTENT_OS_NODE_ID == 1

    #define BLUETOOTH_PRIMARY_IS_MASTER    true

    #define BLUETOOTH_REMOTE_DEVICE_NAME   "PersistentOS2"

    #ifndef BLUETOOTH_SECONDARY_DEVICE_NAME
        #define BLUETOOTH_SECONDARY_DEVICE_NAME "PersistentOS3"
    #endif

#elif PERSISTENT_OS_NODE_ID == 3

    // Igual de capaz que Node 1: maestro hacia su propio
    // primary, con Node 1 como secondary opcional.
    #define BLUETOOTH_PRIMARY_IS_MASTER    true

    #define BLUETOOTH_REMOTE_DEVICE_NAME   "PersistentOS2"

    #ifndef BLUETOOTH_SECONDARY_DEVICE_NAME
        #define BLUETOOTH_SECONDARY_DEVICE_NAME "PersistentOS1"
    #endif

#else

    // Node 2: esclavo hacia su primary (Node 1), con
    // Node 3 como secondary opcional.
    #define BLUETOOTH_PRIMARY_IS_MASTER    false

    #define BLUETOOTH_REMOTE_DEVICE_NAME   "PersistentOS1"

    #ifndef BLUETOOTH_SECONDARY_DEVICE_NAME
        #define BLUETOOTH_SECONDARY_DEVICE_NAME "PersistentOS3"
    #endif

#endif

// Vacío = deshabilitado. Por defecto está ACTIVO para
// los tres nodos: cada uno busca periódicamente a su
// secondary aunque no exista físicamente, y si no lo
// encuentra vuelve de inmediato al primary obligatorio
// -- ver pollPeerRotation() en CommunicationManager.cpp.
// Se puede desactivar con -DBLUETOOTH_SECONDARY_DEVICE_NAME='""'
// como build_flag si se necesita para una campaña
// experimental que no quiera esas desconexiones periódicas.

// Cuánto tiempo se queda conectado al primary antes de
// intentar encontrar al secondary (si está habilitado).
#ifndef BLUETOOTH_PRIMARY_WINDOW_MS
    #define BLUETOOTH_PRIMARY_WINDOW_MS        30000
#endif

// Cuánto tiempo se queda conectado al secondary (si lo
// encuentra) antes de volver al primary.
#ifndef BLUETOOTH_SECONDARY_WINDOW_MS
    #define BLUETOOTH_SECONDARY_WINDOW_MS      15000
#endif

// Tiempo de espera entre reintentos de conexión (connect()) al
// nodo remoto cuando la búsqueda/conexión falla. Se reintenta
// indefinidamente hasta que la conexión se establezca.
#define BLUETOOTH_CONNECT_RETRY_DELAY_MS  1000

// Tiempo mínimo entre reintentos de connect() cuando el
// enlace se cae DESPUÉS del arranque (por ejemplo, si
// PersistentOS2 se reinicia). connect() bloquea unos
// segundos por intento, así que este cooldown evita
// llamarlo en cada tick del loop mientras no hay cliente.
#define BLUETOOTH_RECONNECT_COOLDOWN_MS 5000

#define BLUETOOTH_PIN                  "1234"

// PersistentOS2 (nodo receptor): cada cuánto genera y envía su
// propio dato hacia PersistentOS1, y cuánto espera el ACK antes
// de reintentar el mismo paquete. Placeholder hasta que este
// nodo tenga su propio sensor/monitor de energía.
#define NODE2_SEND_INTERVAL_MS         5000
#define NODE2_ACK_TIMEOUT_MS           3000

// PersistentOS1: cuánto tiempo esperar el ACK de un DATA propio
// antes de reintentar el mismo packetID. Cubre dos casos con el
// mismo mecanismo:
//   1. Ejecución normal: el ACK se demora o se pierde en el aire.
//   2. Resume tras un reboot: el nodo llega directo a WAIT_ACK
//      (no hay CREATE/PREPARE/SEND en el resume) sin saber si el
//      DATA original llegó a salir o si el ACK se perdió con la
//      reconexión Bluetooth. millis() arranca en 0 tras el reboot,
//      así que "now - packet->timestamp" (aritmética sin signo)
//      da un valor enorme de inmediato y dispara el reintento en
//      el primer tick, en vez de esperar indefinidamente.
#define COMM_ACK_TIMEOUT_MS             5000


//====================================================
// WiFi Configuration
//====================================================

// Se utilizará posteriormente
#define WIFI_SSID                      "Luilly@"

#define WIFI_PASSWORD                  "no_me_acuerdo"

/*
======================================================
Simulation Configuration
======================================================
*/

#define SIMULATION_INITIAL_ENERGY      100.0f

#define SIMULATION_MIN_ENERGY          0.0f
#define SIMULATION_MAX_ENERGY          100.0f

#define SIMULATION_STEP                1.0f

#define SIMULATION_UPDATE_MS           500


/*
======================================================
Alarm Configuration
======================================================
*/

// Umbrales de alerta de temperatura: por debajo de MIN o por
// encima de MAX se dispara una alarma de prioridad.
#define ALARM_TEMPERATURE_MIN          20.0f
#define ALARM_TEMPERATURE_MAX          35.0f

#define ALARM_PRIORITY                 255


/*
======================================================
Communication window (predicción de periodos de
comunicación)
======================================================

Ningún nodo conoce el comportamiento real de su fuente
de alimentación (el tercer equipo que la controla), así
que esta estimación se apoya únicamente en los valores
de energía observados: el propio (EnergyManager) y el
del nodo remoto (recibido en cada DATA/ACK). No es una
predicción exacta, es un heurístico probabilístico de
primera aproximación.
*/

// Por debajo de este nivel, un nodo se considera "bajo
// de energía" para efectos de decidir si conviene
// intentar una comunicación de rutina.
#define COMM_MIN_FAVORABLE_ENERGY       25.0f

// Cambio mínimo entre muestras consecutivas (mismas
// unidades que EnergyPredictionManager::trendThreshold)
// para considerar que la energía está cayendo de forma
// sostenida, no solo con ruido.
#define COMM_TREND_FALLING_THRESHOLD    0.10f

// PersistentOS2: simulación local de energía (software),
// independiente del interruptor físico del tercer equipo.
// Le permite reportar su propio valor de energía como
// PersistentOS1, en lugar de un valor de prueba fijo.
#define NODE2_ENERGY_CONSUMPTION_PER_CYCLE   0.4f
#define NODE2_ENERGY_RECOVERY_AMOUNT         0.6f
#define NODE2_ENERGY_SIM_INTERVAL_MS         500


#endif
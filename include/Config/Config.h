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

// Nombre del nodo receptor al que PersistentOS1 se conecta como
// maestro Bluetooth (SPP). PersistentOS2 se queda como esclavo,
// esperando esa conexión.
#define BLUETOOTH_REMOTE_DEVICE_NAME   "PersistentOS2"

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

#define ALARM_TEMPERATURE_MIN          10.0f
#define ALARM_TEMPERATURE_MAX          40.0f

#define ALARM_PRIORITY                 255


#endif
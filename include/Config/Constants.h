#ifndef CONSTANTS_H
#define CONSTANTS_H

#define MAGIC_NUMBER               0xDEADBEEF
#define MAX_TASKS                  10
#define SYSTEM_VERSION             1

constexpr uint8_t MAX_COMMUNICATION_RETRIES = 3;
constexpr uint16_t MAX_PACKET_SIZE = 64;
constexpr uint8_t COMMUNICATION_BUFFER_SIZE = 20;

// Tamaño máximo de una línea de protocolo completa, ej:
// "DATA|4294967295|" (17) + payload (MAX_PACKET_SIZE) + margen
constexpr uint16_t MAX_LINE_SIZE = 96;

#endif
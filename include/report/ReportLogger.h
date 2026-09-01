#ifndef REPORT_LOGGER_H
#define REPORT_LOGGER_H

#include <Arduino.h>

//====================================================
// ReportLogger
//
// Emite una línea CSV por Serial en cada evento
// relevante para las gráficas de la Sección de
// Resultados del paper. Formato fijo, una línea por
// evento, para que un script externo (tools/plot_report.py)
// pueda parsear la captura del puerto serie sin ambigüedad:
//
//   RPT,<millis>,<scenario_id>,<scenario_name>,<event>,<packetId>,<value>
//
// El escenario activo se fija manualmente por Serial
// (comandos '1'..'7', ver main_prueba_reporte.cpp) porque
// dos de los siete escenarios del paper (nodo remoto
// apagado, confirmación demorada) dependen del interruptor
// de energía físico y no son detectables desde el firmware.
//====================================================

class ReportLogger
{

public:

    static void begin();

    static void enable();

    static bool isEnabled();

    static void setScenario(
        uint8_t scenarioId,
        const char* scenarioName
    );

    //--------------------------------------------------
    // Llamar una vez en setup() cuando
    // bootManager.wasRecovery() es true. El siguiente
    // evento "CONFIRM" reportado calcula y emite
    // automáticamente RECOVERY_LATENCY_MS.
    //--------------------------------------------------

    static void beginRecoveryTracking();

    static void event(
        const char* eventName,
        uint32_t packetId,
        long value
    );

    static uint8_t getScenario();

private:

    static void printLine(
        const char* eventName,
        uint32_t packetId,
        long value
    );

    static bool enabled;

    static bool recoveryPending;

    static uint32_t recoveryStartMillis;

    static uint8_t currentScenarioId;

    static char currentScenarioName[24];

};

#endif

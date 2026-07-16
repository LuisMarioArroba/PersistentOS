#ifndef TASK_H
#define TASK_H

#include <Arduino.h>

typedef void (*TaskFunction)();

enum TaskState
{
    READY,
    RUNNING,
    WAITING,
    SUSPENDED
};

struct Task
{
    // Identificación
    const char* name;

    // Función asociada
    TaskFunction run;

    // Estado actual
    TaskState state;

    // Configuración temporal
    uint32_t period;

    // Tiempo de la última ejecución
    uint32_t lastExecution;

    // Estadísticas
    uint32_t executions;
};

#endif
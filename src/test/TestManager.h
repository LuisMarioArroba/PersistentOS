#ifndef TEST_MANAGER_H
#define TEST_MANAGER_H


#include <Arduino.h>


#include "kernel/ExecutionCheckpoint.h"
#include "services/FailureManager.h"
#include "kernel/ExecutionSteps.h"
#include "kernel/TaskID.h"
#include "kernel/EnergyManager.h"



enum TestScenario
{

    TEST_NORMAL = 0,

    TEST_FAIL_CREATE,

    TEST_FAIL_PREPARE,

    TEST_FAIL_SEND,

    TEST_FAIL_WAIT_ACK,

    TEST_FAIL_CONFIRM,

    TEST_TOTAL

};

enum EnergySimulationType
{

    ENERGY_SIM_NONE = 0,

    ENERGY_SIM_LOGARITHMIC,

    ENERGY_SIM_STANDARD_DEVIATION,

    ENERGY_SIM_INVERSE,

    ENERGY_SIM_LINEAR,

    ENERGY_SIM_TOTAL

};


class TestManager
{

private:


    ExecutionCheckpoint* checkpointManager;


    FailureManager* failureManager;


    TestScenario currentScenario;


    bool failureInjected;


    bool lastButtonState;

    uint8_t executionCounter;

    EnergyManager* energyManager;

    EnergySimulationType energySimulation;

    bool energyMenuActive;

    bool energySimulationActive;

    unsigned long energySimulationStart;

    unsigned long energySimulationDuration;

public:


    TestManager();



    void begin(
        ExecutionCheckpoint* checkpointPtr,
        FailureManager* failurePtr,
        EnergyManager* energyPtr
    );


    void execute();


    //--------------------------------------------------
    // Fuerza un escenario específico (no el siguiente en
    // orden), para automatizar el ciclo de pruebas desde
    // fuera (ver main_prueba_reporte.cpp) en vez de
    // depender únicamente del botón físico.
    //--------------------------------------------------

    void forceScenario(
        TestScenario scenario
    );



private:


    bool buttonPressed();


    void nextScenario();

    void processSerial();

    void printEnergyMenu();

    void startEnergySimulation(
        EnergySimulationType type
    );

    void executeEnergySimulation();

    float calculateEnergy(
        float progress
    );

    void stopEnergySimulation();

        void injectFailure(
        uint8_t checkpoint
    );

    void printScenario();
};



#endif
#ifndef ENERGY_MANAGER_H
#define ENERGY_MANAGER_H

#include <Arduino.h>


//====================================================
// Energy states
//====================================================

enum EnergyState
{

    ENERGY_NORMAL = 0,

    ENERGY_LOW,

    ENERGY_CRITICAL,

    ENERGY_OFF

};



//====================================================
// Energy Manager
//====================================================

class EnergyManager
{

private:

    //================================================
    // Energy level
    //================================================

    float energyLevel;


    //================================================
    // Current state
    //================================================

    EnergyState currentState;

    EnergyState previousState;


    //================================================
    // Thresholds
    //================================================

    float lowThreshold;

    float criticalThreshold;

    float offThreshold;


    //================================================
    // State change
    //================================================

    bool stateChanged;


    //================================================
    // Simulation
    //================================================

    float consumptionPerCycle;

    float recoveryAmount;

    unsigned long lastSimulation;

    unsigned long simulationInterval;

    bool simulationMode;

    bool simulateShutdown;

    //================================================
    // Internal
    //================================================

    void updateState();


public:

    //================================================
    // Constructor
    //================================================

    EnergyManager();


    //================================================
    // Initialization
    //================================================

    void begin(
        float initialEnergy = 100.0
    );


    //================================================
    // Main execution
    //================================================

    void execute();


    //================================================
    // State update
    //================================================

    void update();


    //================================================
    // Energy control
    //================================================

    void setEnergy(
        float value
    );


    void consume(
        float amount
    );


    void restore(
        float amount
    );

    void setSimulationMode(
        bool enabled
    );

    void setSimulateShutdown(
        bool enabled
    );

    bool isSimulationMode() const;

    bool isSimulateShutdown() const;


    //================================================
    // State control
    //================================================

    void setEnergyState(
        EnergyState state
    );


    //================================================
    // Getters
    //================================================

    float getEnergy() const;


    EnergyState getState() const;


    EnergyState getPreviousState() const;


    bool isNormal() const;


    bool isLow() const;


    bool isCritical() const;


    bool isOff() const;


    bool hasChanged() const;


    //================================================
    // Information
    //================================================

    void printStatus();

};

#endif
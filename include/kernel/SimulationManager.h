#ifndef SIMULATION_MANAGER_H
#define SIMULATION_MANAGER_H


#include <Arduino.h>


enum SimulationModel
{

    SIM_NONE = 0,

    SIM_LINEAR,

    SIM_LOGARITHMIC,

    SIM_STANDARD_DEVIATION,

    SIM_INVERSE_X,

    SIM_EXPONENTIAL,

    SIM_TOTAL

};


enum SimulationState
{

    SIM_STATE_IDLE = 0,

    SIM_STATE_RUNNING,

    SIM_STATE_OFF

};


class SimulationManager
{

private:

    SimulationModel currentModel;

    SimulationState currentState;


    float energy;

    float initialEnergy;

    float simulationTime;

    float standardDeviation;


    unsigned long lastUpdate;


    bool menuPrinted;


private:

    float calculateValue(
        float x
    );


    void printMenu();


    void printModel();


public:

    SimulationManager();


    void begin();


    void execute();


    void serialMenu();


    void selectModel(
        SimulationModel model
    );


    void start();


    void stop();


    void simulateShutdown();


    void simulateStartup();


    void reset();


    bool isRunning() const;


    bool isOff() const;


    float getEnergy() const;


    float getSimulationTime() const;


    SimulationModel getModel() const;


    SimulationState getState() const;

};


#endif
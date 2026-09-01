#include "kernel/EnergyManager.h"


//====================================================
// Constructor
//====================================================

EnergyManager::EnergyManager()
{

    energyLevel =
        100.0;


    currentState =
        ENERGY_NORMAL;


    previousState =
        ENERGY_NORMAL;


    lowThreshold =
        30.0;


    criticalThreshold =
        10.0;


    offThreshold =
        0.0;


    stateChanged =
        false;


    consumptionPerCycle =
        5.0;


    recoveryAmount =
        0.0;


    lastSimulation =
        0;


    simulationInterval =
        1000;

    simulationMode =
        true;

    simulateShutdown =
        false;
}


//====================================================
// Begin
//====================================================

void EnergyManager::begin(
    float initialEnergy
)
{

    if(
        initialEnergy < 0.0
    )
    {

        initialEnergy =
            0.0;

    }


    if(
        initialEnergy > 100.0
    )
    {

        initialEnergy =
            100.0;

    }


    energyLevel =
        initialEnergy;


    currentState =
        ENERGY_NORMAL;


    previousState =
        ENERGY_NORMAL;


    stateChanged =
        false;


    lastSimulation =
        millis();


    updateState();


    Serial.println();

    Serial.println(
        "[ENERGY] Manager initialized"
    );


    printStatus();

}


//====================================================
// Execute
//====================================================

void EnergyManager::execute()
{

    unsigned long now =
        millis();


    if(
        now - lastSimulation <
        simulationInterval
    )
    {

        return;

    }


    lastSimulation =
        now;


    //--------------------------------------------------
    // Already OFF
    //--------------------------------------------------

    if(
        isOff()
    )
    {

        return;

    }


    //--------------------------------------------------
    // Consume energy
    //--------------------------------------------------

    consume(
        consumptionPerCycle
    );


    //--------------------------------------------------
    // LOW
    //--------------------------------------------------

    if(
        isLow() &&
        previousState != ENERGY_LOW
    )
    {

        Serial.println(
            "[ALERT] Energy level LOW"
        );

    }


    //--------------------------------------------------
    // CRITICAL
    //--------------------------------------------------

    if(
        isCritical() &&
        previousState != ENERGY_CRITICAL
    )
    {

        Serial.println(
            "[ALERT] Energy level CRITICAL"
        );

    }


    //--------------------------------------------------
    // OFF
    //--------------------------------------------------

    if(
        isOff() &&
        previousState != ENERGY_OFF
    )
    {

        Serial.println(
            "[ALERT] Energy depleted"
        );


        Serial.println(
            "[ENERGY] ESP32 simulated OFF"
        );


        if(
            simulateShutdown
        )
        {

            Serial.println(
                "[ENERGY] Simulating system restart..."
            );


            delay(
                200
            );


            ESP.restart();

        }
        else
        {

            Serial.println(
                "[ENERGY] Shutdown simulation active"
            );


            Serial.println(
                "[ENERGY] System remains running for test"
            );

        }

    }

}
    

//====================================================
// Update
//====================================================

void EnergyManager::update()
{

    updateState();

}


//====================================================
// Set energy
//====================================================

void EnergyManager::setEnergy(
    float value
)
{

    if(
        value < 0.0
    )
    {

        value =
            0.0;

    }


    if(
        value > 100.0
    )
    {

        value =
            100.0;

    }


    energyLevel =
        value;


    updateState();

}


//====================================================
// Consume
//====================================================

void EnergyManager::consume(
    float amount
)
{

    if(
        amount < 0.0
    )
    {

        return;

    }


    energyLevel -=
        amount;


    if(
        energyLevel < 0.0
    )
    {

        energyLevel =
            0.0;

    }


    updateState();

}


//====================================================
// Restore
//====================================================

void EnergyManager::restore(
    float amount
)
{

    if(
        amount < 0.0
    )
    {

        return;

    }


    energyLevel +=
        amount;


    if(
        energyLevel > 100.0
    )
    {

        energyLevel =
            100.0;

    }


    updateState();

}


//====================================================
// Set energy state
//====================================================

void EnergyManager::setEnergyState(
    EnergyState state
)
{

    previousState =
        currentState;


    currentState =
        state;


    stateChanged =
        previousState !=
        currentState;

}


//====================================================
// Update state
//====================================================

void EnergyManager::updateState()
{

    EnergyState newState;


    //--------------------------------------------------
    // Determine state
    //--------------------------------------------------

    if(
        energyLevel <=
        offThreshold
    )
    {

        newState =
            ENERGY_OFF;

    }
    else if(
        energyLevel <=
        criticalThreshold
    )
    {

        newState =
            ENERGY_CRITICAL;

    }
    else if(
        energyLevel <=
        lowThreshold
    )
    {

        newState =
            ENERGY_LOW;

    }
    else
    {

        newState =
            ENERGY_NORMAL;

    }


    //--------------------------------------------------
    // Reset flag
    //--------------------------------------------------

    stateChanged =
        false;


    //--------------------------------------------------
    // State changed
    //--------------------------------------------------

    if(
        newState !=
        currentState
    )
    {

        previousState =
            currentState;


        currentState =
            newState;


        stateChanged =
            true;


        printStatus();

    }

}


//====================================================
// Get energy
//====================================================

float EnergyManager::getEnergy() const
{

    return
        energyLevel;

}


//====================================================
// Get state
//====================================================

EnergyState EnergyManager::getState() const
{

    return
        currentState;

}


//====================================================
// Get previous state
//====================================================

EnergyState EnergyManager::getPreviousState() const
{

    return
        previousState;

}


//====================================================
// Is normal
//====================================================

bool EnergyManager::isNormal() const
{

    return
        currentState ==
        ENERGY_NORMAL;

}


//====================================================
// Is low
//====================================================

bool EnergyManager::isLow() const
{

    return
        currentState ==
        ENERGY_LOW;

}


//====================================================
// Is critical
//====================================================

bool EnergyManager::isCritical() const
{

    return
        currentState ==
        ENERGY_CRITICAL;

}


//====================================================
// Is off
//====================================================

bool EnergyManager::isOff() const
{

    return
        currentState ==
        ENERGY_OFF;

}


//====================================================
// Has changed
//====================================================

bool EnergyManager::hasChanged() const
{

    return
        stateChanged;

}


//====================================================
// Print status
//====================================================

void EnergyManager::printStatus()
{

    Serial.print(
        "[ENERGY] Level: "
    );


    Serial.print(
        energyLevel,
        2
    );


    Serial.print(
        "% | State: "
    );


    switch(
        currentState
    )
    {

        case ENERGY_NORMAL:

            Serial.println(
                "NORMAL"
            );

            break;


        case ENERGY_LOW:

            Serial.println(
                "LOW"
            );

            break;


        case ENERGY_CRITICAL:

            Serial.println(
                "CRITICAL"
            );

            break;


        case ENERGY_OFF:

            Serial.println(
                "OFF"
            );

            break;

    }

}

//====================================================
// Set simulation mode
//====================================================

void EnergyManager::setSimulationMode(
    bool enabled
)
{

    simulationMode =
        enabled;

}


//====================================================
// Set simulated shutdown
//====================================================

void EnergyManager::setSimulateShutdown(
    bool enabled
)
{

    simulateShutdown =
        enabled;

}


//====================================================
// Is simulation mode
//====================================================

bool EnergyManager::isSimulationMode() const
{

    return
        simulationMode;

}


//====================================================
// Is simulated shutdown
//====================================================

bool EnergyManager::isSimulateShutdown() const
{

    return
        simulateShutdown;

}
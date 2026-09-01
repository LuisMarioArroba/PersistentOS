#include "kernel/SimulationManager.h"

#include "config/Config.h"


//====================================================
// Constructor
//====================================================

SimulationManager::SimulationManager()
{

    currentModel =
        SIM_NONE;


    currentState =
        SIM_STATE_IDLE;


    energy =
        SIMULATION_INITIAL_ENERGY;


    initialEnergy =
        SIMULATION_INITIAL_ENERGY;


    simulationTime =
        0.0f;


    standardDeviation =
        5.0f;


    lastUpdate =
        0;


    menuPrinted =
        false;

}


//====================================================
// Begin
//====================================================

void SimulationManager::begin()
{

    currentModel =
        SIM_NONE;


    currentState =
        SIM_STATE_IDLE;


    energy =
        SIMULATION_INITIAL_ENERGY;


    initialEnergy =
        SIMULATION_INITIAL_ENERGY;


    simulationTime =
        0.0f;


    lastUpdate =
        millis();


    menuPrinted =
        false;


    Serial.println();

    Serial.println(
        "[SIM] Simulation manager initialized"
    );

}


//====================================================
// Execute
//====================================================

void SimulationManager::execute()
{

    //--------------------------------------------------
    // Serial menu
    //--------------------------------------------------

    serialMenu();


    //--------------------------------------------------
    // Simulation OFF
    //--------------------------------------------------

    if(
        currentState ==
        SIM_STATE_OFF
    )
    {

        return;

    }


    //--------------------------------------------------
    // Simulation not running
    //--------------------------------------------------

    if(
        currentState !=
        SIM_STATE_RUNNING
    )
    {

        return;

    }


    //--------------------------------------------------
    // Timing
    //--------------------------------------------------

    unsigned long now =
        millis();


    if(
        now - lastUpdate <
        SIMULATION_UPDATE_MS
    )
    {

        return;

    }


    lastUpdate =
        now;


    //--------------------------------------------------
    // Increase simulation time
    //--------------------------------------------------

    simulationTime +=
        1.0f;


    //--------------------------------------------------
    // Calculate simulated energy
    //--------------------------------------------------

    float simulatedValue =
        calculateValue(
            simulationTime
        );


    energy =
        initialEnergy +
        simulatedValue;


    //--------------------------------------------------
    // Limit
    //--------------------------------------------------

    if(
        energy >
        SIMULATION_MAX_ENERGY
    )
    {

        energy =
            SIMULATION_MAX_ENERGY;

    }


    if(
        energy <
        SIMULATION_MIN_ENERGY
    )
    {

        energy =
            SIMULATION_MIN_ENERGY;

    }


    //--------------------------------------------------
    // Monitoring
    //--------------------------------------------------

    Serial.print(
        "[SIM] t="
    );

    Serial.print(
        simulationTime
    );

    Serial.print(
        " | Energy="
    );

    Serial.print(
        energy,
        2
    );

    Serial.print(
        "% | Model="
    );

    printModel();

}


//====================================================
// Serial Menu
//====================================================

void SimulationManager::serialMenu()
{

    if(
        !Serial.available()
    )
    {

        return;

    }


    char command =
        Serial.read();


    switch(command)
    {

        //--------------------------------------------------
        // Model menu
        //--------------------------------------------------

        case '1':

            selectModel(
                SIM_LINEAR
            );

            break;


        case '2':

            selectModel(
                SIM_LOGARITHMIC
            );

            break;


        case '3':

            selectModel(
                SIM_STANDARD_DEVIATION
            );

            break;


        case '4':

            selectModel(
                SIM_INVERSE_X
            );

            break;


        case '5':

            selectModel(
                SIM_EXPONENTIAL
            );

            break;


        //--------------------------------------------------
        // Start
        //--------------------------------------------------

        case 's':

        case 'S':

            start();

            break;


        //--------------------------------------------------
        // Stop
        //--------------------------------------------------

        case 'p':

        case 'P':

            stop();

            break;


        //--------------------------------------------------
        // Simulated shutdown
        //--------------------------------------------------

        case 'o':

        case 'O':

            simulateShutdown();

            break;


        //--------------------------------------------------
        // Simulated startup
        //--------------------------------------------------

        case 'r':

        case 'R':

            simulateStartup();

            break;


        //--------------------------------------------------
        // Reset simulation
        //--------------------------------------------------

        case 'x':

        case 'X':

            reset();

            break;


        //--------------------------------------------------
        // Menu
        //--------------------------------------------------

        case 'm':

        case 'M':

            printMenu();

            break;


        default:

            break;

    }

}


//====================================================
// Calculate model
//====================================================

float SimulationManager::calculateValue(
    float x
)
{

    switch(
        currentModel
    )
    {

        //--------------------------------------------------
        // Linear
        //--------------------------------------------------

        case SIM_LINEAR:

            return
                x *
                SIMULATION_STEP;


        //--------------------------------------------------
        // Logarithmic
        //--------------------------------------------------

        case SIM_LOGARITHMIC:

            return
                log(
                    x + 1.0f
                ) *
                10.0f;


        //--------------------------------------------------
        // Standard deviation
        //--------------------------------------------------

        case SIM_STANDARD_DEVIATION:
        {

            float deviation =
                standardDeviation;


            float value =
                sin(
                    x * 0.25f
                ) *
                deviation;


            return value;

        }


        //--------------------------------------------------
        // 1/x
        //--------------------------------------------------

        case SIM_INVERSE_X:

            if(
                x <= 0.0f
            )
            {

                return 0.0f;

            }


            return
                100.0f /
                x;


        //--------------------------------------------------
        // Exponential
        //--------------------------------------------------

        case SIM_EXPONENTIAL:

            return
                100.0f *
                (
                    1.0f -
                    exp(
                        -0.05f * x
                    )
                );


        //--------------------------------------------------
        // None
        //--------------------------------------------------

        default:

            return 0.0f;

    }

}


//====================================================
// Select model
//====================================================

void SimulationManager::selectModel(
    SimulationModel model
)
{

    currentModel =
        model;


    simulationTime =
        0.0f;


    energy =
        initialEnergy;


    currentState =
        SIM_STATE_IDLE;


    Serial.println();

    Serial.println(
        "[SIM] Model selected"
    );

    printModel();

}


//====================================================
// Start
//====================================================

void SimulationManager::start()
{

    if(
        currentModel ==
        SIM_NONE
    )
    {

        Serial.println(
            "[SIM] Select a model first"
        );

        return;

    }


    currentState =
        SIM_STATE_RUNNING;


    lastUpdate =
        millis();


    Serial.println();

    Serial.println(
        "[SIM] Simulation STARTED"
    );

}


//====================================================
// Stop
//====================================================

void SimulationManager::stop()
{

    if(
        currentState ==
        SIM_STATE_RUNNING
    )
    {

        currentState =
            SIM_STATE_IDLE;


        Serial.println(
            "[SIM] Simulation PAUSED"
        );

    }

}


//====================================================
// Simulated shutdown
//====================================================

void SimulationManager::simulateShutdown()
{

    Serial.println();

    Serial.println(
        "[SIM] ==============================="
    );

    Serial.println(
        "[SIM] SIMULATED SHUTDOWN"
    );

    Serial.println(
        "[SIM] ==============================="
    );


    Serial.print(
        "[SIM] Energy preserved: "
    );

    Serial.print(
        energy,
        2
    );

    Serial.println(
        "%"
    );


    currentState =
        SIM_STATE_OFF;

}


//====================================================
// Simulated startup
//====================================================

void SimulationManager::simulateStartup()
{

    Serial.println();

    Serial.println(
        "[SIM] ==============================="
    );

    Serial.println(
        "[SIM] SIMULATED STARTUP"
    );

    Serial.println(
        "[SIM] ==============================="
    );


    Serial.print(
        "[SIM] Restored energy: "
    );

    Serial.print(
        energy,
        2
    );

    Serial.println(
        "%"
    );


    currentState =
        SIM_STATE_RUNNING;


    lastUpdate =
        millis();

}


//====================================================
// Reset
//====================================================

void SimulationManager::reset()
{

    currentState =
        SIM_STATE_IDLE;


    simulationTime =
        0.0f;


    energy =
        initialEnergy;


    Serial.println();

    Serial.println(
        "[SIM] Simulation reset"
    );

}


//====================================================
// Print menu
//====================================================

void SimulationManager::printMenu()
{

    Serial.println();

    Serial.println(
        "================================="
    );

    Serial.println(
        "       SIMULATION MENU"
    );

    Serial.println(
        "================================="
    );

    Serial.println(
        "1 - Linear"
    );

    Serial.println(
        "2 - Logarithmic"
    );

    Serial.println(
        "3 - Standard deviation"
    );

    Serial.println(
        "4 - 1/x"
    );

    Serial.println(
        "5 - Exponential"
    );

    Serial.println();

    Serial.println(
        "S - Start"
    );

    Serial.println(
        "P - Pause"
    );

    Serial.println(
        "O - Simulated OFF"
    );

    Serial.println(
        "R - Simulated ON"
    );

    Serial.println(
        "X - Reset"
    );

    Serial.println(
        "M - Menu"
    );

    Serial.println(
        "================================="
    );

}


//====================================================
// Print model
//====================================================

void SimulationManager::printModel()
{

    switch(
        currentModel
    )
    {

        case SIM_LINEAR:

            Serial.println(
                "LINEAR"
            );

            break;


        case SIM_LOGARITHMIC:

            Serial.println(
                "LOGARITHMIC"
            );

            break;


        case SIM_STANDARD_DEVIATION:

            Serial.println(
                "STANDARD_DEVIATION"
            );

            break;


        case SIM_INVERSE_X:

            Serial.println(
                "1/X"
            );

            break;


        case SIM_EXPONENTIAL:

            Serial.println(
                "EXPONENTIAL"
            );

            break;


        default:

            Serial.println(
                "NONE"
            );

            break;

    }

}


//====================================================
// Getters
//====================================================

bool SimulationManager::isRunning() const
{

    return
        currentState ==
        SIM_STATE_RUNNING;

}


bool SimulationManager::isOff() const
{

    return
        currentState ==
        SIM_STATE_OFF;

}


float SimulationManager::getEnergy() const
{

    return energy;

}


float SimulationManager::getSimulationTime() const
{

    return simulationTime;

}


SimulationModel
SimulationManager::getModel() const
{

    return currentModel;

}


SimulationState
SimulationManager::getState() const
{

    return currentState;

}
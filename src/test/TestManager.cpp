#include "test/TestManager.h"

#include "config/Config.h"


//====================================================
// Constructor
//====================================================

TestManager::TestManager()
{

    checkpointManager =
        nullptr;

    failureManager =
        nullptr;

    currentScenario =
        TEST_NORMAL;

    failureInjected =
        false;

    lastButtonState =
        HIGH;
    energyManager =
    nullptr;

    energySimulation =
        ENERGY_SIM_NONE;


    energyMenuActive =
        false;


    energySimulationActive =
        false;


    energySimulationStart =
        0;


    energySimulationDuration =
        30000;

}


//====================================================
// Begin
//====================================================

void TestManager::begin(

    ExecutionCheckpoint* checkpointPtr,

    FailureManager* failurePtr,

    EnergyManager* energyPtr

)
{

    checkpointManager =
        checkpointPtr;

    failureManager =
        failurePtr;
    
    energyManager =
        energyPtr;

    printScenario();

}

void TestManager::printEnergyMenu()
{

    Serial.println();

    Serial.println(
        "================================="
    );

    Serial.println(
        "      ENERGY SIMULATION MENU"
    );

    Serial.println(
        "================================="
    );


    Serial.println(
        "1. Logarithmic"
    );

    Serial.println(
        "2. Standard deviation"
    );

    Serial.println(
        "3. 1/x  (x > 0)"
    );

    Serial.println(
        "4. Linear"
    );

    Serial.println(
        "S. Stop simulation"
    );


    Serial.println(
        "================================="
    );

    Serial.println(
        "Select behavior:"
    );

}

//====================================================
// Serial processing
//====================================================

void TestManager::processSerial()
{

    if(
        !Serial.available()
    )
    {
        return;
    }


    char command =
        Serial.read();


    if(
        command == '\n' ||
        command == '\r'
    )
    {
        return;
    }


    //--------------------------------------------------
    // Main menu
    //--------------------------------------------------

    if(
        command == 'm' ||
        command == 'M'
    )
    {

        printEnergyMenu();

        return;

    }

    //--------------------------------------------------
    // Energy simulation
    //--------------------------------------------------

    if(
        command == '1'
    )
    {

        startEnergySimulation(
            ENERGY_SIM_LOGARITHMIC
        );

        return;

    }


    if(
        command == '2'
    )
    {

        startEnergySimulation(
            ENERGY_SIM_STANDARD_DEVIATION
        );

        return;

    }


    if(
        command == '3'
    )
    {

        startEnergySimulation(
            ENERGY_SIM_INVERSE
        );

        return;

    }


    if(
        command == '4'
    )
    {

        startEnergySimulation(
            ENERGY_SIM_LINEAR
        );

        return;

    }


    //--------------------------------------------------
    // Stop
    //--------------------------------------------------

    if(
        command == 's' ||
        command == 'S'
    )
    {

        stopEnergySimulation();

        return;

    }

}


//====================================================
// Execute
//====================================================

void TestManager::execute()
{

    //--------------------------------------------------
    // Serial menu
    //--------------------------------------------------

    processSerial();


    //--------------------------------------------------
    // Energy simulation
    //--------------------------------------------------

    if(
        energySimulationActive
    )
    {

        executeEnergySimulation();

    }


    //--------------------------------------------------
    // Button
    //--------------------------------------------------

    if(buttonPressed())
    {

        nextScenario();

        return;

    }


    //--------------------------------------------------
    // Normal scenario
    //--------------------------------------------------

    if(
        currentScenario ==
        TEST_NORMAL
    )
    {

        return;

    }


    //--------------------------------------------------
    // Failure already injected
    //--------------------------------------------------

    if(
        failureInjected
    )
    {

        return;

    }


    //--------------------------------------------------
    // Validate managers
    //--------------------------------------------------

    if(
        checkpointManager == nullptr ||
        failureManager == nullptr
    )
    {

        return;

    }


    //--------------------------------------------------
    // Get communication checkpoint
    //--------------------------------------------------

    uint8_t checkpoint =
        checkpointManager->getCheckpoint(
            TASK_COMMUNICATION
        );


    Serial.print(
        "[TEST] COMM checkpoint: "
    );

    Serial.println(
        checkpoint
    );


    injectFailure(
        checkpoint
    );

}


//====================================================
// Button
//====================================================

bool TestManager::buttonPressed()
{

    bool state =
        digitalRead(
            TEST_BUTTON_PIN
        );


    bool pressed =
    (
        lastButtonState == HIGH &&
        state == LOW
    );


    lastButtonState =
        state;


    return pressed;

}


//====================================================
// Force scenario (automatización externa)
//====================================================

void TestManager::forceScenario(
    TestScenario scenario
)
{

    currentScenario =
        scenario;


    failureInjected =
        false;


    if(
        failureManager != nullptr
    )
    {

        failureManager->clear();

    }


    if(
        checkpointManager != nullptr
    )
    {

        checkpointManager->update(

            TASK_COMMUNICATION,

            STEP_IDLE,

            0

        );

    }


    printScenario();

}


//====================================================
// Next scenario
//====================================================

void TestManager::nextScenario()
{

    currentScenario =
        static_cast<TestScenario>(

            (
                currentScenario + 1
            )
            %
            TEST_TOTAL

        );


    failureInjected =
        false;


    //--------------------------------------------------
    // Clear failure
    //--------------------------------------------------

    if(
        failureManager != nullptr
    )
    {

        failureManager->clear();

    }


    //--------------------------------------------------
    // Reset communication checkpoint
    //--------------------------------------------------

    if(
        checkpointManager != nullptr
    )
    {

        checkpointManager->update(

            TASK_COMMUNICATION,

            STEP_IDLE,

            0

        );

    }


    printScenario();

}


//====================================================
// Inject failure
//====================================================

void TestManager::injectFailure(
    uint8_t checkpoint
)
{

    bool inject =
        false;


    switch(currentScenario)
    {

        //--------------------------------------------------
        // CREATE
        //--------------------------------------------------

        case TEST_FAIL_CREATE:

            inject =
            (
                checkpoint ==
                STEP_COMMUNICATION_CREATE
            );

            break;


        //--------------------------------------------------
        // PREPARE
        //--------------------------------------------------

        case TEST_FAIL_PREPARE:

            inject =
            (
                checkpoint ==
                STEP_COMMUNICATION_PREPARE
            );

            break;


        //--------------------------------------------------
        // SEND
        //--------------------------------------------------

        case TEST_FAIL_SEND:

            inject =
            (
                checkpoint ==
                STEP_COMMUNICATION_SEND
            );

            break;


        //--------------------------------------------------
        // WAIT ACK
        //--------------------------------------------------

        case TEST_FAIL_WAIT_ACK:

            inject =
            (
                checkpoint ==
                STEP_COMMUNICATION_WAIT_ACK
            );

            break;


        //--------------------------------------------------
        // CONFIRM
        //--------------------------------------------------

        case TEST_FAIL_CONFIRM:

            inject =
            (
                checkpoint ==
                STEP_COMMUNICATION_CONFIRM
            );

            break;


        //--------------------------------------------------
        // Default
        //--------------------------------------------------

        default:

            break;

    }


    //--------------------------------------------------
    // Trigger
    //--------------------------------------------------

    if(inject)
    {

        Serial.println();
        
        Serial.println(
            "[TEST] ==============================="
        );

        Serial.print(
            "[TEST] Injecting failure at checkpoint: "
        );

        Serial.println(
            checkpoint
        );

        Serial.println(
            "[TEST] ==============================="
        );


        failureManager->triggerFailure();


        failureInjected =
            true;

    }

}


//====================================================
// Print scenario
//====================================================

void TestManager::printScenario()
{

    Serial.println();

    Serial.println(
        "================================="
    );

    Serial.print(
        "Scenario: "
    );


    switch(currentScenario)
    {

        case TEST_NORMAL:

            Serial.println(
                "NORMAL"
            );

            break;


        case TEST_FAIL_CREATE:

            Serial.println(
                "FAIL CREATE"
            );

            break;


        case TEST_FAIL_PREPARE:

            Serial.println(
                "FAIL PREPARE"
            );

            break;


        case TEST_FAIL_SEND:

            Serial.println(
                "FAIL SEND"
            );

            break;


        case TEST_FAIL_WAIT_ACK:

            Serial.println(
                "FAIL WAIT ACK"
            );

            break;


        case TEST_FAIL_CONFIRM:

            Serial.println(
                "FAIL CONFIRM"
            );

            break;


        default:

            Serial.println(
                "UNKNOWN"
            );

            break;

    }


    Serial.println(
        "================================="
    );

}
void TestManager::startEnergySimulation(
    EnergySimulationType type
)
{

    if(
        energyManager == nullptr
    )
    {

        Serial.println(
            "[TEST] Energy manager unavailable"
        );

        return;

    }


    energySimulation =
        type;


    energySimulationActive =
        true;


    energySimulationStart =
        millis();


    energyManager->setSimulationMode(
        true
    );


    energyManager->setSimulateShutdown(
        false
    );


    energyManager->setEnergy(
        100.0
    );


    Serial.println();

    Serial.println(
        "[TEST] ================================="
    );

    Serial.println(
        "[TEST] ENERGY SIMULATION STARTED"
    );

    Serial.println(
        "[TEST] ================================="
    );


    switch(
        type
    )
    {

        case ENERGY_SIM_LOGARITHMIC:

            Serial.println(
                "[TEST] Behavior: LOGARITHMIC"
            );

            break;


        case ENERGY_SIM_STANDARD_DEVIATION:

            Serial.println(
                "[TEST] Behavior: STANDARD DEVIATION"
            );

            break;


        case ENERGY_SIM_INVERSE:

            Serial.println(
                "[TEST] Behavior: 1/x"
            );

            break;


        case ENERGY_SIM_LINEAR:

            Serial.println(
                "[TEST] Behavior: LINEAR"
            );

            break;


        default:

            break;

    }

}
void TestManager::stopEnergySimulation()
{

    energySimulationActive =
        false;


    energySimulation =
        ENERGY_SIM_NONE;


    if(
        energyManager != nullptr
    )
    {

        energyManager->setSimulationMode(
            false
        );

    }


    Serial.println();

    Serial.println(
        "[TEST] Energy simulation stopped"
    );

}

void TestManager::executeEnergySimulation()
{

    if(
        energyManager == nullptr
    )
    {

        return;

    }


    unsigned long elapsed =
        millis() -
        energySimulationStart;


    if(
        elapsed >=
        energySimulationDuration
    )
    {

        stopEnergySimulation();

        return;

    }


    float progress =
        (
            float(elapsed) /
            float(energySimulationDuration)
        );


    float energy =
        calculateEnergy(
            progress
        );


    energyManager->setEnergy(
        energy
    );

}

float TestManager::calculateEnergy(
    float progress
)
{

    if(
        progress < 0.0
    )
    {
        progress = 0.0;
    }


    if(
        progress > 1.0
    )
    {
        progress = 1.0;
    }


    //--------------------------------------------------
    // Temporary linear behavior
    //--------------------------------------------------

    return
        100.0 *
        (1.0 - progress);

}
#include "kernel/EnergyPredictionManager.h"

//====================================================
// Constructor
//====================================================

EnergyPredictionManager::EnergyPredictionManager()
{

    currentValue =
        0.0;


    previousValue =
        0.0;


    mean =
        0.0;


    variance =
        0.0;


    standardDeviation =
        0.0;


    trend =
        0.0;


    absoluteChange =
        0.0;


    sampleCount =
        0;


    stableSampleCount =
        0;


    stableSignal =
        false;


    stableSince =
        0;


    currentBehavior =
        ENERGY_BEHAVIOR_UNKNOWN;


    //--------------------------------------------------
    // Configuration
    //--------------------------------------------------

    /*
        Diferencia máxima considerada como
        estabilidad entre muestras.

        Esto NO significa que el sistema
        conozca el comportamiento.
    */

    stabilityThreshold =
        0.05;


    /*
        Cambio mínimo para considerar
        una tendencia.
    */

    trendThreshold =
        0.10;


    /*
        Desviación estándar mínima para
        considerar fluctuación.
    */

    fluctuationThreshold =
        0.50;


    /*
        Tiempo mínimo durante el cual una señal
        debe mantenerse estable para dejar de
        considerarla como comportamiento dinámico.
    */

    stabilityTime =
        10000;

}


//====================================================
// Begin
//====================================================

void EnergyPredictionManager::begin()
{

    currentValue =
        0.0;


    previousValue =
        0.0;


    mean =
        0.0;


    variance =
        0.0;


    standardDeviation =
        0.0;


    trend =
        0.0;


    absoluteChange =
        0.0;


    sampleCount =
        0;


    stableSampleCount =
        0;


    stableSignal =
        false;


    stableSince =
        0;


    currentBehavior =
        ENERGY_BEHAVIOR_UNKNOWN;


    Serial.println(
        "[PREDICTION] Manager initialized"
    );

}


//====================================================
// Observe
//====================================================

void EnergyPredictionManager::observe(
    float value
)
{

    //--------------------------------------------------
    // First sample
    //--------------------------------------------------

    if(
        sampleCount == 0
    )
    {

        currentValue =
            value;


        previousValue =
            value;


        mean =
            value;


        sampleCount =
            1;


        stableSince =
            millis();


        return;

    }


    //--------------------------------------------------
    // Previous value
    //--------------------------------------------------

    previousValue =
        currentValue;


    currentValue =
        value;


    //--------------------------------------------------
    // Statistics
    //--------------------------------------------------

    updateStatistics(
        value
    );


    //--------------------------------------------------
    // Behavior
    //--------------------------------------------------

    updateBehavior();

}


//====================================================
// Update statistics
//====================================================

void EnergyPredictionManager::updateStatistics(
    float value
)
{

    sampleCount++;


    //--------------------------------------------------
    // Difference
    //--------------------------------------------------

    float difference =
        value -
        previousValue;


    absoluteChange =
        fabs(
            difference
        );


    trend =
        difference;


    //--------------------------------------------------
    // Running mean
    //--------------------------------------------------

    mean +=
        (
            value -
            mean
        )
        /
        sampleCount;


    //--------------------------------------------------
    // Stability
    //--------------------------------------------------

    if(
        isStable(value)
    )
    {

        stableSampleCount++;


        if(
            stableSampleCount == 1
        )
        {

            stableSince =
                millis();

        }

    }
    else
    {

        stableSampleCount =
            0;


        stableSince =
            0;


        stableSignal =
            false;

    }


    //--------------------------------------------------
    // Variance
    //
    // Simple running approximation.
    //--------------------------------------------------

    float delta =
        value -
        mean;


    variance =
        (
            (
                sampleCount - 1
            )
            *
            variance
            +
            delta * delta
        )
        /
        sampleCount;


    if(
        variance < 0.0
    )
    {

        variance =
            0.0;

    }


    standardDeviation =
        sqrt(
            variance
        );

}


//====================================================
// Is stable
//====================================================

bool EnergyPredictionManager::isStable(
    float value
) const
{

    float difference =
        fabs(
            value -
            previousValue
        );


    return
        difference <=
        stabilityThreshold;

}


//====================================================
// Update behavior
//====================================================

void EnergyPredictionManager::updateBehavior()
{

    unsigned long now =
        millis();


    //--------------------------------------------------
    // Stable signal
    //--------------------------------------------------

    if(
        stableSampleCount > 0 &&
        stableSince > 0 &&
        (
            now -
            stableSince
        ) >= stabilityTime
    )
    {

        stableSignal =
            true;


        currentBehavior =
            ENERGY_BEHAVIOR_STABLE;


        return;

    }


    //--------------------------------------------------
    // Not enough information
    //--------------------------------------------------

    if(
        sampleCount < 3
    )
    {

        currentBehavior =
            ENERGY_BEHAVIOR_UNKNOWN;

        return;

    }


    //--------------------------------------------------
    // Fluctuating
    //--------------------------------------------------

    if(
        standardDeviation >=
        fluctuationThreshold
    )
    {

        currentBehavior =
            ENERGY_BEHAVIOR_FLUCTUATING;

        return;

    }


    //--------------------------------------------------
    // Increasing
    //--------------------------------------------------

    if(
        trend >
        trendThreshold
    )
    {

        currentBehavior =
            ENERGY_BEHAVIOR_INCREASING;

        return;

    }


    //--------------------------------------------------
    // Decreasing
    //--------------------------------------------------

    if(
        trend <
        -trendThreshold
    )
    {

        currentBehavior =
            ENERGY_BEHAVIOR_DECREASING;

        return;

    }


    //--------------------------------------------------
    // Otherwise
    //--------------------------------------------------

    currentBehavior =
        ENERGY_BEHAVIOR_INTERMITTENT;

}


//====================================================
// Update
//====================================================

void EnergyPredictionManager::update()
{

    if(
        sampleCount == 0
    )
    {

        return;

    }


    updateBehavior();

}


//====================================================
// Get behavior
//====================================================

EnergyBehavior
EnergyPredictionManager::getBehavior() const
{

    return
        currentBehavior;

}


//====================================================
// Get mean
//====================================================

float EnergyPredictionManager::getMean() const
{

    return
        mean;

}


//====================================================
// Get variance
//====================================================

float EnergyPredictionManager::getVariance() const
{

    return
        variance;

}


//====================================================
// Get standard deviation
//====================================================

float EnergyPredictionManager::getStandardDeviation() const
{

    return
        standardDeviation;

}


//====================================================
// Get trend
//====================================================

float EnergyPredictionManager::getTrend() const
{

    return
        trend;

}


//====================================================
// Get current value
//====================================================

float EnergyPredictionManager::getCurrentValue() const
{

    return
        currentValue;

}


//====================================================
// Get sample count
//====================================================

uint32_t
EnergyPredictionManager::getSampleCount() const
{

    return
        sampleCount;

}


//====================================================
// Signal stable
//====================================================

bool EnergyPredictionManager::isSignalStable() const
{

    return
        stableSignal;

}


//====================================================
// Print status
//====================================================

void EnergyPredictionManager::printStatus()
{

    Serial.print(
        "[PREDICTION] Value: "
    );

    Serial.print(
        currentValue,
        3
    );


    Serial.print(
        " | Mean: "
    );

    Serial.print(
        mean,
        3
    );


    Serial.print(
        " | StdDev: "
    );

    Serial.print(
        standardDeviation,
        3
    );


    Serial.print(
        " | Trend: "
    );

    Serial.print(
        trend,
        3
    );


    Serial.print(
        " | Behavior: "
    );


    switch(
        currentBehavior
    )
    {

        case ENERGY_BEHAVIOR_UNKNOWN:

            Serial.println(
                "UNKNOWN"
            );

            break;


        case ENERGY_BEHAVIOR_STABLE:

            Serial.println(
                "STABLE"
            );

            break;


        case ENERGY_BEHAVIOR_INCREASING:

            Serial.println(
                "INCREASING"
            );

            break;


        case ENERGY_BEHAVIOR_DECREASING:

            Serial.println(
                "DECREASING"
            );

            break;


        case ENERGY_BEHAVIOR_FLUCTUATING:

            Serial.println(
                "FLUCTUATING"
            );

            break;


        case ENERGY_BEHAVIOR_INTERMITTENT:

            Serial.println(
                "INTERMITTENT"
            );

            break;


        default:

            Serial.println(
                "UNKNOWN"
            );

            break;

    }

}
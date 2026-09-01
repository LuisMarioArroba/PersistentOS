#ifndef ENERGY_PREDICTION_MANAGER_H
#define ENERGY_PREDICTION_MANAGER_H

#include <Arduino.h>

//====================================================
// Energy behavior
//====================================================

enum EnergyBehavior
{
    ENERGY_BEHAVIOR_UNKNOWN = 0,

    ENERGY_BEHAVIOR_STABLE,

    ENERGY_BEHAVIOR_INCREASING,

    ENERGY_BEHAVIOR_DECREASING,

    ENERGY_BEHAVIOR_FLUCTUATING,

    ENERGY_BEHAVIOR_INTERMITTENT
};


//====================================================
// Energy Prediction Manager
//====================================================

class EnergyPredictionManager
{

private:

    //--------------------------------------------------
    // Current observation
    //--------------------------------------------------

    float currentValue;

    float previousValue;


    //--------------------------------------------------
    // Statistics
    //--------------------------------------------------

    float mean;

    float variance;

    float standardDeviation;


    //--------------------------------------------------
    // Trend
    //--------------------------------------------------

    float trend;

    float absoluteChange;


    //--------------------------------------------------
    // Sample information
    //--------------------------------------------------

    uint32_t sampleCount;

    uint32_t stableSampleCount;


    //--------------------------------------------------
    // Stability
    //--------------------------------------------------

    bool stableSignal;

    unsigned long stableSince;


    //--------------------------------------------------
    // Current behavior
    //--------------------------------------------------

    EnergyBehavior currentBehavior;


    //--------------------------------------------------
    // Configuration
    //--------------------------------------------------

    float stabilityThreshold;

    float trendThreshold;

    float fluctuationThreshold;

    unsigned long stabilityTime;


    //--------------------------------------------------
    // Internal
    //--------------------------------------------------

    void updateStatistics(
        float value
    );


    void updateBehavior();


    bool isStable(
        float value
    ) const;


public:

    //--------------------------------------------------
    // Constructor
    //--------------------------------------------------

    EnergyPredictionManager();


    //--------------------------------------------------
    // Initialization
    //--------------------------------------------------

    void begin();


    //--------------------------------------------------
    // Observation
    //--------------------------------------------------

    void observe(
        float value
    );


    //--------------------------------------------------
    // Update
    //--------------------------------------------------

    void update();


    //--------------------------------------------------
    // Getters
    //--------------------------------------------------

    EnergyBehavior getBehavior() const;

    float getMean() const;

    float getVariance() const;

    float getStandardDeviation() const;

    float getTrend() const;

    float getCurrentValue() const;

    uint32_t getSampleCount() const;


    //--------------------------------------------------
    // Stability
    //--------------------------------------------------

    bool isSignalStable() const;


    //--------------------------------------------------
    // Information
    //--------------------------------------------------

    void printStatus();

};

#endif
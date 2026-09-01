#ifndef BEHAVIOR_MANAGER_H
#define BEHAVIOR_MANAGER_H

#include <Arduino.h>


//====================================================
// Behavior types
//====================================================

enum BehaviorType
{

    BEHAVIOR_UNKNOWN = 0,

    BEHAVIOR_LOGARITHMIC,

    BEHAVIOR_STANDARD_DEVIATION,

    BEHAVIOR_INVERSE,

    BEHAVIOR_LINEAR,

    BEHAVIOR_TOTAL

};


//====================================================
// Behavior observation
//====================================================

struct BehaviorObservation
{

    unsigned long timestamp;

    float localValue;

    float remoteValue;

};


//====================================================
// Behavior Manager
//====================================================

class BehaviorManager
{

private:

    //--------------------------------------------------
    // Observation buffer
    //--------------------------------------------------

    static const uint8_t MAX_OBSERVATIONS = 20;

    BehaviorObservation observations[
        MAX_OBSERVATIONS
    ];

    uint8_t observationCount;


    //--------------------------------------------------
    // Probabilities
    //--------------------------------------------------

    float probabilities[
        BEHAVIOR_TOTAL
    ];


    //--------------------------------------------------
    // Current estimation
    //--------------------------------------------------

    BehaviorType estimatedBehavior;


    //--------------------------------------------------
    // Internal calculations
    //--------------------------------------------------

    float calculateScore(
        BehaviorType behavior
    );

    void calculateProbabilities();

    void updateEstimatedBehavior();


public:

    //--------------------------------------------------
    // Constructor
    //--------------------------------------------------

    BehaviorManager();


    //--------------------------------------------------
    // Initialization
    //--------------------------------------------------

    void begin();


    //--------------------------------------------------
    // Observation
    //--------------------------------------------------

    void observe(

        float localValue,

        float remoteValue,

        unsigned long timestamp

    );


    //--------------------------------------------------
    // Information
    //--------------------------------------------------

    BehaviorType getEstimatedBehavior() const;

    float getProbability(
        BehaviorType behavior
    ) const;


    uint8_t getObservationCount() const;


    //--------------------------------------------------
    // Maintenance
    //--------------------------------------------------

    void clear();


    //--------------------------------------------------
    // Debug
    //--------------------------------------------------

    void printStatus();

};

#endif
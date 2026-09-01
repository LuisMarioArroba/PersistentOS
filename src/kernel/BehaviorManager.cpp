#include "kernel/BehaviorManager.h"


//====================================================
// Constructor
//====================================================

BehaviorManager::BehaviorManager()
{

    observationCount =
        0;


    estimatedBehavior =
        BEHAVIOR_UNKNOWN;


    for(
        uint8_t i = 0;
        i < BEHAVIOR_TOTAL;
        i++
    )
    {

        probabilities[i] =
            0.0;

    }

}


//====================================================
// Begin
//====================================================

void BehaviorManager::begin()
{

    clear();


    Serial.println(
        "[BEHAVIOR] Manager initialized"
    );

}


//====================================================
// Observe
//====================================================

void BehaviorManager::observe(

    float localValue,

    float remoteValue,

    unsigned long timestamp

)
{

    //--------------------------------------------------
    // Buffer full
    //--------------------------------------------------

    if(
        observationCount >=
        MAX_OBSERVATIONS
    )
    {

        //------------------------------------------------
        // Shift observations
        //------------------------------------------------

        for(
            uint8_t i = 1;
            i < MAX_OBSERVATIONS;
            i++
        )
        {

            observations[i - 1] =
                observations[i];

        }


        observationCount =
            MAX_OBSERVATIONS - 1;

    }


    //--------------------------------------------------
    // Store observation
    //--------------------------------------------------

    observations[
        observationCount
    ].timestamp =
        timestamp;


    observations[
        observationCount
    ].localValue =
        localValue;


    observations[
        observationCount
    ].remoteValue =
        remoteValue;


    observationCount++;


    //--------------------------------------------------
    // Update estimation
    //--------------------------------------------------

    calculateProbabilities();

    updateEstimatedBehavior();

}


//====================================================
// Calculate score
//====================================================

float BehaviorManager::calculateScore(
    BehaviorType behavior
)
{

    //--------------------------------------------------
    // Not enough observations
    //--------------------------------------------------

    if(
        observationCount < 2
    )
    {

        return 0.0;

    }


    //--------------------------------------------------
    // Initial implementation
    //
    // For now the score is only a placeholder for
    // the statistical model.
    //
    // The real mathematical models will be added
    // without changing the public interface.
    //--------------------------------------------------

    float score =
        0.0;


    for(
        uint8_t i = 1;
        i < observationCount;
        i++
    )
    {

        float delta =
            observations[i].localValue -
            observations[i - 1].localValue;


        //------------------------------------------------
        // Linear
        //------------------------------------------------

        if(
            behavior ==
            BEHAVIOR_LINEAR
        )
        {

            score +=
                1.0 /
                (
                    1.0 +
                    fabs(delta)
                );

        }


        //------------------------------------------------
        // Other behaviors
        //------------------------------------------------

        else
        {

            score +=
                0.5;

        }

    }


    return score;

}


//====================================================
// Calculate probabilities
//====================================================

void BehaviorManager::calculateProbabilities()
{

    float scores[
        BEHAVIOR_TOTAL
    ];


    float totalScore =
        0.0;


    //--------------------------------------------------
    // Calculate scores
    //--------------------------------------------------

    for(
        uint8_t i = 1;
        i < BEHAVIOR_TOTAL;
        i++
    )
    {

        scores[i] =
            calculateScore(
                static_cast<BehaviorType>(i)
            );


        totalScore +=
            scores[i];

    }


    //--------------------------------------------------
    // Normalize
    //--------------------------------------------------

    if(
        totalScore <= 0.0
    )
    {

        for(
            uint8_t i = 0;
            i < BEHAVIOR_TOTAL;
            i++
        )
        {

            probabilities[i] =
                0.0;

        }


        probabilities[
            BEHAVIOR_UNKNOWN
        ] =
            1.0;


        return;

    }


    probabilities[
        BEHAVIOR_UNKNOWN
    ] =
        0.0;


    for(
        uint8_t i = 1;
        i < BEHAVIOR_TOTAL;
        i++
    )
    {

        probabilities[i] =
            scores[i] /
            totalScore;

    }

}


//====================================================
// Update estimated behavior
//====================================================

void BehaviorManager::updateEstimatedBehavior()
{

    float bestProbability =
        0.0;


    estimatedBehavior =
        BEHAVIOR_UNKNOWN;


    for(
        uint8_t i = 1;
        i < BEHAVIOR_TOTAL;
        i++
    )
    {

        if(
            probabilities[i] >
            bestProbability
        )
        {

            bestProbability =
                probabilities[i];


            estimatedBehavior =
                static_cast<BehaviorType>(i);

        }

    }

}


//====================================================
// Get estimated behavior
//====================================================

BehaviorType
BehaviorManager::getEstimatedBehavior() const
{

    return estimatedBehavior;

}


//====================================================
// Get probability
//====================================================

float BehaviorManager::getProbability(
    BehaviorType behavior
) const
{

    if(
        behavior >=
        BEHAVIOR_TOTAL
    )
    {

        return 0.0;

    }


    return probabilities[
        behavior
    ];

}


//====================================================
// Get observation count
//====================================================

uint8_t
BehaviorManager::getObservationCount() const
{

    return observationCount;

}


//====================================================
// Clear
//====================================================

void BehaviorManager::clear()
{

    observationCount =
        0;


    estimatedBehavior =
        BEHAVIOR_UNKNOWN;


    for(
        uint8_t i = 0;
        i < BEHAVIOR_TOTAL;
        i++
    )
    {

        probabilities[i] =
            0.0;

    }


}


//====================================================
// Print status
//====================================================

void BehaviorManager::printStatus()
{

    Serial.println();

    Serial.println(
        "[BEHAVIOR] ==============================="
    );


    Serial.print(
        "[BEHAVIOR] Observations: "
    );

    Serial.println(
        observationCount
    );


    Serial.print(
        "[BEHAVIOR] Linear: "
    );

    Serial.print(
        probabilities[
            BEHAVIOR_LINEAR
        ] * 100.0,
        2
    );

    Serial.println(
        "%"
    );


    Serial.print(
        "[BEHAVIOR] Logarithmic: "
    );

    Serial.print(
        probabilities[
            BEHAVIOR_LOGARITHMIC
        ] * 100.0,
        2
    );

    Serial.println(
        "%"
    );


    Serial.print(
        "[BEHAVIOR] Inverse: "
    );

    Serial.print(
        probabilities[
            BEHAVIOR_INVERSE
        ] * 100.0,
        2
    );

    Serial.println(
        "%"
    );


    Serial.print(
        "[BEHAVIOR] Standard deviation: "
    );

    Serial.print(
        probabilities[
            BEHAVIOR_STANDARD_DEVIATION
        ] * 100.0,
        2
    );

    Serial.println(
        "%"
    );


    Serial.println(
        "[BEHAVIOR] ==============================="
    );

}
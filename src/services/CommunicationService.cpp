#include "services/CommunicationService.h"



CommunicationService::CommunicationService()
{

    buffer = nullptr;

    communicationManager = nullptr;

    sensorService = nullptr;

    packetCounter = 0;

    failureManager = nullptr;

}






void CommunicationService::begin(

    CommunicationBuffer* bufferPtr,

    CommunicationManager* communicationPtr,

    ResumeManager* resumePtr,

    ExecutionCheckpoint* checkpointPtr,

    SensorService* sensorPtr,

    FailureManager* failurePtr

)
{

    buffer = bufferPtr;

    communicationManager = communicationPtr;

    sensorService = sensorPtr;

    failureManager = failurePtr;


    ResumableService::begin(

        resumePtr,

        checkpointPtr,

        TASK_COMMUNICATION

    );

}






//====================================================
// Normal execution
//====================================================


void CommunicationService::executeNormal()
{

    if(
    failureManager != nullptr &&
    failureManager->hasFailure()
)
{

    if(!interrupted)
        {

            Serial.println(
                "[COMM] Paused by failure"
            );


            pauseExecution();

        }


        return;

    }


    if(
        buffer == nullptr ||
        communicationManager == nullptr
    )
    {

        return;

    }

    //--------------------------------------------------
    // STEP 1
    // Create packet
    //--------------------------------------------------


    updateCheckpoint(

        STEP_COMMUNICATION_CREATE,

        20

    );



    createPacket();





    //--------------------------------------------------
    // STEP 2
    // Prepare packet
    //--------------------------------------------------


    updateCheckpoint(

        STEP_COMMUNICATION_PREPARE,

        40

    );



    CommunicationPacket* packet =
        buffer->front();



    if(packet == nullptr)
    {
        return;
    }



    if(
        !preparePacket(packet)
    )
    {

        return;

    }






    //--------------------------------------------------
    // STEP 3
    // Send
    //--------------------------------------------------


    updateCheckpoint(

        STEP_COMMUNICATION_SEND,

        60

    );



    if(
        !sendPacket(packet)
    )
    {

        return;

    }






    //--------------------------------------------------
    // STEP 4
    // Wait ACK
    //--------------------------------------------------


    updateCheckpoint(

        STEP_COMMUNICATION_WAIT_ACK,

        80

    );



    if(
        !waitACK(packet)
    )
    {

        return;

    }





    //--------------------------------------------------
    // STEP 5
    // Confirm
    //--------------------------------------------------


    updateCheckpoint(

        STEP_COMMUNICATION_CONFIRM,

        95

    );



    confirmPacket(packet);




    finishExecution();


}









//====================================================
// Resume execution
//====================================================


void CommunicationService::executeResume()
{

    uint8_t checkpoint =
        getCheckpoint();


    Serial.print(
        "[COMM Resume] Checkpoint: "
    );

    Serial.println(checkpoint);



    CommunicationPacket* packet =
        nullptr;



    if(
        checkpoint != STEP_COMMUNICATION_CREATE
    )
    {

        packet =
            buffer->front();


        if(packet == nullptr)
        {
            Serial.println(
                "[COMM Resume] No packet"
            );

            return;
        }

    }



    switch(checkpoint)
    {


        case STEP_COMMUNICATION_CREATE:
        {

            createPacket();

            packet =
                buffer->front();


            if(packet == nullptr)
            {
                return;
            }

        }


        case STEP_COMMUNICATION_PREPARE:
        {

            preparePacket(packet);

        }


        case STEP_COMMUNICATION_SEND:
        {

            if(!sendPacket(packet))
            {

                Serial.println(
                    "[COMM Resume] Send failed"
                );

                return;

            }

        }


        case STEP_COMMUNICATION_WAIT_ACK:
        {

            if(!waitACK(packet))
            {

                Serial.println(
                    "[COMM Resume] ACK timeout"
                );

                return;

            }

        }


        case STEP_COMMUNICATION_CONFIRM:
        {

            confirmPacket(packet);

        }

        break;



        default:
        {

            Serial.println(
                "[COMM Resume] Invalid checkpoint"
            );

            return;

        }


    }


    finishExecution();

}


//====================================================
// Packet creation
//====================================================


void CommunicationService::createPacket()
{


    CommunicationPacket packet;



    packetCounter++;



    packet.packetID =
        packetCounter;



    packet.timestamp =
        millis();



    packet.status =
        PACKET_PENDING;



    packet.retries = 0;



    packet.ackStatus = ACK_NONE;

    packet.bytesSent = 0;

    packet.encrypted = false;



    packet.checkpoint =
        STEP_COMMUNICATION_CREATE;




    float value = 0;


    if(sensorService != nullptr)
    {
        value =
            sensorService->getLastValue();
    }



    char message[32];


    snprintf(
        message,
        sizeof(message),
        "TEMP: %.2f",
        value
    );



    packet.length =
        strlen(message);



    memcpy(
        packet.payload,
        message,
        packet.length
    );





    buffer->push(
        packet
    );





    Serial.print(
        "[COMM] Packet created ID: "
    );


    Serial.println(
        packet.packetID
    );


}








//====================================================
// Prepare
//====================================================


bool CommunicationService::preparePacket(
    CommunicationPacket* packet
)
{


    if(packet == nullptr)
    {

        return false;

    }




    packet->status =
        PACKET_READY;



    packet->checkpoint =
        STEP_COMMUNICATION_PREPARE;



    return true;

}









//====================================================
// Send
//====================================================


bool CommunicationService::sendPacket(
    CommunicationPacket* packet
)
{

    if(packet == nullptr)
    {
        return false;
    }



    //--------------------------------------------------
    // Save communication state before transmission
    //--------------------------------------------------

    packet->status =
        PACKET_WAITING_ACK;


    packet->ackStatus =
        ACK_WAITING;


    packet->retries++;

    updateCheckpoint(
        STEP_COMMUNICATION_SEND,
        60
    );

    bool result =
        communicationManager->send(
            packet->payload,
            packet->length
        );



    if(result)
    {

        packet->checkpoint =
            STEP_COMMUNICATION_WAIT_ACK;


    }
    else
    {

        packet->ackStatus =
            ACK_FAILED;

    }



    return result;

}








//====================================================
// ACK
//====================================================


bool CommunicationService::waitACK(
    CommunicationPacket* packet
)
{

    if(packet == nullptr)
    {
        return false;
    }


    /*
        Simulación temporal.

        Posteriormente:
        - leer ACK por Serial
        - Bluetooth
        - WiFi
        - LoRa
    */


    bool simulatedACK = true;



    if(simulatedACK)
    {

        packet->ackStatus = ACK_RECEIVED;

        packet->status =
            PACKET_WAITING_ACK;


        return true;

    }



    packet->ackStatus = ACK_FAILED;


    return false;

}








//====================================================
// Confirm
//====================================================


void CommunicationService::confirmPacket(
    CommunicationPacket* packet
)
{

    if(packet == nullptr)
    {
        return;
    }


    uint32_t id =
        packet->packetID;



    if(packet->ackStatus != ACK_RECEIVED)
    {

        Serial.println(
            "[COMM] ACK missing"
        );


        packet->status =
            PACKET_WAITING_ACK;


        return;

    }



    packet->status =
        PACKET_CONFIRMED;


    packet->checkpoint =
        STEP_COMMUNICATION_CONFIRM;

    updateCheckpoint(
        STEP_COMPLETE,
        100
    );

    Serial.print(
        "[COMM] Packet confirmed: "
    );


    Serial.println(id);

    buffer->pop();

}
    #ifndef COMMUNICATION_SERVICE_H
    #define COMMUNICATION_SERVICE_H


    #include <Arduino.h>


    #include "services/ResumeableService.h"

    #include "services/CommunicationBuffer.h"

    #include "kernel/CommunicationPacket.h"

    #include "kernel/CommunicationManager.h"

    #include "kernel/ExecutionSteps.h"

    #include "services/SensorService.h"

    #include "services/FailureManager.h"



    class CommunicationService :
        public ResumableService
    {


    private:


        CommunicationBuffer* buffer;

        CommunicationManager* communicationManager;

        SensorService* sensorService;

        PersistentState* persistentState;


        //--------------------------------------------------
        // Idempotencia para DATA entrante (del nodo remoto).
        // Mismo patrón que usa PersistentOS2 para el DATA que
        // le llega de PersistentOS1.
        //--------------------------------------------------

        uint32_t receivedIds[8];

        uint8_t receivedCount;

        uint8_t receivedNextSlot;

    private:


        void createPacket();


        bool preparePacket(
            CommunicationPacket* packet
        );


        bool sendPacket(
            CommunicationPacket* packet
        );


        bool waitACK(
            CommunicationPacket* packet
        );


        void confirmPacket(
            CommunicationPacket* packet
        );


        //--------------------------------------------------
        // Recepción asíncrona (independiente del checkpoint
        // de envío propio): DATA entrante del nodo remoto y
        // ACK entrante para lo que este nodo envió.
        //--------------------------------------------------

        void handleIncomingData(
            char* line
        );


        void handleIncomingAck(
            uint32_t ackId
        );


        void sendAckFor(
            uint32_t packetId
        );


        bool wasAlreadyReceived(
            uint32_t packetId
        );


        void rememberReceived(
            uint32_t packetId
        );



    public:


        CommunicationService();



        void begin(

            CommunicationBuffer* bufferPtr,

            CommunicationManager* communicationPtr,

            ResumeManager* resumePtr,

            ExecutionCheckpoint* checkpointPtr,

            SensorService* sensorPtr,

            FailureManager* failurePtr,

            PersistentState* persistentStatePtr

        );


        //--------------------------------------------------
        // Drena y procesa todo lo que haya llegado por BT
        // (DATA remoto o ACK propio). Se llama en cada tick,
        // sin importar en qué checkpoint esté el envío propio,
        // para que este nodo sea emisor Y receptor a la vez.
        //--------------------------------------------------

        void pollIncoming();



    protected:


        void executeNormal() override;


        void executeResume() override;



    };



    #endif
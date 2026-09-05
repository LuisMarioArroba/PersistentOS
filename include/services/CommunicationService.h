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

    #include "kernel/EnergyManager.h"

    #include "kernel/EnergyPredictionManager.h"

    #include "kernel/BehaviorManager.h"



    class CommunicationService :
        public ResumableService
    {


    private:


        CommunicationBuffer* buffer;

        CommunicationManager* communicationManager;

        SensorService* sensorService;

        PersistentState* persistentState;


        //--------------------------------------------------
        // Energía y comportamiento: para incluir la energía
        // local en cada DATA saliente, alimentar la
        // predicción del lado remoto con lo que llega, y
        // decidir si el momento es favorable para comunicar.
        //--------------------------------------------------

        EnergyManager* energyManager;

        EnergyPredictionManager* localEnergyPrediction;

        EnergyPredictionManager* remoteEnergyPrediction;

        BehaviorManager* behaviorManager;


        //--------------------------------------------------
        // Idempotencia para DATA entrante (del nodo remoto).
        // Mismo patrón que usa PersistentOS2 para el DATA que
        // le llega de PersistentOS1.
        //--------------------------------------------------

        uint32_t receivedIds[8];

        uint8_t receivedCount;

        uint8_t receivedNextSlot;


        //--------------------------------------------------
        // Alarma pendiente: AlarmManager la deja aquí vía
        // requestAlarm() y createPacket() la consume antes
        // que cualquier telemetría de rutina, sin reordenar
        // el buffer mientras un paquete está en vuelo.
        //--------------------------------------------------

        bool alarmPending;

        char alarmPayload[MAX_PACKET_SIZE];

        uint16_t alarmPayloadLength;


        //--------------------------------------------------
        // Nodos conocidos: última temperatura reportada por
        // cada nodo (incluido este mismo), obtenida
        // localmente o recibida -- directa o transitivamente
        // vía el campo "K:" de un DATA ajeno -- de otro nodo.
        // El objetivo es que, con suficientes rotaciones de
        // enlace, todos los nodos terminen conociendo la
        // última lectura de todos los demás, no solo la de
        // su par directo.
        //--------------------------------------------------

        struct KnownNode
        {
            uint8_t nodeId;
            float temperature;
            bool valid;
            uint32_t lastSeen;
        };

        static const uint8_t MAX_KNOWN_NODES = 4;

        KnownNode knownNodes[MAX_KNOWN_NODES];


        //--------------------------------------------------
        // Alarmas ya reenviadas (por nodo de origen + tipo),
        // para no reenviar la misma alarma en cada rotación
        // de enlace ni generar un rebote entre nodos.
        //--------------------------------------------------

        struct RelayedAlarm
        {
            uint8_t originNodeId;
            uint8_t alarmType;
            uint32_t timestamp;
        };

        static const uint8_t MAX_RELAYED_ALARMS = 4;

        RelayedAlarm relayedAlarms[MAX_RELAYED_ALARMS];

    private:


        bool createPacket();


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
        // Predicción de ventana de comunicación: ningún nodo
        // conoce el perfil real de su fuente de alimentación,
        // así que esto solo mira energía local observada y la
        // última energía remota conocida (y sus tendencias).
        //--------------------------------------------------

        bool isCommunicationWindowFavorable();


        //--------------------------------------------------
        // Recepción asíncrona (independiente del checkpoint
        // de envío propio): DATA/ALARM entrante del nodo
        // remoto y ACK entrante para lo que este nodo envió.
        //--------------------------------------------------

        void handleIncomingData(
            char* line,

            bool isAlarm
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


        //--------------------------------------------------
        // Tabla de nodos conocidos (identificación de
        // origen y propagación transitiva de temperatura).
        //--------------------------------------------------

        void updateKnownNode(
            uint8_t nodeId,
            float temperature,
            bool valid
        );


        void buildKnownSummary(
            char* buffer,
            size_t bufferSize
        );


        void parseKnownSummary(
            const char* summary
        );


        //--------------------------------------------------
        // Relevo de alarmas ajenas: si esta alarma no es de
        // este nodo y no se reenvió todavía, se vuelve a
        // encolar como prioritaria para que llegue al
        // siguiente salto (primary o secondary).
        //--------------------------------------------------

        bool wasAlreadyRelayed(
            uint8_t originNodeId,
            uint8_t alarmType
        );


        void rememberRelayed(
            uint8_t originNodeId,
            uint8_t alarmType
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

            PersistentState* persistentStatePtr,

            EnergyManager* energyManagerPtr,

            EnergyPredictionManager* localEnergyPredictionPtr,

            EnergyPredictionManager* remoteEnergyPredictionPtr,

            BehaviorManager* behaviorManagerPtr

        );


        //--------------------------------------------------
        // Drena y procesa todo lo que haya llegado por BT
        // (DATA remoto o ACK propio). Se llama en cada tick,
        // sin importar en qué checkpoint esté el envío propio,
        // para que este nodo sea emisor Y receptor a la vez.
        //--------------------------------------------------

        void pollIncoming();


        //--------------------------------------------------
        // Llamado por AlarmManager cuando la temperatura
        // cruza un umbral. El próximo paquete que este nodo
        // cree será esta alarma, antes que cualquier
        // telemetría de rutina pendiente.
        //--------------------------------------------------

        void requestAlarm(

            const char* payload,

            uint16_t length

        );



    protected:


        void executeNormal() override;


        void executeResume() override;



    };



    #endif
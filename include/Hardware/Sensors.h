#ifndef SENSORS_H
#define SENSORS_H

#include <Arduino.h>


class Sensor
{

public:

    Sensor();


    bool begin();


    bool update();


    float getValue() const;


    bool isConnected() const;



private:

    float currentValue;

    bool connected;

};


#endif
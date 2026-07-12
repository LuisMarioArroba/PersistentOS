#ifndef SENSOR_H
#define SENSOR_H

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
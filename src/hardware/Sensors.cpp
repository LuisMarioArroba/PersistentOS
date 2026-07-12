#include "hardware/Sensors.h"
#include "config/Config.h"

#include <OneWire.h>
#include <DallasTemperature.h>

/*
Sensor Global definition
*/
OneWire oneWire(ONE_WIRE_BUS);

DallasTemperature sensors(&oneWire);
Sensor::Sensor()
{
    currentValue = 0.0;

    connected = false;
}

bool Sensor::begin(){
    sensors.begin();
    connected = (sensors.getDeviceCount() > 0);
    return connected;
}
bool Sensor::update(){
    if(!connected)
        return false;
    sensors.requestTemperatures();
    currentValue = sensors.getTempCByIndex(0);
    return true;
}
float Sensor::getValue() const{
    return currentValue;
}
bool Sensor::isConnected() const{
    return connected;
}
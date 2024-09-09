#pragma once

#include "Arduino.h"
#include "Errors.h"
#include "Updatable.h"
//new
class VoltageMeter:public Updatable
{
private:
    int pin;
    float maxVoltage;
    float voltage;
    float coef;
public:
    VoltageMeter(int setPin, float setCoef, float setMaxVoltage )
    {
        pin = setPin;
        maxVoltage = setMaxVoltage;
        coef = setCoef;
        pinMode(pin, INPUT);

    }
    float getVoltage()
    {
        return voltage;
    }
    float getPercentage()
    {
        return (maxVoltage - voltage)/(maxVoltage)*100.0;
    }
    ERROR_TYPE update() override
    {

        voltage = analogRead(pin)*coef;
        return NO_ERRORS;
    }
};
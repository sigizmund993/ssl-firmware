#pragma once

#include "Arduino.h"
#include "Errors.h"
#include "Updatable.h"
//new
class BallSensor:public Updatable
{
public:
    BallSensor(int setPin, uint16_t setThresh){
        this ->pin = setPin;
        this ->threshold = setThresh;
        pinMode(pin, INPUT);
    }
    bool isBallIn(){
        return analog<threshold;
    }
    uint16_t getAnalogValue(){
        return analog;
    }
    ERROR_TYPE update() override{
        analog = analogRead(pin);
        return NO_ERRORS;
    }
    
private:
    int pin;
    uint16_t analog, threshold;
};
#pragma once
//GOIDAAAAAAA
#include "Arduino.h"
#include "Errors.h"
#include "Updatable.h"
// new
class Button : public Updatable
{
private:
    int pin;
    bool isReleased, isPressed;
    bool state, prevstate;

public:
    Button(int setPin)
    {
        this->pin = setPin;
        pinMode(pin, INPUT);
    }
    ERROR_TYPE update() override
    {
        state = digitalRead(pin);
        isReleased = prevstate && state != prevstate;
        isPressed = !prevstate && state != prevstate;
        prevstate = state;
        return NO_ERRORS;
    }
    bool getState()
    {
        return state;
    }
    bool isButtonPressed()
    {
        return isPressed;
    }
    bool isButtonReleased()
    {
        return isReleased;
    }
};
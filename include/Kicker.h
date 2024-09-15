#pragma once

#include "Arduino.h"
#include "Updatable.h"
//new
class Kicker:public Updatable
{
public:
    Kicker(int setPin,uint32_t setKickLen, uint32_t setKickCD)
    {
        this -> pin = setPin;
        this ->lastKickTime = 0;
        this ->kickCD = setKickCD;
        this ->kickLen = setKickLen;
        pinMode(pin, OUTPUT);
    }
    void kick()
    {
        doKick = 1;
    }
    ERROR_TYPE update() override
    {   

        if(doKick)
        {
            
            if(millis() < lastKickTime + kickCD)
            {
                doKick = 0;
                return KICKER_WAIT_TIMEOUT;
            }   
            digitalWrite(pin, HIGH);
            lastKickTime = millis();
            doKick = 0;
        }
        if(millis() > lastKickTime + kickLen)
        {
            digitalWrite(pin, LOW);
        }
        return NO_ERRORS;
        
    }
private:
    int pin;
    bool doKick;
    uint32_t lastKickTime;
    uint32_t kickCD;
    uint32_t kickLen;
};
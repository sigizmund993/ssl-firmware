#pragma once
//GOIDAAAAAAA
#include "Arduino.h"
#include "Errors.h"
#include "Updatable.h"
#include "Tau.h"
#define PPR 210.0*2
#define VOLTS_PER_RAD_S 0.84
#define MOTOR_MAX_VOLTAGE 12.0
#define TS_S 0.005
#define MOTOR_MOVE_VOLTAGE 1
class Motor : public Updatable
{
private:
    int pin1, pin2;
    float pwm1, pwm2;
    int encPin;
    volatile long int encCounter;
    volatile double ang, angTgt, speedR, prevAng;
    PIreg piReg;

public:
    Motor(int pin1, int pin2, int encPin)
        : piReg(TS_S,0.7,1,MOTOR_MAX_VOLTAGE)
    {
        this->pin1 = pin1;
        this->pin2 = pin2;
        this->encPin = encPin;
        pinMode(pin1, OUTPUT);
        pinMode(pin2, OUTPUT);
        pinMode(encPin, INPUT);
    }
    void handler()
    {
        if (digitalRead(encPin))
            encCounter++;
        else
            encCounter--;
        ang = encCounter / PPR * M_PI;
        
    }
    void applyVoltage(float u)
    {
        if (abs(u) < MOTOR_MOVE_VOLTAGE)
            u = 0;
        float pwm = constrain(255.0 * u / MOTOR_MAX_VOLTAGE, -255, 255);
        if (pwm > 0)
        {
            pwm1 = 255;
            pwm2 = 255 - pwm;
        }
        else
        {
            pwm1 = 255 + pwm;
            pwm2 = 255;
        }
        analogWrite(pin1, pwm1);
        analogWrite(pin2, pwm2);
    }
    ERROR_TYPE update() override
    {
        angTgt += speedR*TS_S;
        
        // Serial.println(angTgt);
        Serial.println(ang);
        Serial.println(encCounter);
        Serial.println(millis());
        float u = angTgt - ang;

        u*=10;
        applyVoltage(u);
        
        prevAng = ang;
        return NO_ERRORS;
        
    }
    void setSpeed(float speed)
    {
        speedR = speed;//RAD/S
    }
};
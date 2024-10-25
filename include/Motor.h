#pragma once

#include "Arduino.h"
#include "Errors.h"
#include "Updatable.h"
#include "Tau.h"
class Motor : public Updatable
{
private:
    int pin1, pin2, pwm1, pwm2;
    int encPin;
    int encCounter, p2r;
    float maxU, maxSpd, radS2volt, timeSec, gain, ki, alpha, alphaPrev, speedEnc, speedTgt;
    float gearRatio;
    PIreg piReg;
    RateLimiter accelerationLim;
    RateLimiter voltChangeLim;
    FOD spdFilt;

public:
    Motor(int pin1, int pin2, int encPin, float maxU, float maxSpd, float radS2volt, int p2r, float timeSec, float gain, float ki, float gearRatio)
        : piReg(timeSec, gain, ki, maxU),
          accelerationLim(timeSec, 9999),
          voltChangeLim(timeSec, 9999),
          spdFilt(timeSec, timeSec * 2, true)
    {
        this->gearRatio = gearRatio;
        this->pin1 = pin1;
        this->pin2 = pin2;
        this->encPin = encPin;
        this->maxU = maxU;
        this->maxSpd = maxSpd;       //
        this->radS2volt = radS2volt; //
        this->p2r = p2r;             //
        this->timeSec = timeSec;
        this->gain = gain;
        this->ki = ki;

        pinMode(pin1, OUTPUT);
        pinMode(pin2, OUTPUT);
        pinMode(encPin, INPUT);
    }
    void handler()
    {   
        // Serial.println(encCounter);
        if (digitalRead(encPin))
            encCounter++;
        else
            encCounter--;
    }
    void applyVoltage(float u)
    {
        if (abs(u) >= 1.5)
        {
            float pwm = constrain(255.0 * voltChangeLim.tick(u) / maxU, -255, 255);
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
        else
        {
            analogWrite(pin1, 255);
            analogWrite(pin2, 255);
        }
    }
    ERROR_TYPE update() override
    {
        float cntInc = encCounter;
        encCounter = 0;

        alpha += cntInc / p2r * M_PI * 2;
        speedEnc = spdFilt.tick(alpha);
        float u = speedTgt * radS2volt + piReg.tick(speedTgt - speedEnc);
        // Serial.println(u);
        applyVoltage(u);

        alphaPrev = alpha;
        return NO_ERRORS;
    }
    void setSpeed(float speed)
    {
        speedTgt = accelerationLim.tick(constrain(speed, -maxSpd, maxSpd)); // rad/s
    }
    float getAngle()
    {
        return alpha;
    }
    float getSpeedEnc()
    {
        return speedEnc;
    }
};
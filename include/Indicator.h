#pragma once

#include "Arduino.h"
#include "Errors.h"
#include "Updatable.h"
// New
//  Segments location:
//        ----A----
//        |       |
//        F       B
//        |       |
//        ----G----
//        |       |
//        E       C
//        |       |
//        ----D----  dot
//        ----0----
//        |       |
//        5       1
//        |       |
//        ----6----
//        |       |
//        4       2
//        |       |
//        ----3----  7
//  A0 B1 C2 D3 E4 F5 G6 Dot7
class Indicator : public Updatable
{
public:
    Indicator(int pinA, int pinB, int pinC, int pinD, int pinE, int pinF, int pinG, int pinDot)
    {
        pins[0] = pinA;
        pins[1] = pinB;
        pins[2] = pinC;
        pins[3] = pinD;
        pins[4] = pinE;
        pins[5] = pinF;
        pins[6] = pinG;
        pins[7] = pinDot;
        for (int i = 0; i < sizeof(pins); i++)
        {
            pinMode(pins[i], OUTPUT);
        }
    }
    void clear()
    {
        for (int i = 0; i < sizeof(pins) - 1; i++)
            state[i] = 0;
    }
    void drawH()
    {
        for (int i = 0; i < sizeof(pins) - 1; i++)
        {
            if (i == 1 || i == 2 || i == 4 || i == 5 || i == 6)
                state[i] = 1;
            else
                state[i] = 0;
        }
    }
    void drawL()
    {
        for (int i = 0; i < sizeof(pins) - 1; i++)
        {
            if (i == 3 || i == 4 || i == 5)
                state[i] = 1;
            else
                state[i] = 0;
        }
    }
    void drawDash()
    {
        for (int i = 0; i < sizeof(pins) - 1; i++)
        {
            if (i == 6)
                state[i] = 1;
            else
                state[i] = 0;
        }
    }
    void drawC()
    {
        for (int i = 0; i < sizeof(pins) - 1; i++)
        {
            if (i == 0 || i == 3 || i == 4 || i == 5 || i == 7)
                state[i] = 1;
            else
                state[i] = 0;
        }
    }
    void drawR()
    {
        for (int i = 0; i < sizeof(pins) - 1; i++)
        {
            if (i == 2 || i == 3 || i == 7)
                state[i] = 0;
            else
                state[i] = 0;
        }
    }
    void drawDot()
    {
        state[7] = 1;
    }
    void clearDot()
    {
        state[7] = 0;
    }
    void drawNumber(uint8_t N)
    {

        if (N == 0)
        {
            for (int i = 0; i < sizeof(pins) - 1; i++)
            {
                if (i == 7 || i == 6)
                    state[i] = 0;
                else
                    state[i] = 1;
            }
        }
        if (N == 1)
        {
            for (int i = 0; i < sizeof(pins) - 1; i++)
            {
                if (i == 1 || i == 2)
                    state[i] = 1;
                else
                    state[i] = 0;
            }
        }
        if (N == 2)
        {
            for (int i = 0; i < sizeof(pins) - 1; i++)
            {
                if (i == 5 || i == 2 || i == 7)
                    state[i] = 0;
                else
                    state[i] = 1;
            }
        }
        if (N == 3)
        {
            for (int i = 0; i < sizeof(pins) - 1; i++)
            {
                if (i == 5 || i == 4 || i == 7)
                    state[i] = 0;
                else
                    state[i] = 1;
            }
        }
        if (N == 4)
        {
            for (int i = 0; i < sizeof(pins) - 1; i++)
            {
                if (i == 0 || i == 3 || i == 4 || i == 7)
                    state[i] = 0;
                else
                    state[i] = 1;
            }
        }
        if (N == 5)
        {
            for (int i = 0; i < sizeof(pins) - 1; i++)
            {
                if (i == 7 || i == 1 || i == 4)
                    state[i] = 0;
                else
                    state[i] = 1;
            }
        }
        if (N == 6)
        {
            for (int i = 0; i < sizeof(pins) - 1; i++)
            {
                if (i == 7 || i == 1)
                    state[i] = 0;
                else
                    state[i] = 1;
            }
        }
        if (N == 7)
        {
            for (int i = 0; i < sizeof(pins) - 1; i++)
            {
                if (i == 0 || i == 1 || i == 2)
                    state[i] = 1;
                else
                    state[i] = 0;
            }
        }
        if (N == 8)
        {
            for (int i = 0; i < sizeof(pins) - 1; i++)
            {
                if (i == 7)
                    state[i] = 0;
                else
                    state[i] = 1;
            }
        }
        if (N == 9)
        {
            for (int i = 0; i < sizeof(pins) - 1; i++)
            {
                if (i == 7 || i == 4)
                    state[i] = 0;
                else
                    state[i] = 1;
            }
        }
        if (N > 9)
        {
            if (millis() % 1000 < 500)
                drawNumber((N - N % 10) / 10);
            else
                drawNumber(N % 10);
        }
    }
    ERROR_TYPE update() override
    {
        for (int i = 0; i < sizeof(pins) - 1; i++)
        {
            digitalWrite(pins[i], state[i]);
        }
        return NO_ERRORS;
    }

private:
    int pins[8] = {0};
    int state[8] = {0};
};
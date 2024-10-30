#include "Arduino.h"
#include "BallSensor.h"
#include "Button.h"
#include "ConnectionList.h"
#include "EEPROM.h"
#include "Indicator.h"
#include "Motor.h"
#include "VoltageMeter.h"
#include <SPI.h>
#include "RF24.h"
#include "NRF24.h"
#include "Kicker.h"
#include "IMU.h"
#define GEAR_RATIO 1.0
#define PPR 320.0
#define VOLTS_PER_RAD_S 0.84
#define MOTOR_MAX_VOLTAGE 12.0
#define MOTOR_MAX_SPEED 30.0
#define TS_S 0.005
#define TS_MCS 5000
#define MOTORS_ROBOT_RAD_MM 82.0
#define ROBOT_MAX_SPEED 1500.0
#define WHEEL_RAD_MM 23.5
Button buttonPlus(BUTTON_CHANEL_PLUS);
Button buttonMinus(BUTTON_CHANEL_MINUS);
Button buttonEnter(BUTTON_ENTER);
Kicker kicker(KICKER, 50, 2000);
IMU imu(IMU_CHIP_SELECT);
Motor motor1(MOTOR1_IN2, MOTOR1_IN1, MOTOR1_ENCB_PIN);
Motor motor2(MOTOR2_IN2, MOTOR2_IN1, MOTOR2_ENCB_PIN);
Motor motor3(MOTOR3_IN2, MOTOR3_IN1, MOTOR3_ENCB_PIN);
Motor mot(1000,1000,1000);
Indicator indicator(INDICATOR_A, INDICATOR_B, INDICATOR_C, INDICATOR_D, INDICATOR_E, INDICATOR_F, INDICATOR_G, INDICATOR_DOT);
VoltageMeter voltMeter(BATTERY_VOLTAGE, 5 * 2.5 / 1024.0, 12.4);
BallSensor ballSensor(BALL_SENSOR, 20);
NRF nrf(NRF_CHIP_ENABLE, NRF_CHIP_SELECT);
float calcMototVel(int motorN, float sXmms, float sYmms, float sWrads)
{
    sXmms = constrain(sXmms, -1500, 1500);
    sYmms = constrain(sYmms, -1500, 1500);
    sWrads = constrain(sWrads, -8, 8);
    float globalSpeed = sqrt(sXmms * sXmms + sYmms * sYmms);
    float sWmms = sWrads * 90.0;
    float angle = atan2(sXmms, -sYmms);
    float s1Rad = (sWmms + sin(angle - 0.33 * M_PI) * globalSpeed) / WHEEL_RAD_MM;
    float s2Rad = (sWmms + sin(angle - M_PI) * globalSpeed) / WHEEL_RAD_MM;
    float s3Rad = (sWmms + sin(angle + 0.33 * M_PI) * globalSpeed) / WHEEL_RAD_MM;

    switch (motorN)
    {
    case 1:
        return s1Rad;
        break;
    case 2:
        return s2Rad;
        break;
    case 3:
        return s3Rad;
        break;
    default:
        return 0;
        break;
    }
}
void updIN()
{
    buttonEnter.update();
    buttonMinus.update();
    buttonPlus.update();
    imu.update();
    voltMeter.update();
    ballSensor.update();
    nrf.update();
}
void updOUT()
{
    kicker.update();
    motor1.update();
    motor2.update();
    motor3.update();
    indicator.update();
}
int NRFchannel = 0;
bool initSuccess = false;
void setup()
{
    imu.init();
    if (nrf.init() == NRF_CONNECTION_ERROR)
    {

        indicator.drawC();
        indicator.update();
    }
    else
    {
        NRFchannel = EEPROM.read(0);
        attachInterrupt(MOTOR1_ENCA_CH, []()
                        { motor1.handler(); }, RISING);
        attachInterrupt(MOTOR2_ENCA_CH, []()
                        { motor2.handler(); }, RISING);
        attachInterrupt(MOTOR3_ENCA_CH, []()
                        { motor3.handler(); }, RISING);
        Serial.begin(115200);
        initSuccess = true;
    }
}

unsigned long long int lastUpdate = 0;
unsigned long long int lastAutokick = 0;
Integrator yawInt(TS_S);
Integrator pitchInt(TS_S);
Integrator rollInt(TS_S);
RateLimiter sXlim(TS_S, 8000);
RateLimiter sYlim(TS_S, 8000);
bool flagY = false;
float pitch = 0;
float roll = 0;
bool autoKick = 0;

float sXmms = 0;
float sYmms = 0;
float sWrads = 0;
void loop()
{
    // t

    while (micros() < lastUpdate + TS_MCS);
    lastUpdate = micros();
    // s
    nrf.setChannel(NRFchannel);
    updIN();
    pitch = pitchInt.tick(imu.getPitch());
    roll = rollInt.tick(imu.getRoll());
    // Serial.println(autoKick);
    // p

    if (buttonPlus.isButtonReleased())
    {
        NRFchannel++;
        // if(NRFchannel >= 10)
        //     NRFchannel = 0;
        EEPROM.write(0, NRFchannel);
    }
    if (buttonMinus.isButtonReleased())
    {
        NRFchannel--;
        // if(NRFchannel <= -1)
        //     NRFchannel = 9;
        EEPROM.write(0, NRFchannel);
    }
    if (nrf.avalible())
        digitalWrite(LED_GREEN, 1);
    else
        digitalWrite(LED_GREEN, 1);
    indicator.drawNumber(NRFchannel);

    if (voltMeter.getVoltage() < 9.6)
    {
        Serial.println("BATTERY CRITICAL");
        indicator.drawL();
    }
    else if (voltMeter.getVoltage() < 11.1 && millis() % 1000 <= 500)
    {
        Serial.println("BATTERY LOW");
        indicator.drawL();
    }

    else
    {
        // battery is NOT low
        if (buttonEnter.isButtonReleased())
            kicker.kick();

        if (NRFchannel == nrf.getadress() - 16)
        {
            // Kick
            if (nrf.kickFlag())
            {

                indicator.drawDash();
                kicker.kick();
            }
            // Auto kick
            autoKick = nrf.autoKickFlag();
            sXmms = nrf.getsXmms();

            sYmms = nrf.getsYmms();
            sWrads = nrf.getsWrads();

            nrf.resetUpdate();
        }

        if (ballSensor.isBallIn() && autoKick && millis() - lastAutokick > 1000)
        {
            kicker.kick();
            indicator.drawDash();
            lastAutokick = millis();
        }
        sXmms = sXlim.tick(sXmms);
        sYmms = sYlim.tick(sYmms);
        sWrads += 8 * yawInt.tick(sWrads + imu.getYaw());
        if(abs(pitch)>1 || abs(roll) > 1)
        {
            sXmms = 0;
            sYmms = 0;
            sWrads = 0;
        }


        sWrads = constrain(sWrads, -7, 7);
        
        motor1.setSpeed(calcMototVel(1, sXmms, sYmms, sWrads));
        motor2.setSpeed(calcMototVel(2, sXmms, sYmms, sWrads));
        motor3.setSpeed(calcMototVel(3, sXmms, sYmms, sWrads));
        // analogWrite(MOTOR1_IN1, 255);
        // analogWrite(MOTOR1_IN2, 0);
        // analogWrite(MOTOR2_IN1, 255);
        // analogWrite(MOTOR2_IN2, 0);
        // analogWrite(MOTOR3_IN1, 255);
        // analogWrite(MOTOR3_IN2, 0);
    }
    // a
    if (autoKick)
        indicator.drawDot();
    else
        indicator.clearDot();
    digitalWrite(LED_GREEN, 0);
    if (!initSuccess && millis() % 1000 <= 500)
        digitalWrite(LED_GREEN, 1);
    updOUT();
    digitalWrite(LED_BLUE, ballSensor.isBallIn());
    Serial.println(pitch);
    Serial.println(roll);
}

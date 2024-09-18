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
#define PPR 320
#define VOLTS_PER_RAD_S 0.84
#define MOTOR_MAX_VOLTAGE 12.0
#define MOTOR_MAX_SPEED 30
#define TS_S 0.005
#define TS_MCS 5000
Button buttonPlus(BUTTON_CHANEL_PLUS);
Button buttonMinus(BUTTON_CHANEL_MINUS);
Button buttonEnter(BUTTON_ENTER);
Kicker kicker(KICKER,50,2000);
IMU imu(IMU_CHIP_SELECT);
///
MotorConnectionParams mconnp1 =
{
    .IN1 =      MOTOR1_IN1,
    .IN2 =      MOTOR1_IN2,
    .ENCA_PIN = MOTOR1_ENCA_PIN,
    .ENCA_CH =  MOTOR1_ENCA_CH,
    .ENCB_PIN = MOTOR1_ENCB_PIN,
    .ENCB_CH =  MOTOR1_ENCB_CH,
    .ENC_PPR =  48,
    .i =        29,
    .ke =       0.185,
    .ENC_PORT = &PINE,
    .ENC_MASK = 0b00110000,
    .ENC_SHIFT= 4,
    .ENC_DIR =  1,
};
MotorConnectionParams mconnp2 =
{
    .IN1 =      MOTOR2_IN1,
    .IN2 =      MOTOR2_IN2,
    .ENCA_PIN = MOTOR2_ENCA_PIN,
    .ENCA_CH =  MOTOR2_ENCA_CH,
    .ENCB_PIN = MOTOR2_ENCB_PIN,
    .ENCB_CH =  MOTOR2_ENCB_CH,
    .ENC_PPR =  48,
    .i =        29,
    .ke =       0.185,
    .ENC_PORT = &PIND,
    .ENC_MASK = 0b00001100,
    .ENC_SHIFT= 2,
    .ENC_DIR =  -1,
};
MotorConnectionParams mconnp3 =
{
    .IN1 =      MOTOR3_IN1,
    .IN2 =      MOTOR3_IN2,
    .ENCA_PIN = MOTOR3_ENCA_PIN,
    .ENCA_CH =  MOTOR3_ENCA_CH,
    .ENCB_PIN = MOTOR3_ENCB_PIN,
    .ENCB_CH =  MOTOR3_ENCB_CH,
    .ENC_PPR =  48,
    .i =        29,
    .ke =       0.185,
    .ENC_PORT = &PIND,
    .ENC_MASK = 0b00000011,
    .ENC_SHIFT= 0,
    .ENC_DIR =  -1,
};

MotorControllerParams mctrlp = 
{
    .maxU = 12.0,
    .moveU = 2,
    .maxSpeed = 30.0,
    .maxAccel = 9999,
    .Ts = TS_S,
    .kp = ((0.185/(2.0*((float)6000 * 0.000001)))*0.030),
    .ki = (0.185/(2.0*((float)6000 * 0.000001))),
    .speedFilterT = 2*TS_S,
    .maxUi = 9999
};

Motor motor1(&mconnp1, &mctrlp);
Motor motor2(&mconnp2, &mctrlp);
Motor motor3(&mconnp3, &mctrlp);
///
Indicator indicator(INDICATOR_A,INDICATOR_B,INDICATOR_C,INDICATOR_D,INDICATOR_E,INDICATOR_F,INDICATOR_G,INDICATOR_DOT);
VoltageMeter voltMeter(BATTERY_VOLTAGE,5 * 2.5 / 1024.0,12.4);
BallSensor ballSensor(BALL_SENSOR,20);
NRF nrf (NRF_CHIP_ENABLE,NRF_CHIP_SELECT);

float calcMototVel(int motorN,float sXmms,float sYmms,float sWrads)
{
    sXmms = constrain(sXmms,-1500,1500);
    sYmms = constrain(sYmms,-1500,1500);
    float globalSpeed = sqrt(sXmms*sXmms + sYmms*sYmms);
    float sWmms = sWrads*90.0;
    float angle = atan2(sXmms,-sYmms);
    float s1Rad =  (sWmms + sin(angle - 0.33 * M_PI) * globalSpeed)/25.0;
    float s2Rad =  (sWmms + sin(angle - M_PI) * globalSpeed)/25.0;
    float s3Rad =  (sWmms + sin(angle + 0.33 * M_PI) * globalSpeed)/25.0;
    
    
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
int NRFchannel =0;
bool initSuccess = false;
void setup()
{
    imu.init();
    if(nrf.init() == NRF_CONNECTION_ERROR)
    {
        
        indicator.drawC();
        indicator.update();
    }
    else
    {
    NRFchannel = EEPROM.read(0);
    attachInterrupt(MOTOR1_ENCA_CH, []()
                    { motor1.interruptHandler(); }, RISING);
    attachInterrupt(MOTOR2_ENCA_CH, []()
                    { motor2.interruptHandler(); }, RISING);
    attachInterrupt(MOTOR3_ENCA_CH, []()
                    { motor3.interruptHandler(); }, RISING);
    Serial.begin(115200);
    initSuccess = true;
    }
}

uint32_t lastUpdate = 0;
Integrator yawInt(TS_S);
Integrator pitchInt(TS_S);
Integrator rollInt(TS_S);
RateLimiter sXlim(TS_S, 400000);
RateLimiter sYlim(TS_S, 400000);
bool flagY = false;
float pitch = 0;
float roll = 0;
bool autoKick = 0;
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
    Serial.println(autoKick);
    //p
    

    if(buttonPlus.isButtonReleased())
    {
        NRFchannel++;
        EEPROM.write(0,NRFchannel);
    }
    if(buttonMinus.isButtonReleased())
    {
        NRFchannel--;
        EEPROM.write(0,NRFchannel);
    }
    if(nrf.avalible())
        digitalWrite(LED_GREEN,1);
    else 
        digitalWrite(LED_GREEN,1);
    indicator.drawNumber(NRFchannel);

    if(voltMeter.getVoltage()<9.6)
    {
        Serial.println("BATTERY CRITICAL");
        indicator.drawL();
    }
    else if(voltMeter.getVoltage()<11.1 && millis()%1000<=500)
    {
        Serial.println("BATTERY LOW");
        indicator.drawL();
    }
    else
    {
        //battery is NOT low
        if(buttonEnter.isButtonReleased())kicker.kick();
        
        float sXmms = 0;
        float sYmms = 0;
        float sWrads = 0;
        


        if (NRFchannel == nrf.getadress()-16)
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
        
        if(ballSensor.isBallIn() && autoKick)
        {
            kicker.kick();
            indicator.drawDash();
        }
        sXmms = sXlim.tick(sXmms);
        sYmms = sYlim.tick(sYmms);
        sWrads += 8*yawInt.tick(sWrads +imu.getYaw());
        if(abs(pitch)>1 || abs(roll) > 1)
        {
            sXmms = 0;
            sYmms = 0;
            sWrads = 0;
        }
        Serial.println(calcMototVel(1,sXmms,sYmms,sWrads));
        Serial.println(calcMototVel(2,sXmms,sYmms,sWrads));
        Serial.println(calcMototVel(3,sXmms,sYmms,sWrads));

        motor1.setSpeed(calcMototVel(1,sXmms,sYmms,sWrads));
        motor2.setSpeed(calcMototVel(2,sXmms,sYmms,sWrads));
        motor3.setSpeed(calcMototVel(3,sXmms,sYmms,sWrads));
    }
    //a
    if(autoKick)
        indicator.drawDot();
    else
        indicator.clearDot();
    digitalWrite(LED_GREEN,0);
    if(!initSuccess && millis()%1000<=500)digitalWrite(LED_GREEN,1);
    updOUT();
    digitalWrite(LED_BLUE,ballSensor.isBallIn());


   
}

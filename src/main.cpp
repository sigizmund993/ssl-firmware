#include <Arduino.h>
#include "ConnectionList.h"
#include "Tau.h"
#define MOTOR_MAX_VOLTAGE 12.0
#define MOTOR_MOVE_VOLTAGE 1.5
#define WHEEL_RAD_MM 23.5
#define MOTOR_PPR 320.0
#define TS_S 0.005
#define RADS2VOLT 0.84
// put function declarations here:
class Motor
{
private:
  int pin1, pin2, encPin;
  unsigned long long int encCnt, ang;
  PIreg piReg;
  float tgtPos;
public:
  Motor(int pin1, int pin2, int encPin): piReg(TS_S,0.7,1000,100500)
  {
    
    this->pin1 = pin1;
    this->pin2 = pin2;
    this->encPin = encPin;
    pinMode(pin1, OUTPUT);
    pinMode(pin2, OUTPUT);
    pinMode(encPin, INPUT);
    
  }
  
  void applyVoltage(float V)
  {
    V = constrain(V, -MOTOR_MAX_VOLTAGE, MOTOR_MAX_VOLTAGE);
    if (abs(V) <= MOTOR_MOVE_VOLTAGE)
      V = 0;
    float pwm = V / MOTOR_MAX_VOLTAGE * 255.0;
    float pwm1;
    float pwm2;

    if (pwm > 0)
    {
      pwm1 = 255;
      pwm2 = 255 - pwm;
    }
    else
    {
      pwm1 = 255;
      pwm2 = 255 - pwm;
    }
    analogWrite(pin1, pwm1);
    analogWrite(pin2, pwm2);
  }
  void handler()
  {
    if(digitalRead(encPin))
      encCnt++;
    else
      encCnt--;
    ang = encCnt/MOTOR_PPR*M_PI*2;
  }
  
  void move(float sRads)
  {
    
    tgtPos += sRads;
    applyVoltage(piReg.tick(tgtPos-ang)*RADS2VOLT);
  }
};
Motor motor1(MOTOR1_IN1,MOTOR1_IN2,MOTOR1_ENCB_PIN);
void setup()
{
  Serial.begin(115200);
  attachInterrupt(MOTOR1_ENCA_CH,[](){motor1.handler();},RISING);

}

void loop()
{
  motor1.move(M_PI*2);
}

#pragma once

#include <SPI.h>
#include "RF24.h"

#include "Arduino.h"
#include "Errors.h"
#include "Updatable.h"
#include "math.h"
#define NRF2MMS 10.0
#define NRF2RADS 0.06
class NRF : public Updatable
{
  private:
  RF24 rad;
  uint8_t adress;
  int8_t sX;
  int8_t sY;
  int8_t sW;
  uint8_t flags;
  bool isUpdated;
  int NRFchannel = 1;
  public:
  NRF(uint8_t chipEnable, uint8_t chipSelect) : rad(chipEnable,chipSelect){}
  ERROR_TYPE init()
  {
    if(!rad.begin())return NRF_CONNECTION_ERROR;
    if(!rad.setDataRate(RF24_2MBPS))return NRF_DATA_RATE_ERROR;
    rad.setPALevel(RF24_PA_MAX);
    rad.enableDynamicPayloads();
    rad.setCRCLength(RF24_CRC_16);
    rad.setAutoAck(1, false);

    rad.setAddressWidth(3);
    byte self_addr[]{0xAB, 0xAD, 0xAF};
    rad.openReadingPipe(1, self_addr);
    
    rad.startListening();
    rad.powerUp();

    return NO_ERRORS;
  }
  ERROR_TYPE update() override
  {
    while(rad.available())
    {
      byte recv[6];
      rad.read(&recv, 6);
      if(recv[0] == NRFchannel+16)
      {
        adress = recv[0];
        sX = recv[1];
        
        sY = recv[2];
        
        sW = recv[3];
        flags = recv[5];
        isUpdated = 1;
      }
      
    }
    return NO_ERRORS;
  }
  void setChannel(int ch)
  {
    NRFchannel = ch;
  }
  void resetUpdate()
  {
    isUpdated = 0;
  }
  bool avalible()
  {
    return !isUpdated;
  }
  uint8_t getadress()
  {
    return adress;
  }
  float getsXmms()
  {
    if(abs(sX)<15)
    { 
      if(sX>0)
        sX = 17;
      if(sX<0)
        sX = -17;
    }
    return sX * NRF2MMS;
  }
  float getsYmms()
  {
    if(abs(sY)<15)
    { 
      if(sY>0)
        sY = 17;
      if(sY<0)
        sY = -17;
    }
    return sY * NRF2MMS;
  }
  float getsWrads()
  {
    return sW * NRF2RADS;
  }
  bool kickFlag()
  {
    return flags & (0x01 << 6);
  }
  bool autoKickFlag()
  {
    return flags & (0x01 << 4);
  }
};
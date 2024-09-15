#pragma once

#include "Arduino.h"
#include "Errors.h"
#include "Updatable.h"

#include "MPU9250_WE.h"
//new
class IMU : public Updatable
{
public:
    IMU(int chipSelect) : mpu(&SPI, chipSelect, true) {}
    ERROR_TYPE init()
    {
        mpu.init();
        delay(1000);
        mpu.autoOffsets();
        delay(200);
        return NO_ERRORS;
    }
    ERROR_TYPE update() override;

    float getYaw() 
    { 
        return gyr.z; 
    }
    float getPitch() 
    { 
        return gyr.x; 
    }
    float getRoll() 
    { 
        return gyr.y; 
    }
protected:
    MPU9250_WE mpu;
    xyzFloat gyr;
    
};

ERROR_TYPE IMU::update()
{
  gyr = mpu.getGyrValues() * M_PI/180;
  return NO_ERRORS;
}
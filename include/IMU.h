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
        mpu.enableGyrDLPF();
        mpu.setGyrDLPF(MPU9250_DLPF_6);
        mpu.setSampleRateDivider(5);
        mpu.setGyrRange(MPU9250_GYRO_RANGE_250);
        mpu.setAccRange(MPU9250_ACC_RANGE_2G);
        mpu.enableAccDLPF(true);
        mpu.setAccDLPF(MPU9250_DLPF_6);
        mpu.setMagOpMode(AK8963_CONT_MODE_100HZ);
        delay(200);
        return NO_ERRORS;
    }
    ERROR_TYPE update() override;

    float getYaw() 
    { 
        float x = gyr.z;
        if(abs(x)<0.01)
            x = 0;
        return x; 
    }
    float getPitch() 
    { 
        float x = gyr.x;
        if(abs(x)<0.01)
            x = 0;
        return x;
    }
    float getRoll() 
    { 
        float x = gyr.y;
        if(abs(x)<0.01)
            x = 0;
        return x;
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
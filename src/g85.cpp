#include "GY_85.h"
#include <Arduino.h>
#include <ArduinoJson.h>


DynamicJsonDocument doc(1024);

DynamicJsonDocument* GY_85::toJson(){
  int *accelerometerReadings = readFromAccelerometer();
  int ax = accelerometer_x(accelerometerReadings);
  int ay = accelerometer_y(accelerometerReadings);
  int az = accelerometer_z(accelerometerReadings);

  int *compassReadings = readFromCompass();
  int cx = compass_x(compassReadings);
  int cy = compass_y(compassReadings);
  int cz = compass_z(compassReadings);

  float heading = atan2(cy / 0.92, cx / 0.92);

  // http://www.magnetic-declination.com/
  float declinationAngle = 0.09;
  heading += declinationAngle;

  while (heading < 0)
    heading += 2 * PI;

  while (heading > 2 * PI)
    heading -= 2 * PI;

  float *gyroReadings = readGyro();
  float gx = gyro_x(gyroReadings);
  float gy = gyro_y(gyroReadings);
  float gz = gyro_z(gyroReadings);
  float gt = temp(gyroReadings);

  doc["gyro"][0] = gx;
  doc["gyro"][1] = gy;
  doc["gyro"][2] = gz;

  doc["accel"][0] = ax;
  doc["accel"][1] = ay;

  doc["compassHeading"] = (int) heading * 180 / PI;

  doc["temperature"] = gt;

  return &doc;

}

void GY_85::SetAccelerometer()
{
    //Put the ADXL345 into +/- 4G range by writing the value 0x01 to the DATA_FORMAT register.
    Wire.beginTransmission( ADXL345 );      // start transmission to device
    Wire.write( 0x31 );                     // send register address
    Wire.write( 0x01 );                     // send value to write
    Wire.endTransmission();                 // end transmission
    
    //Put the ADXL345 into Measurement Mode by writing 0x08 to the POWER_CTL register.
    Wire.beginTransmission( ADXL345 );      // start transmission to device
    Wire.write( 0x2D );                     // send register address  //Power Control Register
    Wire.write( 0x08 );                     // send value to write
    Wire.endTransmission();                 // end transmission
    
}

int* GY_85::readFromAccelerometer()
{
    static int axis[3];
    int buff[6];
    
    Wire.beginTransmission( ADXL345 );      // start transmission to device
    Wire.write( DATAX0 );                   // sends address to read from
    Wire.endTransmission();                 // end transmission
    
    Wire.beginTransmission( ADXL345 );      // start transmission to device
    Wire.requestFrom( ADXL345, 6 );         // request 6 bytes from device
    
    uint8_t i = 0;
    while(Wire.available())                 // device may send less than requested (abnormal)
    {
        buff[i] = Wire.read();              // receive a byte
        i++;
    }
    Wire.endTransmission();                 // end transmission
    int16_t i0 = ((buff[1]) << 8) | buff[0];
    int16_t i1 = ((buff[3]) << 8) | buff[2];
    int16_t i2 = ((buff[5]) << 8) | buff[4];
    axis[0] = i0;
    axis[1] = i1;
    axis[2] = i2;
    // axis[0] |= buff[1][]
    // axis[0] &= 0x1111111101111111

    //     if(axis[i] > 32768){
    //         axis[i] = 65535 - axis[i];
    //     }
    // }
    
    return axis;
}
//----------------------------------------
void GY_85::SetCompass()
{
    //Put the HMC5883 IC into the correct operating mode
    Wire.beginTransmission( HMC5883 );      //open communication with HMC5883
    Wire.write( 0x02 );                     //select mode register
    Wire.write( 0x00 );                     //continuous measurement mode
    Wire.endTransmission();
}

int* GY_85::readFromCompass()
{
    static int axis[3];
    
    //Tell the HMC5883 where to begin reading data
    Wire.beginTransmission( HMC5883 );
    Wire.write( 0x03 );               //select register 3, X MSB register
    Wire.endTransmission();
    
    
    //Read data from each axis, 2 registers per axis
    Wire.requestFrom( HMC5883, 6 );

    int16_t i0, i1, i2;
    if(6<=Wire.available()){
        i0 = Wire.read()<<8;           //X msb
        i0 |= Wire.read();             //X lsb
        i2 = Wire.read()<<8;           //Z msb
        i2 |= Wire.read();             //Z lsb
        i1 = Wire.read()<<8;           //Y msb
        i1 |= Wire.read();             //Y lsb
    }
    axis[0] = i0;
    axis[1] = i1;
    axis[2] = i2;

    return axis;
}

//----------------------------------------

int g_offx = 0;
int g_offy = 0;
int g_offz = 0;

void GY_85::SetGyro()
{
    Wire.beginTransmission( ITG3200 );
    Wire.write( 0x3E );
    Wire.write( 0x00 );
    Wire.endTransmission();
    
    Wire.beginTransmission( ITG3200 );
    Wire.write( 0x15 );
    Wire.write( 0x07 );
    Wire.endTransmission();
    
    Wire.beginTransmission( ITG3200 );
    Wire.write( 0x16 );
    Wire.write( 0x1E );                         // +/- 2000 dgrs/sec, 1KHz, 1E, 19
    Wire.endTransmission();
    
    Wire.beginTransmission( ITG3200 );
    Wire.write( 0x17 );
    Wire.write( 0x00 );
    Wire.endTransmission();
    
    delay(10);
    
    GyroCalibrate();
}

void GY_85::GyroCalibrate()
{
    static int tmpx = 0;
    static int tmpy = 0;
    static int tmpz = 0;
    
    g_offx = 0;
    g_offy = 0;
    g_offz = 0;
    
    for( uint8_t i = 0; i < 10; i ++ ) //take the mean from 10 gyro probes and divide it from the current probe
    {
        delay(10);
        float* gp = readGyro();
        tmpx += *(  gp);
        tmpy += *(++gp);
        tmpz += *(++gp);
    }
    g_offx = tmpx/10;
    g_offy = tmpy/10;
    g_offz = tmpz/10;
}

float* GY_85::readGyro()
{
    static float axis[4];
    
    Wire.beginTransmission( ITG3200 );
    Wire.write( 0x1B );
    Wire.endTransmission();
    
    Wire.beginTransmission( ITG3200 );
    Wire.requestFrom( ITG3200, 8 );             // request 8 bytes from ITG3200
    
    int i = 0;
    uint8_t buff[8];
    while(Wire.available())
    {
        buff[i] = Wire.read();
        i++;
    }
    Wire.endTransmission();

    int16_t i0, i1, i2, i3;
    
    i0 = ((buff[2] << 8) | buff[3]);
    i1 = ((buff[4] << 8) | buff[5]);
    i2 = ((buff[6] << 8) | buff[7]);
    i3 = ((buff[0] << 8) | buff[1]);       // temperature

    axis[0] = i0 - g_offx;
    axis[1] = i1 - g_offy;
    axis[2] = i2 - g_offz;
    axis[3] = i3;
    
    return axis;
}

void GY_85::init()
{
    Wire.begin();
    SetAccelerometer();
    SetCompass();
    SetGyro();
}

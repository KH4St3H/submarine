#include "Arduino.h"
#include "compass.h"
#include <Wire.h>

Compass::Compass() {
};

void Compass::init(){
	Wire.begin();
	_writeReg(0x0B,0x01);
	setMode(0x01,0x0C,0x10,0X00);
}

void Compass::_writeReg(byte r, byte v){
	Wire.beginTransmission(_ADDR);
	Wire.write(r);
	Wire.write(v);
	Wire.endTransmission();
}


void Compass::setMode(byte mode, byte odr, byte rng, byte osr){
	_writeReg(0x09,mode|odr|rng|osr);
}



void Compass::setMagneticDeclination(int degrees, uint8_t minutes) {
	_magneticDeclinationDegrees = degrees + minutes / 60;
}



void Compass::calibrate() {
	clearCalibration();
	long calibrationData[3][2] = {{65000, -65000}, {65000, -65000}, {65000, -65000}};
  	long	x = calibrationData[0][0] = calibrationData[0][1] = getX();
  	long	y = calibrationData[1][0] = calibrationData[1][1] = getY();
  	long	z = calibrationData[2][0] = calibrationData[2][1] = getZ();

	unsigned long startTime = millis();

	while((millis() - startTime) < 20000) {
		read();

  		x = getX();
  		y = getY();
  		z = getZ();

		if(x < calibrationData[0][0]) {
			calibrationData[0][0] = x;
		}
		if(x > calibrationData[0][1]) {
			calibrationData[0][1] = x;
		}

		if(y < calibrationData[1][0]) {
			calibrationData[1][0] = y;
		}
		if(y > calibrationData[1][1]) {
			calibrationData[1][1] = y;
		}

		if(z < calibrationData[2][0]) {
			calibrationData[2][0] = z;
		}
		if(z > calibrationData[2][1]) {
			calibrationData[2][1] = z;
		}
	}
	Serial.print(calibrationData[0][0]);
	Serial.print(" ");

	Serial.print(calibrationData[0][1]);
	Serial.print(" ");
	Serial.print(calibrationData[1][0]);
	Serial.print(" ");
	Serial.print(calibrationData[1][1]);
	Serial.print(" ");
	Serial.print(calibrationData[2][0]);
	Serial.print(" ");
	Serial.print(calibrationData[2][1]);
	Serial.print("\n");

	setCalibration(
		calibrationData[0][0],
		calibrationData[0][1],
		calibrationData[1][0],
		calibrationData[1][1],
		calibrationData[2][0],
		calibrationData[2][1]
	);
}

/**
    SET CALIBRATION
	Set calibration values for more accurate readings
		
	@author Claus Näveke - TheNitek [https://github.com/TheNitek]
	
	@since v1.1.0

	@deprecated Instead of setCalibration, use the calibration offset and scale methods.
**/
void Compass::setCalibration(int x_min, int x_max, int y_min, int y_max, int z_min, int z_max){
	setCalibrationOffsets(
		(x_min + x_max)/2,
		(y_min + y_max)/2,
		(z_min + z_max)/2
	);

	float x_avg_delta = (x_max - x_min)/2;
	float y_avg_delta = (y_max - y_min)/2;
	float z_avg_delta = (z_max - z_min)/2;

	float avg_delta = (x_avg_delta + y_avg_delta + z_avg_delta) / 3;

	setCalibrationScales(
		avg_delta / x_avg_delta,
		avg_delta / y_avg_delta,
		avg_delta / z_avg_delta
	);
}

void Compass::setCalibrationOffsets(float x_offset, float y_offset, float z_offset) {
	_offset[0] = x_offset;
	_offset[1] = y_offset;
	_offset[2] = z_offset;
}

void Compass::setCalibrationScales(float x_scale, float y_scale, float z_scale) {
	_scale[0] = x_scale;
	_scale[1] = y_scale;
	_scale[2] = z_scale;
}

float Compass::getCalibrationOffset(uint8_t index) {
	return _offset[index];
}

float Compass::getCalibrationScale(uint8_t index) {
	return _scale[index];
}

void Compass::clearCalibration(){
	setCalibrationOffsets(0., 0., 0.);
	setCalibrationScales(1., 1., 1.);
}

/**
	READ
	Read the XYZ axis and save the values in an array.
	
	@since v0.1;
**/
void Compass::read(){
	Wire.beginTransmission(_ADDR);
	Wire.write(0x00);
	int err = Wire.endTransmission();
	if (!err) {
		Wire.requestFrom(_ADDR, (byte)6);
		_vRaw[0] = (int)(int16_t)(Wire.read() | Wire.read() << 8);
		_vRaw[1] = (int)(int16_t)(Wire.read() | Wire.read() << 8);
		_vRaw[2] = (int)(int16_t)(Wire.read() | Wire.read() << 8);

		_applyCalibration();
		
		if ( _smoothUse ) {
			_smoothing();
		}
		
		//byte overflow = Wire.read() & 0x02;
		//return overflow << 2;
	}
}

/**
    APPLY CALIBRATION
	This function uses the calibration data provided via @see setCalibration() to calculate more
	accurate readings
	
	@author Claus Näveke - TheNitek [https://github.com/TheNitek]
	
	Based on this awesome article:
	https://appelsiini.net/2018/calibrate-magnetometer/
	
	@since v1.1.0
	
**/
void Compass::_applyCalibration(){
	_vCalibrated[0] = (_vRaw[0] - _offset[0]) * _scale[0];
	_vCalibrated[1] = (_vRaw[1] - _offset[1]) * _scale[1];
	_vCalibrated[2] = (_vRaw[2] - _offset[2]) * _scale[2];
}


/**
	SMOOTH OUTPUT
	This function smooths the output for the XYZ axis. Depending on the options set in
	@see setSmoothing(), we can run multiple methods of smoothing the sensor readings.
	
	First we store (n) samples of sensor readings for each axis and store them in a rolling array.
	As each new sensor reading comes in we replace it with a new reading. Then we average the total
	of all (n) readings.
	
	Advanced Smoothing
	If you turn advanced smoothing on, we will select the min and max values from our array
	of (n) samples. We then subtract both the min and max from the total and average the total of all
	(n - 2) readings.
	
	NOTE: This function does several calculations and can cause your sketch to run slower.
	
	@since v0.3;
**/
void Compass::_smoothing(){
	byte max = 0;
	byte min = 0;
	
	if ( _vScan > _smoothSteps - 1 ) { _vScan = 0; }
	
	for ( int i = 0; i < 3; i++ ) {
		if ( _vTotals[i] != 0 ) {
			_vTotals[i] = _vTotals[i] - _vHistory[_vScan][i];
		}
		_vHistory[_vScan][i] = _vCalibrated[i];
		_vTotals[i] = _vTotals[i] + _vHistory[_vScan][i];
		
		if ( _smoothAdvanced ) {
			max = 0;
			for (int j = 0; j < _smoothSteps - 1; j++) {
				max = ( _vHistory[j][i] > _vHistory[max][i] ) ? j : max;
			}
			
			min = 0;
			for (int k = 0; k < _smoothSteps - 1; k++) {
				min = ( _vHistory[k][i] < _vHistory[min][i] ) ? k : min;
			}
					
			_vSmooth[i] = ( _vTotals[i] - (_vHistory[max][i] + _vHistory[min][i]) ) / (_smoothSteps - 2);
		} else {
			_vSmooth[i] = _vTotals[i]  / _smoothSteps;
		}
	}
	
	_vScan++;
}


/**
	GET X AXIS
	Read the X axis
	
	@since v0.1;
	@return int x axis
**/
int Compass::getX(){
	return _get(0);
}


/**
	GET Y AXIS
	Read the Y axis
	
	@since v0.1;
	@return int y axis
**/
int Compass::getY(){
	return _get(1);
}


/**
	GET Z AXIS
	Read the Z axis
	
	@since v0.1;
	@return int z axis
**/
int Compass::getZ(){
	return _get(2);
}

/**
	GET SENSOR AXIS READING
	Get the smoothed, calibration, or raw data from a given sensor axis
	
	@since v1.1.0
	@return int sensor axis value
**/
int Compass::_get(int i){
	if ( _smoothUse ) 
		return _vSmooth[i];
	
	return _vCalibrated[i];
}



/**
	GET AZIMUTH
	Calculate the azimuth (in degrees);
	Correct the value with magnetic declination if defined. 
	
	@since v0.1;
	@return int azimuth
**/
int Compass::getAzimuth(){
	float heading = atan2( getY(), getX() ) * 180.0 / PI;

	heading += _magneticDeclinationDegrees;
	if(heading<0)
		heading += 360;
	return ((int)heading) % 360;
}
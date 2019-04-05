#include "Arduino.h"
#include "LineSensor.h"

LineSensor::LineSensor(){
  pinMode(0, OUTPUT);
  _pin = 0;
  _calibrationValue = -1;
}

LineSensor::LineSensor(int pin){
  pinMode(pin, OUTPUT);
  _pin = pin;
  _calibrationValue = -1;
}

void LineSensor::calibrate(int samples){	
	int mean = 0;
	for (int i = 0; i < samples; i++)
		mean += analogRead(_pin);
	
	_calibrationValue = (int) (mean / samples);
}

bool LineSensor::readSensor(){
	if (_calibrationValue < 0)
		return false;
	
	int mean = 0;
	for (int i = 0; i < 10; i++)
		mean += analogRead(_pin);
	
	return (mean / 10) < (_calibrationValue / 2);
}
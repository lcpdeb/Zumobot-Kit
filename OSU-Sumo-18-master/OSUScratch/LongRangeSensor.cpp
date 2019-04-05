#include "Arduino.h"
#include "LongRangeSensor.h"

LongRangeSensor::LongRangeSensor(){
  pinMode(0, OUTPUT);
  _pin = 0;
}

LongRangeSensor::LongRangeSensor(int pin){
  pinMode(pin, OUTPUT);
  _pin = pin;
}

int LongRangeSensor::readSensor(){
	int mean = 0;
	for (int i = 0; i < 10; i++)
		mean += analogRead(_pin);
	
	mean /= 10;
	
  return mean;
}
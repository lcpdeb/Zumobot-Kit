#include "Arduino.h"
#include "ShortRangeSensor.h"

ShortRangeSensor::ShortRangeSensor(){
  pinMode(0, OUTPUT);
  _pin = 0;
}

ShortRangeSensor::ShortRangeSensor(int pin){
  pinMode(pin, OUTPUT);
  _pin = pin;
}

bool ShortRangeSensor::readSensor(){
	int mean = 0;
	for (int i = 0; i < 10; i++){
		if (digitalRead(_pin) == HIGH)
			mean++;
	}
		
  return mean > 2;
}
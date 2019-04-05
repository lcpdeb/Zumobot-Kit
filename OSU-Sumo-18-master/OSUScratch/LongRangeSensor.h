#ifndef LongRangeSensor_h
#define LongRangeSensor_h
#include "Arduino.h"

class LongRangeSensor{
  public:
	LongRangeSensor();
    LongRangeSensor(int pin);
    int readSensor();

  private:
    int _pin;
};
#endif
#ifndef ShortRangeSensor_h
#define ShortRangeSensor_h
#include "Arduino.h"

class ShortRangeSensor{
  public:
	ShortRangeSensor();
    ShortRangeSensor(int pin);
    bool readSensor();

  private:
    int _pin;
};
#endif
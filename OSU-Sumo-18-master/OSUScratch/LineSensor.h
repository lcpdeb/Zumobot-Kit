#ifndef LineSensor_h
#define LineSensor_h
#include "Arduino.h"

class LineSensor{
  public:
	LineSensor();
    LineSensor(int pin);
    bool readSensor();
	void calibrate(int samples);

  private:
    int _pin;
	int _calibrationValue;
};
#endif
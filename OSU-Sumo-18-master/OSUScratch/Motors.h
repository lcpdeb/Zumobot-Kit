#ifndef Motors_h
#define Motors_h
#include "Arduino.h"
#include "Adafruit_MotorShield.h"

class Motors{
  public:
    Motors();
	
    void setSpeed(bool isLeft, int speed);
	void setSpeeds(int left, int right);
  private:
	Adafruit_MotorShield _AFMS;
};
#endif
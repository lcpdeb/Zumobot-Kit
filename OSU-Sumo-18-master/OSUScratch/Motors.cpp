#include "Arduino.h"
#include "Motors.h"
#include <Adafruit_MotorShield.h>
#include <Wire.h>

Adafruit_MotorShield _AFMS = Adafruit_MotorShield(0x60);

Adafruit_DCMotor *_motor1 = _AFMS.getMotor(3);
Adafruit_DCMotor *_motor2 = _AFMS.getMotor(2);

Motors::Motors() {
    _AFMS.begin();
}

void Motors::setSpeed(bool isLeft, int speed) {
    speed = speed > 255 ? 255 : (speed < -255 ? -255 : speed);

    if (isLeft)
    {
        if (speed == 0){
			_motor1->run(RELEASE);
		} 
        else {
            _motor1->setSpeed(speed);

            if (speed < 0){
				_motor1->run(BACKWARD);
			}
            else{
                _motor1->run(FORWARD);
			}
        }
    } else {
        if (speed == 0){
			_motor2->run(RELEASE);
		} 
        else {
            _motor2->setSpeed(speed);

            if (speed > 0){
				_motor2->run(BACKWARD);
			}
            else{
                _motor2->run(FORWARD);
			}
        }
    }
}

void Motors::setSpeeds(int left, int right) {
    left = left > 255 ? 255 : (left < -255 ? -255 : left);
	right = right > 255 ? 255 : (right < -255 ? -255 : right);

	Serial.println("DOG");
	Serial.println(left);
	Serial.println(right);
    if (left == 0) {
        _motor1->run(RELEASE);
    } else if (left > 0) {
		_motor1->setSpeed(left);
		
        _motor1->run(BACKWARD);
    } else {
		_motor1->setSpeed(left);
		
        _motor1->run(FORWARD);
	}

	if (right == 0) {
        _motor2->run(RELEASE);
    } else if (right > 0) {
		_motor2->setSpeed(right);

        _motor2->run(FORWARD);
    } else {
		_motor2->setSpeed(right);
		
        _motor2->run(BACKWARD);
    }
}
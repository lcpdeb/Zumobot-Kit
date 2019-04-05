#include <LineSensor.h>
#include <LongRangeSensor.h>
#include <ShortRangeSensor.h>
#include <Wire.h>
#include <Motors.h>
#include <Bounce.h>

Motors motors;
ShortRangeSensor shortFront;
ShortRangeSensor shortBack;

LongRangeSensor longFrontLeft;
LongRangeSensor longFrontRight;
LongRangeSensor longBackLeft;
LongRangeSensor longBackRight;

LineSensor lineFrontLeft;
LineSensor lineFrontRight;
LineSensor lineBackLeft;
LineSensor lineBackRight;

const int16_t CHARGESPEED = 255;
const int LONG_RANGE_THRESHHOLD = 0;
const int VEER_MULTIPLIER = 1;

bool lastDir = false; //True - right | False - left

void shake();
//void lineSensorManeuver();
int16_t CheckSensors();

void setup() {
    pinMode(13, OUTPUT);
    pinMode(2, INPUT_PULLUP);
  shortFront = ShortRangeSensor(15); //TODO: Update pin value
  shortBack = ShortRangeSensor(2); //TODO: Update pin value

  longFrontLeft = LongRangeSensor(35);
  longFrontRight = LongRangeSensor(36);
  longBackLeft = LongRangeSensor(5);
  longBackRight = LongRangeSensor(6);

  lineFrontLeft = LineSensor(7);
  lineFrontRight = LineSensor(8);
  lineBackLeft = LineSensor(9);
  lineBackRight = LineSensor(10);

  delay(5000);

  
  shake();

  
}

void loop() {
      digitalWrite(13, HIGH);
    delay(250);
  int sensorReadings = CheckSensors();
  bool onLine = lineFrontLeft.readSensor() || lineFrontRight.readSensor() || lineBackLeft.readSensor() || lineBackRight.readSensor();
  int leftMotorSpeed = CHARGESPEED;
  int rightMotorSpeed = CHARGESPEED;

  switch (sensorReadings) {
    //Case 0: None of the sensors see an object
    case (0):
//      if (onLine)
//        lineSensorManeuver();
//      else {
      if (lastDir)
        rightMotorSpeed = -1 * CHARGESPEED;
      else
        leftMotorSpeed = -1 * CHARGESPEED;

      motors.setSpeeds(leftMotorSpeed, leftMotorSpeed);
      //}

      break;
    //Case -1: Only shortBack sees an object
    case (-1):
//      if (onLine)
//        lineSensorManeuver();
//      else
      motors.setSpeeds(-CHARGESPEED, -CHARGESPEED);

      break;

    //Case 1: Only shortFront sees an object
    case (1):
//      if (onLine)
//        lineSensorManeuver();
//      else
      motors.setSpeeds(CHARGESPEED, CHARGESPEED);

      break;
      
    //Case 10: Long range sensor sees an object
    case (10):
      int frontLeftValue = longFrontLeft.readSensor();
      int frontRightValue = longFrontRight.readSensor();
      int backLeftValue = longBackLeft.readSensor();
      int backRightValue = longBackRight.readSensor();

      if (frontLeftValue > LONG_RANGE_THRESHHOLD && frontRightValue < LONG_RANGE_THRESHHOLD)
        lastDir = false;
      else if (frontLeftValue < LONG_RANGE_THRESHHOLD && frontRightValue > LONG_RANGE_THRESHHOLD)
        lastDir = true;
      else if (backLeftValue > LONG_RANGE_THRESHHOLD && backRightValue < LONG_RANGE_THRESHHOLD)
        lastDir = true;
      else if (backLeftValue < LONG_RANGE_THRESHHOLD && backRightValue > LONG_RANGE_THRESHHOLD)
        lastDir = false;

//      if (!onLine) {
      bool isFront = (frontLeftValue + frontRightValue) > (backLeftValue + backRightValue);

      int leftMotorSpeed = CHARGESPEED;
      int rightMotorSpeed = CHARGESPEED;

      int sensorDifference = isFront ? frontLeftValue - frontRightValue : backLeftValue - backRightValue;

      if (sensorDifference > 0)
        leftMotorSpeed = leftMotorSpeed - (VEER_MULTIPLIER * abs(sensorDifference));
      else if (sensorDifference < 0)
        rightMotorSpeed = rightMotorSpeed - (VEER_MULTIPLIER * abs(sensorDifference));

      leftMotorSpeed = isFront ? leftMotorSpeed : -1 * leftMotorSpeed;
      rightMotorSpeed = isFront ? rightMotorSpeed : -1 * rightMotorSpeed;

      motors.setSpeeds(leftMotorSpeed, rightMotorSpeed);
//      }
//      else
//        lineSensorManeuver();

      break;
  }
}

void shake() {
      digitalWrite(13, HIGH);
    delay(250);
  int shakeDelay = 50;
  motors.setSpeeds(CHARGESPEED, -CHARGESPEED);
  delay(shakeDelay);
  motors.setSpeeds(-CHARGESPEED, CHARGESPEED);
  delay(shakeDelay);
  motors.setSpeeds(-CHARGESPEED, CHARGESPEED);
  delay(shakeDelay);
  motors.setSpeeds(CHARGESPEED, -CHARGESPEED);
}

//void lineSensorManeuver() {
//  bool frontLeftValue = lineFrontleft.readSensor();
//  bool frontRightValue = lineFrontRight.readSensor();
//  bool backLeftValue = lineBackleft.readSensor();
//  bool backRightValue = lineBackRight.readSensor();
//
//  int maneuverDelay = 300;
//  double speedMultiplier = .75;
//
//  if (frontLeftValue && !frontRightValue)
//  {
//    motors.setSpeeds(-CHARGESPEED, -speedMultiplier * CHARGESPEED);
//    delay(maneuverDelay);
//  }
//  else if (!frontLeftValue && frontRightValue)
//  {
//    motors.setSpeeds(-speedMultiplier * CHARGESPEED, -CHARGESPEED);
//    delay(maneuverDelay);
//  }
//  else if (frontLeftValue && frontRightValue)
//  {
//    motors.setSpeeds(-CHARGESPEED, -CHARGESPEED);
//    delay(maneuverDelay);
//  }
//  else if (backLeftValue && !backRightValue)
//  {
//    motors.setSpeeds(CHARGESPEED, speedMultiplier * CHARGESPEED);
//    delay(maneuverDelay);
//  }
//  else if (!backLeftValue && backRightValue)
//  {
//    motors.setSpeeds(speedMultiplier * CHARGESPEED, CHARGESPEED);
//    delay(maneuverDelay);
//  }
//  else if (backLeftValue && backRightValue)
//  {
//    motors.setSpeeds(CHARGESPEED, CHARGESPEED);
//    delay(maneuverDelay);
//  }
//}

int16_t CheckSensors() {
  bool shortFrontValue = shortFront.readSensor();
  bool shortBackValue = shortBack.readSensor();

  bool longFrontValue = longFrontLeft.readSensor() + longFrontRight.readSensor() > LONG_RANGE_THRESHHOLD;
  bool longBackValue = longBackLeft.readSensor() + longBackRight.readSensor() > LONG_RANGE_THRESHHOLD;

  if (longFrontValue || longBackValue)
    return 10;

  if (shortFrontValue)
    return 1;

  if (shortBackValue)
    return -1;

  return 0;
}

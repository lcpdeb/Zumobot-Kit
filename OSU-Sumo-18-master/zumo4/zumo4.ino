#include <Wire.h>
#include <Zumo32U4.h>

Zumo32U4Motors motors;
Zumo32U4ButtonA buttonA;
Zumo32U4ProximitySensors proxSensors;
Zumo32U4LineSensors lineSensors;
Zumo32U4LCD lcd;

const int16_t CHARGESPEED = 400;
const int16_t TURNSPEED = 200;
const uint16_t FRONTTHRESHHOLD = 1;
const uint16_t SIDETHRESHHOLD = 2;
const uint16_t VEER_MULTIPLIER = 50;
const uint16_t TIMEOUT = 3000;
const uint16_t DOORDIETHRESHOLD = 6;

//lastDir is if the last direction the robot was spinning was clockwise
bool lastDir = false;
//Ignores proximity sensors if in doOrDie mode, but keeps checking for lines
bool doOrDie = false;
uint16_t startTime = 0;
uint16_t checkTime;
uint16_t timeElapsed;
uint16_t jumpBackCount = 0;
//{L C R 0 0}
uint16_t lineSensorValues[5] = { 0, 0, 0, 0, 0 };
uint16_t lineThreshold = 7000;

void lineSensorManeuver(uint8_t lineReading);
int16_t CheckSensors();
void calibrateLineSensors();
uint16_t CheckLineSensors();

void setup() {
  //Initializes the proximity and line sensors
  proxSensors.initThreeSensors();
  lineSensors.initThreeSensors();
  //Waits for button A to be pressed before starting
  lcd.print(F("Loaded"));
  lcd.setCursor(0, 1); 
  lcd.print(F("Press A"));
  buttonA.waitForButton();
  calibrateLineSensors();
  startTime = millis();
}

void loop() {
  int16_t sensorReadings = CheckSensors();
  int16_t leftMotorSpeed = CHARGESPEED;
  int16_t rightMotorSpeed = CHARGESPEED;
  uint8_t lineReading = CheckLineSensors();

  if (doOrDie)
    sensorReadings = 10;

  switch (sensorReadings) {
    //Case 0: None of the proximity sensors see an object
    case (0):
      checkTime  = millis();
      timeElapsed = checkTime - startTime;

      //If the robot has been spinning for longer than TIMEOUT, then move foward slightly
      if (timeElapsed > TIMEOUT){
        lineReading = CheckLineSensors();
        if(lineReading == 0){
          rightMotorSpeed = CHARGESPEED;
          leftMotorSpeed = CHARGESPEED;
        }
        else{
          lineSensorManeuver(lineReading);
        }
      }
      else if (lastDir){
        rightMotorSpeed = -TURNSPEED;
      }
      else{
        leftMotorSpeed = -TURNSPEED;
      }

      motors.setSpeeds(leftMotorSpeed, rightMotorSpeed);

      break;
    //Left proximity sensor sees an object
    case (-1):
      startTime = millis();
      leftMotorSpeed = -TURNSPEED;
      lastDir = false;

      motors.setSpeeds(leftMotorSpeed, rightMotorSpeed);

      break;
    //Right Proximity sensor sees an object
    case (1):
      startTime = millis();
      rightMotorSpeed = -TURNSPEED;
      lastDir = true;

      motors.setSpeeds(leftMotorSpeed, rightMotorSpeed);

      break;
    //Front proximity sensors see an object
    case (10):
      startTime = millis();

      if (doOrDie) {
        if (lineReading == 0){
          motors.setSpeeds(CHARGESPEED, CHARGESPEED);
        }
        else {
          doOrDie = false;
          lineSensorManeuver(lineReading);
        }
      }
      else {
        if (lineReading == 0){
          int16_t frontLeft = proxSensors.countsFrontWithLeftLeds();
          int16_t frontRight = proxSensors.countsFrontWithRightLeds();
          int16_t frontSensorDifference = frontLeft - frontRight;
        
          if (frontLeft + frontRight > DOORDIETHRESHOLD)
            doOrDie = true;
        
          if (frontSensorDifference > 0) {
            leftMotorSpeed = leftMotorSpeed - (VEER_MULTIPLIER * abs(frontSensorDifference));
          }
          else if (frontSensorDifference < 0) {
            rightMotorSpeed = rightMotorSpeed - (VEER_MULTIPLIER * abs(frontSensorDifference));
          }
        
          motors.setSpeeds(leftMotorSpeed, rightMotorSpeed);
          delay(10);
        }
        else{
          lineSensorManeuver(lineReading);
        }
      }
      break;
  }
}

void lineSensorManeuver(uint8_t lineReading) {
  int lineDelay = 300;
  int spinDelay = 500;

  switch (lineReading) {
    //If only the left line sensor sees a line, go backward and to the right
    case (1):
      if(jumpBackCount > 1){
        motors.setSpeeds(-CHARGESPEED, -.75 * CHARGESPEED);
        delay(2*lineDelay);
        motors.setSpeeds(-CHARGESPEED, CHARGESPEED);
        delay(spinDelay);
        jumpBackCount = 0;
        doOrDie = false;
        break;
      }
      motors.setSpeeds(-CHARGESPEED, -.75 * CHARGESPEED);
      delay(lineDelay);
      jumpBackCount++;
      break;
    //If only the left line sensor sees a line, go backward and to the left
    case (2):
      if(jumpBackCount > 1){
        motors.setSpeeds(-.75 * CHARGESPEED, -CHARGESPEED);
        delay(2*lineDelay);
        motors.setSpeeds(-CHARGESPEED, CHARGESPEED);
        delay(spinDelay);
        jumpBackCount = 0;
        doOrDie = false;
        break;
      }
      motors.setSpeeds(-.75 * CHARGESPEED, -CHARGESPEED);
      delay(lineDelay);
      jumpBackCount++;
      break;
    //If left and center line sensors see a line, go backward and to the right
    case (3):
      if(jumpBackCount > 1){
        motors.setSpeeds(-CHARGESPEED, -.75 * CHARGESPEED);
        delay(2*lineDelay);
        motors.setSpeeds(-CHARGESPEED, CHARGESPEED);
        delay(spinDelay);
        jumpBackCount = 0;
        doOrDie = false;
        break;
      }
      motors.setSpeeds(-CHARGESPEED, -.75 * CHARGESPEED);
      delay(lineDelay);
      jumpBackCount++;
      break;
    //If right and center line sensors see a line, go backward and to the left
    case (4):
      if(jumpBackCount > 1){
        motors.setSpeeds(-.75 * CHARGESPEED, -CHARGESPEED);
        delay(2*lineDelay);
        motors.setSpeeds(-CHARGESPEED, CHARGESPEED);
        delay(spinDelay);
        jumpBackCount = 0;
        doOrDie = false;
        break;
      }
      motors.setSpeeds(-.75 * CHARGESPEED, -CHARGESPEED);
      delay(lineDelay);
      jumpBackCount++;
      break;
    //If all three line sensors see a line, go straight backward
    case (5):
      if(jumpBackCount > 1){
        motors.setSpeeds(-CHARGESPEED, -CHARGESPEED);
        delay(2*lineDelay);
        motors.setSpeeds(-CHARGESPEED, CHARGESPEED);
        delay(spinDelay);
        jumpBackCount = 0;
        doOrDie = false;
        break;
      }
      motors.setSpeeds(-CHARGESPEED, -CHARGESPEED);
      delay(lineDelay);
      jumpBackCount++;
      break;
  }
}

int16_t CheckSensors() {
  proxSensors.read();

  //Returns 10 if both of the front proximity sensors see an object
  if ((proxSensors.countsFrontWithRightLeds() + proxSensors.countsFrontWithLeftLeds()) > FRONTTHRESHHOLD)
    return 10;
  //Returns -1 if the left proximity sensor sees an object
  else if (proxSensors.countsLeftWithLeftLeds() > SIDETHRESHHOLD)
    return -1;
  //Returns 1 if the right proximity sensor sees an object
  else if (proxSensors.countsRightWithRightLeds() > SIDETHRESHHOLD)
    return 1;
  //Returns 0 if none of the proximity sensors see an object
  else
    return 0;
}

void calibrateLineSensors() {
  uint32_t average = 0;
  ledYellow(1);

  //Calculates the average value of black using the three line sensors
  for (uint16_t i = 0; i < 10; i++)
  {
    lineSensors.read(lineSensorValues);
    average += (lineSensorValues[0] + lineSensorValues[1] + lineSensorValues[2]);
    delay(500);
  }

  average = average / (3 * 10);
  lineThreshold = average;
  ledYellow(0);
}

uint16_t CheckLineSensors() {
  lineSensors.read(lineSensorValues);
  bool leftLineOn = false;
  bool centerLineOn = false;
  bool rightLineOn = false;

  //Checks to see if on the line by checking if the value is below
  //half the value that was calibrated as black
  if (lineSensorValues[0] < (lineThreshold / 2))
    leftLineOn = true;
  if (lineSensorValues[1] < (lineThreshold / 2))
    centerLineOn = true;
  if (lineSensorValues[2] < (lineThreshold / 2))
    rightLineOn = true;

  //if only left sensor is on return 1
  if (leftLineOn && !centerLineOn && !rightLineOn)
    return 1;
  //if only right sensor is on return 2
  else if (!leftLineOn && !centerLineOn && rightLineOn)
    return 2;
  //if left and center sensors are on return 3
  else if (leftLineOn && centerLineOn && !rightLineOn)
    return 3;
  //if right and center sensors are on return 4
  else if (!leftLineOn && centerLineOn && rightLineOn)
    return 4;
  //if all sensors are on return 5
  else if (leftLineOn && centerLineOn && rightLineOn)
    return 5;
  //if no sensors are on return 0
  else
    return 0;
}

#include <Wire.h>
#include <Zumo32U4.h>

Zumo32U4LCD lcd;
Zumo32U4Motors motors;
Zumo32U4ProximitySensors proxSensors;
Zumo32U4ButtonA buttonA;
Zumo32U4LineSensors lineSensors;

// A sensors reading must be greater than or equal to this
// threshold in order for the program to consider that sensor as
// seeing an object.
const uint8_t sensorThreshold = 1;

// The maximum speed to drive the motors while turning.  400 is
// full speed.
const uint16_t turnSpeedMax = 400;

// The minimum speed to drive the motors while turning.  400 is
// full speed.
const uint16_t turnSpeedMin = 150;

// The amount to decrease the motor speed by during each cycle
// when an object is seen.
const uint16_t deceleration = 15;

// The amount to increase the speed by during each cycle when an
// object is not seen.
const uint16_t acceleration = 15;

#define LEFT 0
#define RIGHT 1

// Stores the last indication from the sensors about what
// direction to turn to face the object.  When no object is seen,
// this variable helps us make a good guess about which direction
// to turn.
bool senseDir = RIGHT;

// True if the robot is turning left (counter-clockwise).
bool turningLeft = false;

// True if the robot is turning right (clockwise).
bool turningRight = false;

// If the robot is turning, this is the speed it will use.
uint16_t turnSpeed = turnSpeedMax;

// The time, in milliseconds, when an object was last seen.
uint16_t lastTimeObjectSeen = 0;

//line sensor values in this order {L, C, R}
uint16_t lineSensorValues[3] = {0, 0, 0};

//threshold that distinguishes between on the line and off the line
//initially set to 1000, but changed using calibrateLineSensors()
uint16_t lineThreshold = 1000;

// The variable is used when adjusting the charge speed
// The chargeLeft and chargeRight starts at maximum speed
float chargeSpeed = 400;

// Desicribe the deceleration of charging speed
float chargeSpeedModifier = 0.985;

//Charge time
uint16_t chargeTime = 30;

void setup()
{
  proxSensors.initFrontSensor();

  // Wait for the user to press A before driving the motors.
  lcd.clear();
  lcd.print(F("Press A"));
  buttonA.waitForButton();
  //delay(1000);
  //test code
  lcd.clear();
  //initializes three line sensor configuration
  lineSensors.initThreeSensors();
  calibrateLineSensors();
}

void calibrateLineSensors(){
  uint32_t average = 0;
  //turns on yellow LED to show that it is calibrating
  ledYellow(1);
  lcd.clear();
  lcd.print(F("Line cal"));

  //Calculates the average value of black using the three line sensors
  for (uint16_t i = 0; i < 10; i++)
  {
    lineSensors.read(lineSensorValues);
    lcd.gotoXY(0, 1);
    lcd.print(i);
    average += (lineSensorValues[0] + lineSensorValues[1] + lineSensorValues[2]);
    delay(500);
  }

  average = average/(3 * 10);
  lineThreshold = average;
  ledYellow(0);
  lcd.clear();
}

void turnRight()
{
  motors.setSpeeds(turnSpeed, -turnSpeed);
  turningLeft = false;
  turningRight = true;
}

void chargeRight()
{
  if(turningLeft == true&&turningRight == false){
    chargeSpeed=400;
  }
  chargeSpeed = chargeSpeed * chargeSpeedModifier;
  motors.setSpeeds(turnSpeedMax, chargeSpeed);
  turningLeft = false;
  turningRight = true;
}

void turnLeft()
{
  motors.setSpeeds(-turnSpeed, turnSpeed);
  turningLeft = true;
  turningRight = false;
}

void chargeLeft()
{
  if(turningLeft == false&&turningRight == true){
    chargeSpeed=400;
  }
  chargeSpeed = chargeSpeed * chargeSpeedModifier;
  motors.setSpeeds(chargeSpeed, turnSpeedMax);
  turningLeft = true;
  turningRight = false;
}

void charge()
{
  motors.setSpeeds(turnSpeedMax, turnSpeedMax);
  turningLeft = false;
  turningRight = false;
}

int16_t CheckLineSensors(){
  lineSensors.read(lineSensorValues);
  bool leftLineOn = false;
  bool centerLineOn = false;
  bool rightLineOn = false;

  //Checks to see if on the line by checking if the value is below
  //half the value that was calibrated as black
  if(lineSensorValues[0] < (lineThreshold)/2){
    leftLineOn = true;
  }
  if(lineSensorValues[1] < (lineThreshold)/2){
    centerLineOn = true;
  }
  if(lineSensorValues[2] < (lineThreshold)/2){
    rightLineOn = true;
  }

//if(leftLineOn || centerLineOn || rightLineOn){
//    return 1;
//  }else{
//    return 0;
//  }

  //if only left sensor is on return 1
  if(leftLineOn && !centerLineOn && !rightLineOn){
    return 1;
  }
  //if only right sensor is on return 2
  else if(!leftLineOn && !centerLineOn && rightLineOn){
    return 2;
  }
  //if left and center sensors are on return 3
  else if(leftLineOn && centerLineOn){
    return 3;
  }
  //if right and center sensors are on return 4
  else if(centerLineOn && rightLineOn){
    return 4;
  }
  //if all sensors are on return 5
  else if(leftLineOn && centerLineOn && rightLineOn){
    return 5;
  }
  //if no sensors are on return 0
  else{
    return 0;
  }
}

void JumpBack() {
  int16_t sensor = CheckLineSensors();

  switch (sensor) {
    //If only the left line sensor sees a line, go backward and to the right
    case (1):
      motors.setSpeeds(-turnSpeedMax, -.85 * turnSpeedMax);
      delay(200);
      break;
    //If only the left line sensor sees a line, go backward and to the left
    case (2):
      motors.setSpeeds(-.85 * turnSpeedMax, -turnSpeedMax);
      delay(200);
      break;
    //If left and center line sensors see a line, go backward and to the right
    case (3):
      motors.setSpeeds(-turnSpeedMax, -.85 * turnSpeedMax);
      delay(200);
      break;
    //If right and center line sensors see a line, go backward and to the left
    case (4):
      motors.setSpeeds(-.85 * turnSpeedMax, -turnSpeedMax);
      delay(200);
      break;
    //If all three line sensors see a line, go straight backward
    case (5):
      motors.setSpeeds(-turnSpeedMax, -turnSpeedMax);
      delay(200);
      break;
  }
  
//  if(sensor!=0){
//    lcd.print(sensor);
//    motors.setSpeeds(-turnSpeedMax, -turnSpeedMax);
//    delay(200);
//  }
}

void Turn() {
    motors.setSpeeds(-30, -turnSpeedMax);
    delay(500);
    turningRight = true;
    turningLeft = false;
}

void loop()
{
  // Read the front proximity sensor and gets its left value (the
  // amount of reflectance detected while using the left LEDs)
  // and right value.
  proxSensors.read();
  uint8_t leftValue = proxSensors.countsFrontWithLeftLeds();
  uint8_t rightValue = proxSensors.countsFrontWithRightLeds();
   
  JumpBack();
  // Determine if an object is visible or not.
  bool objectSeen = leftValue >= sensorThreshold || rightValue >= sensorThreshold;

  if (objectSeen)
  {
    // An object is visible, so we will start decelerating in
    // order to help the robot find the object without
    // overshooting or oscillating.
    turnSpeed -= deceleration;
  }
  else
  {
    // An object is not visible, so we will accelerate in order
    // to help find the object sooner.
    turnSpeed += acceleration;
  }

  // Constrain the turn speed so it is between turnSpeedMin and
  // turnSpeedMax.
  turnSpeed = constrain(turnSpeed, turnSpeedMin, turnSpeedMax);

  if (objectSeen)
  {
    // An object seen.
    ledYellow(1);

    lastTimeObjectSeen = millis();

    bool lastTurnRight = turnRight;

    if (leftValue < rightValue)
    {
      // The right value is greater, so the object is probably
      // closer to the robot's right LEDs, which means the robot
      // is not facing it directly.  Turn to the right to try to
      // make it more even.
      chargeRight();
      chargeTime ++;
      senseDir = RIGHT;
    }
    else if (leftValue > rightValue)
    {
      // The left value is greater, so turn to the left.
      chargeLeft();
      chargeTime ++;
      senseDir = LEFT;
    }
    else if(chargeTime > 70){
      chargeTime = 0;
      Turn();
    }
    else
    {
      // The values are equal, so stop the motors.
      charge();
      chargeTime ++;
    }
  }
  else
  {
    // No object is seen, so just keep turning in the direction
    // that we last sensed the object.
    ledYellow(0);

    if (senseDir == RIGHT)
    {
      turnRight();
    }
    else
    {
      turnLeft();
    }
  }

  lcd.gotoXY(0, 0);
  lcd.print(leftValue);
  lcd.print(' ');
  lcd.print(rightValue);
  lcd.gotoXY(0, 1);
  lcd.print(turningRight ? 'R' : (turningLeft ? 'L' : ' '));
  lcd.print(' ');
  lcd.print(turnSpeed);
  lcd.print(' ');
  lcd.print(' ');
}

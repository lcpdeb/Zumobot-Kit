/*
  ZumoBot Kit
  OSU-Yuan You

*/
#include <Wire.h>
#include <Zumo32U4.h>

Zumo32U4LCD lcd;
Zumo32U4ButtonA buttonA;
Zumo32U4ButtonB buttonB;
Zumo32U4ButtonC buttonC;
Zumo32U4Buzzer buzzer;
Zumo32U4Motors motors;
Zumo32U4LineSensors lineSensors;
Zumo32U4ProximitySensors proxSensors;

unsigned int lineSensorValues[3];

// When the reading on a line sensor goes below this value, we
// consider that line sensor to have detected the white border at
// the edge of the ring.  This value might need to be tuned for
// different lighting conditions, surfaces, etc.
const uint16_t lineSensorThreshold = 500;

// The speed that the robot uses when backing up.
const uint16_t reverseSpeed = 400;

// The speed that the robot uses when turning.
const uint16_t turnSpeed = 400;

// The speed that the robot usually uses when moving forward.
// You don't want this to be too fast because then the robot
// might fail to stop when it detects the white border.
const uint16_t forwardSpeed = 300;

// These two variables specify the speeds to apply to the motors
// when veering left or veering right.  While the robot is
// driving forward, it uses its proximity sensors to scan for
// objects ahead of it and tries to veer towards them.
const uint16_t veerSpeedLow = 200;
const uint16_t veerSpeedHigh = 400;

// The speed that the robot drives when it detects an opponent in
// front of it, either with the proximity sensors or by noticing
// that it is caught in a stalemate (driving forward for several
// seconds without reaching a border).  400 is full speed.
const uint16_t rammingSpeed = 400;

// The amount of time to spend backing up after detecting a
// border, in milliseconds.
const uint16_t reverseTime = 200;

// The minimum amount of time to spend scanning for nearby
// opponents, in milliseconds.
const uint16_t scanTimeMin = 200;

// The maximum amount of time to spend scanning for nearby
// opponents, in milliseconds.
const uint16_t scanTimeMax = 2100;

// The amount of time to wait between detecting a button press
// and actually starting to move, in milliseconds.  Typical robot
// sumo rules require 5 seconds of waiting.
const uint16_t waitTime = 5000;

// If the robot has been driving forward for this amount of time,
// in milliseconds, without reaching a border, the robot decides
// that it must be pushing on another robot and this is a
// stalemate, so it increases its motor speed.
const uint16_t stalemateTime = 1000;

// This enum lists the top-level states that the robot can be in.
enum State
{
  StatePausing,
  StateWaiting,
  StateScanning,
  StateDriving,
  StateBacking,
};

State state = StatePausing;

enum Direction
{
  DirectionLeft,
  DirectionRight,
  DirectionFront,
};

// scanDir is the direction the robot should turn the next time
// it scans for an opponent.
Direction scanDir = DirectionFront;


// The time, in milliseconds, that we entered the current top-level state.
uint16_t stateStartTime;

// The time, in milliseconds, that the LCD was last updated.
uint16_t displayTime;

// This gets set to true whenever we change to a new state.
// A state can read and write this variable this in order to
// perform actions just once at the beginning of the state.
bool justChangedState;

// This gets set whenever we clear the display.
bool displayCleared;

void setup()
{
  // Uncomment if necessary to correct motor directions:
  //motors.flipLeftMotor(true);
  //motors.flipRightMotor(true);

  lineSensors.initThreeSensors();
  proxSensors.initThreeSensors();

  changeState(StatePausing);
}

void loop()
{
  bool buttonPressA = buttonA.getSingleDebouncedPress();
  bool buttonPressB = buttonB.getSingleDebouncedPress();
  bool buttonPressC = buttonC.getSingleDebouncedPress();
  if (state == StatePausing)
  {
    // In this state, we just wait for the user to press button
    // A, while displaying the battery voltage every 100 ms.

    motors.setSpeeds(0, 0);

    if (justChangedState)
    {
      justChangedState = false;
      lcd.print(F("Press A"));
    }

    if (displayIsStale(100))
    {
      displayUpdated();
      lcd.gotoXY(0, 1);
      lcd.print(readBatteryMillivolts() / 1000);
      lcd.print(".");
      lcd.print(readBatteryMillivolts() / 100 % 10);
      lcd.print("V");
      lcd.print(' ');
      lcd.print(' ');
      lcd.print("YY");
    }

    if (buttonPressA)
    {
      // The user pressed button A, so go to the waiting state.
      scanDir = DirectionLeft;
      changeState(StateWaiting);

    }
    else if (buttonPressB)
    {
      scanDir = DirectionFront;
      changeState(StateWaiting);
    }
    else if (buttonPressC)
    {
      scanDir = DirectionRight;
      changeState(StateWaiting);
    }
  }
  else if (buttonPressA)
  {
    // The user pressed button A while the robot was running, so pause.
    changeState(StatePausing);
  }
  else if (state == StateWaiting)
  {
    // In this state, we wait for a while and then move on to the
    // scanning state.

    motors.setSpeeds(0, 0);

    uint16_t time = timeInThisState();

    if (time < waitTime)
    {
      // Display the remaining time we have to wait.
      uint16_t timeLeft = waitTime - time;
      lcd.gotoXY(0, 0);
      lcd.print(timeLeft / 1000 % 10);
      lcd.print('.');
      lcd.print(timeLeft / 100 % 10);
      lcd.print('s');
    }
    else
    {
      // We have waited long enough.  Start moving.
      changeState(StateScanning);
    }
  }
  else if (state == StateBacking)
  {
    // In this state, the robot drives in reverse.

    if (justChangedState)
    {
      justChangedState = false;
      lcd.print(F("BACK"));
      lcd.gotoXY(0, 1);
      lineSensors.read(lineSensorValues);
      lcd.print(lineSensorValues[0]);

      //lcd.print(lineSensorValues[1]);
      lcd.print(lineSensorValues[2]);
    }

    motors.setSpeeds(-reverseSpeed, -reverseSpeed);

    // After backing up for a specific amount of time, start
    // scanning.
    if (timeInThisState() >= reverseTime)
    {
      changeState(StateScanning);
    }
  }
  else if (state == StateScanning)
  {
    // In this state the robot rotates in place and tries to find
    // its opponent.

    if (justChangedState)
    {
      justChangedState = false;
      lcd.print(F("SCAN"));
      lcd.gotoXY(0, 1);
      lineSensors.read(lineSensorValues);
      lcd.print(lineSensorValues[0]);

      //lcd.print(lineSensorValues[1]);
      lcd.print(lineSensorValues[2]);
    }
    if (scanDir == DirectionFront)
    {
      // Check for borders.

      if (lineSensorValues[1] < lineSensorThreshold)
      {
        changeState(StateBacking);
      }
      else if (lineSensorValues[0] < lineSensorThreshold)
      {
        scanDir = DirectionRight;
        changeState(StateBacking);
      }
      else if (lineSensorValues[2] < lineSensorThreshold)
      {
        scanDir = DirectionLeft;
        changeState(StateBacking);
      }
      else {
        lcd.print(F("testfront"));
        motors.setSpeeds(forwardSpeed, forwardSpeed);
      }


    }
    if (scanDir == DirectionRight)
    {
      // Check for borders.
      lineSensors.read(lineSensorValues);

      if (lineSensorValues[1] < lineSensorThreshold)
      {
        changeState(StateBacking);   
      }
      else if (lineSensorValues[0] < lineSensorThreshold)
      {
        scanDir = DirectionRight;
        changeState(StateBacking);
      }
      else if (lineSensorValues[2] < lineSensorThreshold)
      {
        scanDir = DirectionLeft;
        changeState(StateBacking);
      }
      else {

        motors.setSpeeds(turnSpeed, -turnSpeed);
      }
    }
    else if (scanDir == DirectionLeft)
    {
      // Check for borders.
      lineSensors.read(lineSensorValues);

      if (lineSensorValues[1] < lineSensorThreshold)
      {
        changeState(StateBacking);
      }
      else if (lineSensorValues[0] < lineSensorThreshold)
      {
        scanDir = DirectionRight;
        changeState(StateBacking);
      }
      else if (lineSensorValues[2] < lineSensorThreshold)
      {
        scanDir = DirectionLeft;
        changeState(StateBacking);
      }
      else
      {

        motors.setSpeeds(-turnSpeed, turnSpeed);
      }
    }

    uint16_t time = timeInThisState();

    if (time > scanTimeMax)
    {
      // We have not seen anything for a while, so start driving.
      changeState(StateDriving);
    }
    else if (time > scanTimeMin)
    {
      // Read the proximity sensors.  If we detect anything with
      // the front sensor, then start driving forwards.
      proxSensors.read();
      if (proxSensors.countsFrontWithLeftLeds() >= 2 || proxSensors.countsFrontWithRightLeds() >= 2 || proxSensors.countsLeftWithLeftLeds() >= 2 || proxSensors.countsRightWithRightLeds() >= 2 || proxSensors.countsRightWithLeftLeds() >= 2 || proxSensors.countsLeftWithRightLeds() >= 2)
      {
        changeState(StateDriving);
      }
    }
  }
  else if (state == StateDriving)
  {
    // In this state we drive forward while also looking for the
    // opponent using the proximity sensors and checking for the
    // white border.

    if (justChangedState)
    {
      justChangedState = false;
      lcd.print(F("DRIVE"));
    }

    // Check for borders.
    lineSensors.read(lineSensorValues);

    if (lineSensorValues[1] < lineSensorThreshold)
    {
      changeState(StateBacking);
    }
    if (lineSensorValues[0] < lineSensorThreshold)
    {
      scanDir = DirectionRight;
      changeState(StateBacking);
    }
    if (lineSensorValues[2] < lineSensorThreshold)
    {
      scanDir = DirectionLeft;
      changeState(StateBacking);
    }

    // Read the proximity sensors to see if know where the
    // opponent is.
    proxSensors.read();
    uint8_t sum_of_front_sensor = proxSensors.countsFrontWithRightLeds() + proxSensors.countsFrontWithLeftLeds();
    int8_t diff_of_front_sensor = proxSensors.countsFrontWithRightLeds() - proxSensors.countsFrontWithLeftLeds();
    int8_t diff_of_right_front_sensor = proxSensors.countsRightWithRightLeds() - proxSensors.countsFrontWithRightLeds();
    int8_t diff_of_left_front_sensor = proxSensors.countsLeftWithLeftLeds() - proxSensors.countsFrontWithLeftLeds();

    if (proxSensors.countsFrontWithLeftLeds() >= 2 && proxSensors.countsFrontWithRightLeds() >= 2) //sum_of_front_sensor >= 4)更改测试
    {
      // The front sensor is getting a strong signal, or we have
      // been driving forward for a while now without seeing the
      // border.  Either way, there is probably a robot in front
      // of us and we should switch to ramming speed to try to
      // push the robot out of the ring.

      justChangedState = false;
      proxSensors.read();
      lcd.gotoXY(0, 0);
      lcd.print(proxSensors.countsFrontWithLeftLeds());
      lcd.print(' ');
      lcd.print("RAM");
      lcd.print(' ');
      lcd.print(' ');
      lcd.print(proxSensors.countsFrontWithRightLeds());

      lcd.gotoXY(0, 1);
      lcd.print(proxSensors.countsLeftWithLeftLeds());
      lcd.print(' ');
      lcd.print("RAM");
      lcd.print(' ');
      lcd.print(' ');
      lcd.print(proxSensors.countsRightWithRightLeds());

      motors.setSpeeds(rammingSpeed, rammingSpeed);

      // Turn on the red LED when ramming.
      ledRed(1);
    }
    else if (sum_of_front_sensor == 0)
    {
      // We don't see anything with the front sensor, so just
      // keep driving forward.  Also monitor the side sensors; if
      // they see an object then we want to go to the scanning
      // state and turn torwards that object.

      motors.setSpeeds(forwardSpeed, forwardSpeed);

      if (proxSensors.countsLeftWithLeftLeds() == 0 && proxSensors.countsRightWithRightLeds() == 0 && proxSensors.countsFrontWithLeftLeds() >= 2 && proxSensors.countsFrontWithRightLeds() >= 2 )
      {
        // Detected something to the front.
        scanDir = DirectionFront;
        changeState(StateScanning);
      }
      if (proxSensors.countsLeftWithLeftLeds() >= 5)
      {
        // Detected something to the left.
        scanDir = DirectionLeft;
        changeState(StateScanning);
      }

      if (proxSensors.countsRightWithRightLeds() >= 5)
      {
        // Detected something to the right.
        scanDir = DirectionRight;
        changeState(StateScanning);
      }

      ledRed(0);
    }

  }
}

// Gets the amount of time we have been in this state, in
// milliseconds.  After 65535 milliseconds (65 seconds), this
// overflows to 0.
uint16_t timeInThisState()
{
  return (uint16_t)(millis() - stateStartTime);
}

// Changes to a new state.  It also clears the LCD and turns off
// the LEDs so that the things the previous state were doing do
// not affect the feedback the user sees in the new state.
void changeState(uint8_t newState)
{
  state = (State)newState;
  justChangedState = true;
  stateStartTime = millis();
  ledRed(0);
  ledYellow(0);
  ledGreen(0);
  lcd.clear();
  displayCleared = true;
}

// Returns true if the display has been cleared or the contents
// on it have not been updated in a while.  The time limit used
// to decide if the contents are staled is specified in
// milliseconds by the staleTime parameter.
bool displayIsStale(uint16_t staleTime)
{
  return displayCleared || (millis() - displayTime) > staleTime;
}

// Any part of the code that uses displayIsStale to decide when
// to update the LCD should call this function when it updates the
// LCD.
void displayUpdated()
{
  displayTime = millis();
  displayCleared = false;
}

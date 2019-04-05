#include <Wire.h>
#include <Zumo32U4.h>

Zumo32U4Encoders encoders;
Zumo32U4Motors motors;
Zumo32U4ButtonA buttonA;
Zumo32U4ProximitySensors proxSensors;
Zumo32U4LineSensors lineSensors;
Zumo32U4LCD lcd;

int TURNSPEED = 300;
int CHARGESPEED = 300;

String loopstate;
int threshold = 0;
bool closing = 0;
int leftSpeed;
double rightSpeed;
int lastDisplayTime;
long leftLastCount = 0;
long rightLastCount = 0;
bool charging = 0;
int lastClosinTime = 0;
int lastChargeTime = 0;

void charge();

void spin(bool dir);

uint8_t readFrontLeft();

uint8_t readFrontRight();

void printToLCD(uint8_t value);

bool checkSides();

void debugs();

void setup() {
  Serial.begin(9600);
  lineSensors.initThreeSensors();
  proxSensors.initThreeSensors();
  lineSensors.calibrate();
  buttonA.waitForButton();

  delay(1000);
}

void loop() {
  proxSensors.read();
  debugs();

  if (closing == 0 && charging == 0) {
    if (proxSensors.countsFrontWithLeftLeds() == threshold && proxSensors.countsFrontWithRightLeds() == threshold) {
      spin(checkSides());
      loopstate = "spin";
    }
    else if (proxSensors.countsFrontWithLeftLeds() != 0 || proxSensors.countsFrontWithRightLeds() != 0 ) {
      approach();
    }
    else if (proxSensors.countsFrontWithLeftLeds() == 5 || proxSensors.countsFrontWithRightLeds() == 5) {
      int left, right;
      for (int i = 0; i < 3; i++) {
        proxSensors.read();
        left += proxSensors.countsFrontWithLeftLeds();
        right += proxSensors.countsFrontWithRightLeds();
      }
      if (left > 5 || right > 5) {
        closing = 1;
      }
      else {
        closing = 0;
      }
    }
  }
  else if (closing == 1 && charging == 0) {
    closein();
  }
  else {
    charge();
  }
}

void approach() {
  ledYellow(1);
  ledGreen(0);
  ledRed(0);
  if (proxSensors.countsLeftWithLeftLeds() > 3) {
    motors.setSpeeds(CHARGESPEED - 300, CHARGESPEED);
  }
  else if (proxSensors.countsRightWithLeftLeds() > 3) {
    motors.setSpeeds(CHARGESPEED, CHARGESPEED - 300);
  }
  else if (proxSensors.countsFrontWithLeftLeds() > proxSensors.countsFrontWithRightLeds()) {
    motors.setSpeeds(CHARGESPEED - 100, CHARGESPEED);
  }
  else if (proxSensors.countsFrontWithLeftLeds() < proxSensors.countsFrontWithRightLeds()) {
    motors.setSpeeds(CHARGESPEED, CHARGESPEED - 100);
  }

  else {
    motors.setSpeeds(CHARGESPEED, CHARGESPEED);
  }
}

void currentSpeed() {
  if ((int)(millis() - lastDisplayTime) >= 100)
  {
    int timelapsed = (int)(millis() - lastDisplayTime);
    long i = encoders.getCountsLeft();
    long j = encoders.getCountsRight();
    leftSpeed = 100 * (i - leftLastCount) / timelapsed;
    rightSpeed = 100 * (j - leftLastCount) / timelapsed;

    //    Serial.print(leftSpeed);
    //    Serial.print("\t");
    //    Serial.print(i);
    //    Serial.print("\t");
    //    Serial.print(leftLastCount);
    //    Serial.print("\t");
    //    Serial.print(i  - leftLastCount);
    //    Serial.print("\t");
    //    Serial.println(timelapsed);
    lastDisplayTime = millis();
    leftLastCount = i;
    rightLastCount = j;
  }
}

void closein() {
  if ((int)(millis() - lastClosinTime) >= 3000)
  {
    lastClosinTime = millis();
  }
  else {
    loopstate = "clos";
    if (abs(leftSpeed) < 200 || abs(rightSpeed) < 200) {
      charge();
    }
    if (proxSensors.countsFrontWithRightLeds() > proxSensors.countsFrontWithLeftLeds()) {
      motors.setSpeeds(-CHARGESPEED + 100, CHARGESPEED);
    }
    else if (proxSensors.countsFrontWithRightLeds() < proxSensors.countsFrontWithLeftLeds()) {
      motors.setSpeeds(CHARGESPEED, -CHARGESPEED + 100);
    }
    else {
      motors.setSpeeds(CHARGESPEED, CHARGESPEED);
    }
    ledYellow(1);
    ledGreen(0);
    ledRed(1);
  }
}

void charge() {
  if ((int)(millis() - lastChargeTime) >= 5000) {
    lastChargeTime = millis();

  }
  else {
    loopstate = "chag";
    motors.setSpeeds(400, 400);
    charging = 1;
    ledYellow(0);
    ledGreen(0);
    ledRed(1);
    //while (1);
  }
}

void spin(bool dir) {
  if (dir) {
    motors.setSpeeds(-TURNSPEED, TURNSPEED);
  } else {
    motors.setSpeeds(TURNSPEED, -TURNSPEED);
  }
  ledYellow(0);
  ledGreen(1);
  ledRed(0);
}

bool checkSides() {
  proxSensors.read();
  if (proxSensors.countsLeftWithLeftLeds() > 6) {
    return true;
  } else if (proxSensors.countsRightWithRightLeds() > 6) {
    return false;
  }
  return false;
}

void debugs() {
  currentSpeed();
  lcd.clear();
  lcd.gotoXY(0, 0);
  lcd.print(proxSensors.countsFrontWithLeftLeds());
  lcd.gotoXY(7, 0);
  lcd.print(proxSensors.countsFrontWithRightLeds());
  lcd.gotoXY(0, 1);
  lcd.print(loopstate);
  lcd.gotoXY(4, 1);
  lcd.print(leftSpeed);
}


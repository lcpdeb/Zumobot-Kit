#include <Wire.h>
#include <Zumo32U4.h>

Zumo32U4LCD lcd;
Zumo32U4LineSensors lineSensors;

//line sensor values in this order {L, C, R, 0, 0}
uint16_t lineSensorValues[5] = { 0, 0, 0, 0, 0 };

//threshold that distinguishes between on the line and off the line
//initially set to 7000, but changed using calibrateLineSensors()
uint16_t lineThreshold = 7000;

void setup()
{
  //initializes three line sensor configuration
  lineSensors.initThreeSensors();

  calibrateLineSensors();
}

//Calibrates the line sensors by determing the value for black
//and setting it as lineThreshold
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

int16_t CheckLineSensors(){
  lineSensors.read(lineSensorValues);
  bool leftLineOn = false;
  bool centerLineOn = false;
  bool rightLineOn = false;

  //Checks to see if on the line by checking if the value is below
  //half the value that was calibrated as black
  if(lineSensorValues[0] < (lineThreshold/2)){
    leftLineOn = true;
  }
  if(lineSensorValues[1] < (lineThreshold/2)){
    centerLineOn = true;
  }
  if(lineSensorValues[2] < (lineThreshold/2)){
    rightLineOn = true;
  }


  //if only left sensor is on return 1
  if(leftLineOn && !centerLineOn && !rightLineOn){
    return 1;
  }
  //if only right sensor is on return 2
  else if(!leftLineOn && !centerLineOn && rightLineOn){
    return 2;
  }
  //if left and center sensors are on return 3
  else if(leftLineOn && centerLineOn && !rightLineOn){
    return 3;
  }
  //if right and center sensors are on return 4
  else if(!leftLineOn && centerLineOn && rightLineOn){
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

void loop()
{
  static uint16_t lastSampleTime = 0;
  uint16_t leftLineSensorValue = 0;
  uint16_t centerLineSensorValue = 0;
  uint16_t rightLineSensorValue = 0;
  int16_t lineSensorCase = 0;
  if ((uint16_t)(millis() - lastSampleTime) >= 100)
  {
    lastSampleTime = millis();
    
    // Read the line sensors
    lineSensors.read(lineSensorValues);
    leftLineSensorValue = lineSensorValues[0];
    centerLineSensorValue = lineSensorValues[1];
    rightLineSensorValue = lineSensorValues[2];
    
    lineSensorCase = CheckLineSensors();

    switch(lineSensorCase){
      case(0):
        lcd.print("None");
        break;

      case(1):
        lcd.print("L");
        break;

      case(2):
        lcd.print("R");
        break;

      case(3):
        lcd.print("LC");
        break;

      case(4):
        lcd.print("CR");
        break;

      case(5):
        lcd.print("LCR");
        break;
    }

    //Displays int value from CheckLineSensors
    lcd.gotoXY(0, 1);
    lcd.print(lineSensorCase);
    

    
//    lcd.gotoXY(2, 0);
//    lcd.print(leftLineSensorValue);
//    lcd.gotoXY(0, 1);
//    lcd.print(centerLineSensorValue);
//    lcd.gotoXY(4, 1);
//    lcd.print(rightLineSensorValue);    
    
    //refreshes every 1000 ms
    delay(1000);
    lcd.clear();
  }
}



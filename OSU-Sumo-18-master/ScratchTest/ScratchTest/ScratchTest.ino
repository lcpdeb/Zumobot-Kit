    #include <LineSensor.h>
#include <LongRangeSensor.h>
#include <ShortRangeSensor.h>
#include <Motors.h>

Motors motors = Motors();
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

void setup() {
  pinMode(13, OUTPUT);
  shortFront = ShortRangeSensor(1); //TODO: Update pin value
  shortBack = ShortRangeSensor(2); //TODO: Update pin value

  longFrontLeft = LongRangeSensor(3);
  longFrontRight = LongRangeSensor(4);
  longBackLeft = LongRangeSensor(5);
  longBackRight = LongRangeSensor(6);

  lineFrontLeft = LineSensor(7);
  lineFrontRight = LineSensor(8);
  lineBackLeft = LineSensor(9);
  lineBackRight = LineSensor(10);



}

void loop() {
    motors.setSpeeds(255 , 255);

    digitalWrite(13, HIGH);
    delay(1000);
    digitalWrite(13, LOW);
    delay(1000);
}

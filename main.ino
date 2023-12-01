#include <Wire.h>
#include <math.h>
#include <Adafruit_LIS3MDL.h>
#include <Adafruit_LSM6DSOX.h>
#include <Adafruit_SH110X.h>
#include <Adafruit_GFX.h>
#include <Adafruit_Sensor.h>
#include <Serial.h>

Adafruit_LSM6DSOX sox;     // decleare accel sensor as sox
Adafruit_LIS3MDL lis3mdl;  //declare mag sensor as lis3mdl

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  float compass;
  float angle;

  if (!sox.begin_I2C()) {  // wait until accel sensor is found
    while (1) {
      delay(10);
    }
  }

  if (!lis3mdl.begin_I2C()) {
    while (1) {
      delay(10);
    }
  }

  sox.setAccelRange(LSM6DS_ACCEL_RANGE_4_G);  // set accel range to 4 G's
  sox.setAccelDataRate(LSM6DS_RATE_12_5_HZ);  // set accel data rate to 12.5 hz

  lis3mdl.setPerformanceMode(LIS3MDL_ULTRHIGHMODE);  // set mag performance mode
  lis3mdl.setRange(LIS3MDL_RANGE_4_GAUSS);           // set mag range to 4 gauss
  lis3mdl.setIntThreshold(500);
  lis3mdl.configInterrupt(true, true, true, true, false, true); 
}

void loop() {
  // put your main code here, to run repeatedly:
  float compass = getheadingdata();
  float angle = getangledata();

  Serial.print("Compass Heading: ");
  Serial.print(compass);
  Serial.print(" Angle: ");
  Serial.println(angle);

  delay(500);
}

void magcal() {
  // magnetometer calibration sequence
}

float getheadingdata() {
  // collect data from magnetometer, transform to compass
  return 2;
}

float getangledata() {
  // collect accel data, convert to angle
  sensors_event_t accel;
  sensors_event_t gyro;
  sensors_event_t temp;

  sox.getEvent(&accel, &gyro, &temp);

  int xaccel = accel.acceleration.x;
  int yaccel = accel.acceleration.y;
  int zaccel = accel.acceleration.z;

  float tancalc = xaccel / zaccel;  // figure out tangent number

  float radiancalc = atan(tancalc);              // find angle in rads from tangent
  float anglecalc = radiancalc * 180 / 3.14159;  // convert to degrees

  return anglecalc;
}

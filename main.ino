#include <Wire.h>
#include <math.h>
#include <Adafruit_LIS3MDL.h>
#include <Adafruit_LSM6DS3TRC.h>
#include <Adafruit_SH110X.h>
#include <Adafruit_GFX.h>
#include <Adafruit_Sensor.h>
#include <Serial.h>

Adafruit_LSM6DS3TRC lsm6ds3trc;                 // decleare accel sensor as lsm6d3trc
Adafruit_LIS3MDL lis3mdl = Adafruit_LIS3MDL();  //declare mag sensor as lis3mdl

int minX, minY, minZ;
int maxX, maxY, maxZ;

float softIronMatrix[3][3];

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  float compass;
  float angle;

  if (!lsm6ds3trc.begin_I2C()) {  // wait until accel sensor is found
    int i = 0;
    while (1) {
      delay(10);
      i++;
      if (i > 10) {
        Serial.println("Error connecting to LSM6DS3trc");
        break;
      }
    }
  }

  if (!lis3mdl.begin_I2C()) {  // wait until mag sensor is found
    int i = 0;
    while (1) {
      delay(10);
      i++;
      if (i > 10) {
        Serial.println("Error connecting to LIS3MDL");
        break;
      }
    }
  }

  lsm6ds3trc.setAccelRange(LSM6DS_ACCEL_RANGE_4_G);  // set accel range to 4 G's
  lsm6ds3trc.setAccelDataRate(LSM6DS_RATE_12_5_HZ);  // set accel data rate to 12.5 hz
  lsm6ds3trc.configInt1(false, false, true);         // accelerometer DRDY on INT1

  lis3mdl.setPerformanceMode(LIS3MDL_ULTRAHIGHMODE);  // set mag performance mode
  lis3mdl.setRange(LIS3MDL_RANGE_16_GAUSS);            // set mag range to 16 gauss
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

  if (Serial.available() == 1) {  // check serial for calibration sequence
    int instruction = Serial.read();
    if (instruction == 1) {
      calibrateMagnetometer();
      calculateSoftIronCalibration();
    }
  }

  delay(500);
}

float getheadingdata() {
  // collect data from magnetometer, transform to compass
  sensors_event_t event;
  lis3mdl.getEvent(&event);

  // Apply hard iron calibration
  float calibratedX = event.magnetic.x - (maxX + minX) / 2;
  float calibratedY = event.magnetic.y - (maxY + minY) / 2;
  float calibratedZ = event.magnetic.z - (maxZ + minZ) / 2;

  // Apply soft iron calibration
  float correctedX = softIronMatrix[0][0] * calibratedX + softIronMatrix[0][1] * calibratedY + softIronMatrix[0][2] * calibratedZ;
  float correctedY = softIronMatrix[1][0] * calibratedX + softIronMatrix[1][1] * calibratedY + softIronMatrix[1][2] * calibratedZ;
  float correctedZ = softIronMatrix[2][0] * calibratedX + softIronMatrix[2][1] * calibratedY + softIronMatrix[2][2] * calibratedZ;

  // Calculate compass heading
  float heading = atan2(correctedY, correctedX) * 180.0 / PI;
  if (heading < 0) {
    heading += 360.0;
  }
}

void calibrateMagnetometer() { // calibrate hard iron offsets
  Serial.println("Calibrating magnetometer. Rotate the sensor in all three axes.");

  // Initialize calibration values
  minX = minY = minZ = INFINITY;
  maxX = maxY = maxZ = -INFINITY;

  // Capture minimum and maximum values
  for (int i = 0; i < 500; i++) { // Adjust the number of samples as needed
    sensors_event_t event;
    lis3mdl.getEvent(&event);

    minX = min(minX, event.magnetic.x);
    minY = min(minY, event.magnetic.y);
    minZ = min(minZ, event.magnetic.z);

    maxX = max(maxX, event.magnetic.x);
    maxY = max(maxY, event.magnetic.y);
    maxZ = max(maxZ, event.magnetic.z);

    delay(10); // Adjust delay between samples if needed
  }

  // Print calibration values
  Serial.println("Calibration complete. Use these values in your main code:");
  Serial.print("minX: "); Serial.print(minX); Serial.print("\tmaxX: "); Serial.println(maxX);
  Serial.print("minY: "); Serial.print(minY); Serial.print("\tmaxY: "); Serial.println(maxY);
  Serial.print("minZ: "); Serial.print(minZ); Serial.print("\tmaxZ: "); Serial.println(maxZ);
}

void calculateSoftIronCalibration() { // calibrate soft iron offsets
  // Calculate the soft iron calibration matrix
  softIronMatrix[0][0] = (maxX - minX) / 2;
  softIronMatrix[1][1] = (maxY - minY) / 2;
  softIronMatrix[2][2] = (maxZ - minZ) / 2;

  softIronMatrix[0][1] = (maxX + minX) / 2;
  softIronMatrix[1][0] = (maxX + minX) / 2;

  softIronMatrix[0][2] = (maxX + minX) / 2;
  softIronMatrix[2][0] = (maxX + minX) / 2;

  softIronMatrix[1][2] = (maxY + minY) / 2;
  softIronMatrix[2][1] = (maxY + minY) / 2;

  // Normalize the matrix
  float maxScale = max(max(softIronMatrix[0][0], softIronMatrix[1][1]), softIronMatrix[2][2]);
  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 3; j++) {
      softIronMatrix[i][j] /= maxScale;
    }
  }

  // Print the soft iron calibration matrix
  Serial.println("Soft Iron Calibration Matrix:");
  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 3; j++) {
      Serial.print(softIronMatrix[i][j]);
      Serial.print("\t");
    }
    Serial.println();
  }
}

float getangledata() {
  // collect accel data, convert to angle
  sensors_event_t accel;
  sensors_event_t gyro;
  sensors_event_t temp;

  lsm6ds3trc.getEvent(&accel, &gyro, &temp);

  float xaccel = accel.acceleration.x;
  float yaccel = accel.acceleration.y;
  float zaccel = accel.acceleration.z;

  /* Serial.print("zaccel: "); // testing outputs
  Serial.print(zaccel); 
  Serial.print(" yaccel: ");
  Serial.println(yaccel); */

  float tancalc = yaccel / zaccel;  // figure out tangent number

  float radiancalc = atan(tancalc);         // find angle in rads from tangent
  float anglecalc = radiancalc * 180 / PI;  // convert to degrees

  return anglecalc;
}

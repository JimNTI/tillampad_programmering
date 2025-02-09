/* 
  Name: Distance_Project
  Author: Jim Palo
  Description: 
  Code for Arduino Uno that reads ADXL-accelerometer acceleration values, and converts it to distance.
  Including a drift stabilizer, acceleration and velocity reseter while stationary and callibration for more accurate readings.
  Code also includes an output to an U8GLIB_SSD1306_128X64 screen with X, Y and total distance readings measured in cm.
*/ 

// Libraries
#include <SparkFun_ADXL345.h>
#include "U8glib.h"

// Insert classes
ADXL345 adxl = ADXL345();
U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_NO_ACK);

int range = 2;  // ±2g range setting for ADXL345

// Variables to display information to screen
const int display_x = 0;
const int display_y = 10;

// Variables for readings and convertions
double offset_X;
double accel_X = 0.0;
double velocity_X = 0.0;
double distance_X = 0.0;

double offset_Y;
double accel_Y = 0.0;
double velocity_Y = 0.0;    
double distance_Y = 0.0;

double accel_Z = 0.0;
double velocity_Z = 0.0;   
double distance_Z = 0.0; 

double total_distance = 0.0;

// Constant values to convert readings
const double gravity = 9.81;
const double corrector = 0.00380; // Correction value for more accurate values for accelerometer
const double drift_stabilizer = 0.9; // Value to fix drift problem
const double stationary_accel = 0.1; // Value to check if accelerometer is stationary & reset acceleration
const double stationary_velocity = 0.01; // Value to check if acceleromter is stationary & reset velocity

// Variable to execute code depending if stationary
bool isStationary = false;

// Variables for time difference to calculate velocity & distance
unsigned long prevTime = 0;
unsigned long lastMoveTime = 0; 



// Calibrates to measure when accelerometer is stationary
void calibrator() {
  int x, y, z;
  int sumX = 0;
  int sumY = 0;
  int samples = 100;
  oledWrite(display_x, display_y, "Callibrating", "PLZ HOLD STILL", "...", ""); // Loading screen for entertainment

  delay(2000);

  // Adds callibrations to late get out average callibration & displays calculations for "entertainment purposes"
  for (int i = 0; i < samples; i++){
    adxl.readAccel(&x, &y, &z);
    sumX += x;
    sumY += y;
    oledWrite(display_x, display_y, "Callibrating", String(sumX), String(sumY), "");
    Serial.println(sumX);
    Serial.println(sumY);
    delay(1);
  }

  // Average offset calculations
  offset_X = (sumX/samples)*corrector*gravity;
  offset_Y = (sumY/samples)*corrector*gravity;
  
  oledWrite(display_x, display_y, "Callibrating", String(offset_X), String(offset_Y), "");
  Serial.print("Offset calculated to be: ");
  Serial.println(offset_X);
  Serial.println(offset_Y);
  delay(2000);

}

void setup() {
  Serial.begin(9600);
  u8g.setFont(u8g_font_unifont);
  adxl.powerOn();
  adxl.setRangeSetting(range); // Initiliaze adxl Range settings
  calibrator(); // Executes callibration
  prevTime = millis(); // Initiliaze time tracking
}

void loop() {
  distance(); // Runs full code for distance calculations
  total_distance += abs(total_distance); // Adds total distance travelled

  Serial.print("Distance X dm: ");
  Serial.println(distance_X*10, 4);

  oledWrite(display_x, display_y, "Distance traveled:", "X: "+String(distance_X*100)+" cm", "Y: "+String(distance_Y*10)+" cm", "Total: "+String(total_distance*10)+ " cm");
  delay(1);
}

void distance () {
  int x, y, z;
  unsigned long currTime = millis();              // Current time
  double dt = (currTime - prevTime) / 1000.0;     // Convert elapsed time to seconds
  prevTime = currTime;
      // Read acceleration data
      adxl.readAccel(&x, &y, &z);
      // ADXL345 sensitivity is 3.9 mg/LSB for ±2g range (scale factor: 0.0039)
      // Integrate acceleration to calculate velocity
  
      accel_X = (x * corrector * gravity) - offset_X; // Calculates readings into m/s^2 acceleration values with callibration

      // Code checks if accelerometer in X-direction is stationary, if not stationary reset acceleration values
      if (abs(accel_X) < stationary_accel){
        isStationary = true;
        accel_X = 0;
      }
      else {
        isStationary = false;
        lastMoveTime = currTime;
      }
  
      velocity_X += accel_X * dt; // Converts acceleration to velocity with time change
      velocity_X *= drift_stabilizer;

      // if not stationary reset acceleration values
      if (abs(velocity_X) < stationary_velocity){
        isStationary = true;
      }
      if (isStationary){
        velocity_X = 0;
      }

      distance_X += velocity_X * dt;


      // Y-readings
      accel_Y = (y * corrector * gravity) - offset_Y;

      if (abs(accel_Y) < stationary_accel){
        isStationary = true;
        accel_X = 0;
      }
      else {
        isStationary = false;
        lastMoveTime = currTime;
      }

      velocity_Y += accel_Y * dt;
      velocity_Y *= drift_stabilizer;
      if (abs(velocity_Y) < stationary_velocity){
        isStationary = true;
      }

      if (isStationary){
        velocity_Y = 0;
      }

      distance_Y += velocity_Y * dt;

      total_distance = sqrt((pow(distance_X, 2))+(pow(distance_Y, 2))); // Calculates total distance with pythagoras-formula

      //returns all values (this part isn't necessary at the moment)
      return distance_X;
      return distance_Y;
      return total_distance;
}

  // Outputs information to screen in 4 rows, title, X: distance, Y: distance, total distance:
  void oledWrite(int x, int y, String title, String stats, String stats2, String stats3) {

    u8g.firstPage();
    do {
      u8g.drawStr(x, y, title.c_str());
      u8g.drawStr(x, y+15, stats.c_str());
      u8g.drawStr(x, y+30, stats2.c_str());
      u8g.drawStr(x, y+45, stats3.c_str());
    } while (u8g.nextPage());
  
}




/* 
*  Name: Date, clock and temperature displayer
*  Author: Jim Palo
*  Description: Code for an Arduino UNO that measures date, time, and temp and displays on a U8GLIB_SSD1306_128X64 screen
*  The code also includes a tick tock sound that occurs every second, sound is produced with servo and piezo component
*/ 

#include "U8glib.h"
#include <RTClib.h>
#include <Wire.h>
#include <Servo.h>
// Libraries

RTC_DS3231 rtc;
U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_NO_ACK);
Servo myservo;
// Classes

const int tmpPin = A1;
const int soundPin = A2;
int pos = 0;
// Pins and variables

void setup() {


  Serial.begin(9600);
  Wire.begin();
  rtc.begin();
  rtc.adjust(DateTime(F(__DATE__),F(__TIME__)));
  //rtc.adjust(DateTime(2019, 1, 21, 5, 0, 0));
  u8g.setFont(u8g_font_unifont);

  pinMode(tmpPin, INPUT);
  myservo.attach(9);
  pinMode(soundPin, INPUT);
}


void loop() {


  DateTime now = rtc.now();
  oledWrite (0, 10 ,("Date: " + String(now.day()) + "." + String(now.month()) + "." + String(now.year())) ,("Time: " + String(now.hour()) + ":" + String(now.minute()) + ":" + String(now.second())) , ("Temp: " + String(getTemp())));
  if (pos >= 180) {
    pos = pos - 180;
    tone(soundPin, 500);
  }
  else {
    pos = pos + 180;
    tone(soundPin, 700);
  }
  myservo.write(pos);


  delay (50);
  noTone(soundPin);

  delay(950);

}

/*
*This function reads time from an ds3231 module and measures the temperature with " " which gets packaged in a String
*The function also makes the servo spin by 180 degrees back and forward including sending out a frequency every other time
*Parameters: Void
*Returns: information in Date: day:month:year Time: hh:mm:ss Temp:tmp as String
*/

  float getTemp() {

  int Vo;
  float R1 = 10000;  // value of R1 on board
  float logR2, R2, T;
  float c1 = 0.001129148, c2 = 0.000234125, c3 = 0.0000000876741;  //steinhart-hart coeficients for thermistor

  Vo = analogRead(tmpPin);
  R2 = R1 * (1023.0 / (float)Vo - 1.0);  //calculate resistance on thermistor
  logR2 = log(R2);
  T = (1.0 / (c1 + c2 * logR2 + c3 * logR2 * logR2 * logR2));  // temperature in Kelvin
  T = T - 273.15;                                              //convert Kelvin to Celcius

  return float(T);
}

// reads temperature and converts it to Celcius


  void oledWrite(int x, int y, String date, String time, String temp_display) {
    
    
    u8g.firstPage();
    do {

      u8g.drawStr(x, y, date.c_str());
      u8g.drawStr(x, y+20, time.c_str());
      u8g.drawStr(x, y+40, temp_display.c_str());

    } while (u8g.nextPage());
  }
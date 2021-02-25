/*
 * Connections:
 * ---- SD CARD -----
 * Ground, VCC (5V)
 * MISO -> Digital 12
 * MOSI -> Digital 11
 * SCK -> DigitaL 13
 * cs -> Digital 10
 * ------------------
 * Light A0
 * Sound A1 
 * 
 * SD CARD:
 * SDA - A4
 * SCL - A5
 * 
 * RTC DS3231
 * SDA - SDA
 * SCL - SCL
 * 
 */

#include <Wire.h> 
#include <SPI.h>
#include <SD.h>
#include <RTClib.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27,20,4);  // set the LCD address to 0x27 for a 16 chars and 2 line display
RTC_DS3231 rtc;
#define lightSensor A0 
#define soundSensor A1
File myFile;
/*********************************************************/
void save_values(int light, int sound) {
DateTime rightNow = rtc.now();
myFile = SD.open("values2.csv", FILE_WRITE);
myFile.print(rightNow.hour(), DEC);
myFile.print(':');
myFile.print(rightNow.minute(), DEC);
myFile.print(':');
myFile.print(rightNow.second(), DEC);
myFile.print(",");
myFile.print(light); 
myFile.print(","); 
myFile.println(sound); 
myFile.close();
}
void setup()
{
  lcd.init();  //initialize the lcd
  lcd.backlight();  //open the backlight 
 
  
  Serial.begin(9600);
Serial.print("Initializing SD card...");
if (!SD.begin(10)) {
Serial.println("initialization failed!");
while (1);
}
Serial.println("initialization done.");
  
  
  lcd.setCursor ( 0, 0 );            // go to the top left corner
  lcd.print("    Start Counting    "); // write this string on the top row
  lcd.setCursor ( 0, 1 );            // go to the 2nd row
  lcd.print("   IIC/I2C LCD2004  "); // pad string with spaces for centering
  lcd.setCursor ( 0, 2 );            // go to the third row
  lcd.print("  20 cols, 4 rows   "); // pad with spaces for centering
//  lcd.setCursor ( 0, 3 );            // go to the fourth row
//  lcd.print(" www.sunfounder.com ");


// as far as my understanding goes, this will only be needed if the clock is completely disconnected from power
// so, this should be run on the very first usage of the clock but should not be needed again
if (!rtc.begin()) { //(!rtc.initialized()) {
    // following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__))); //example: to set January 27 2017 at 12:56, input rtc.adjust(DateTime(2017, 1, 27, 12, 56, 0));
    
myFile = SD.open("values.csv", FILE_WRITE);
DateTime rightNow = rtc.now();
myFile.print(rightNow.year(), DEC);
myFile.print('/');
myFile.print(rightNow.month(), DEC);
myFile.print('/');
myFile.println(rightNow.day(), DEC);
myFile.close();
}
/*********************************************************/
}
void loop() 
{
  delay(1000); //1 sec
  delay(1000); //1 sec
  delay(1000); //1 sec

  lcd.clear();
  int counter = 0;
  int lightLevel; //initialize
  int soundLevel;
  rtc.begin();
  DateTime rightNow = rtc.now();
  String year_ = String(rightNow.year());
  String month_ = String(rightNow.month());
  String day_ = String(rightNow.day());
  String date = year_ + "_" + month_ + "_" + day_;

  bool save = false;
  DateTime one_day (rightNow + TimeSpan(1,0,0,0)); //1 day 0 hr 0 min 0sec
  save = rtc.setAlarm1(one_day, DS3231_A1_Day);
  while (save) {
  lightLevel = analogRead(lightSensor);
  soundLevel = analogRead(soundSensor);
    counter++;

  lcd.setCursor ( 0, 0 );            // go to the top left corner
  lcd.print("    Time: " ); // write this string on the top row
  lcd.print( counter ); // write this string on the top row
  lcd.setCursor ( 0, 1 );            // go to the 2nd row
  lcd.print("   Light Level: "); // pad string with spaces for centering
    lcd.print(lightLevel); // pad string with spaces for centering

  lcd.setCursor ( 0, 2 );            // go to the third row
  lcd.print("  Sound Level: "); // pad with spaces for centering
      lcd.print(soundLevel); // pad string with spaces for centering

  save_values(lightLevel, soundLevel);
 
    delay(500); //half a second

  }
  Serial.println("END");
    lcd.setCursor ( 0, 3 );            // go to the fourth row
  lcd.print(" DONE ");
  
  while(1){
    lightLevel = analogRead(lightSensor);
  soundLevel = analogRead(soundSensor);
    counter++;

  lcd.setCursor ( 0, 0 );            // go to the top left corner
  lcd.print("    Time: " ); // write this string on the top row
  lcd.print( counter ); // write this string on the top row
  lcd.setCursor ( 0, 1 );            // go to the 2nd row
  lcd.print("   Light Level: "); // pad string with spaces for centering
    lcd.print(lightLevel); // pad string with spaces for centering

  lcd.setCursor ( 0, 2 );            // go to the third row
  lcd.print("  Sound Level: "); // pad with spaces for centering
      lcd.print(soundLevel); // pad string with spaces for centering
 delay(1000); //1 sec
    }
   
}
/************************************************************/

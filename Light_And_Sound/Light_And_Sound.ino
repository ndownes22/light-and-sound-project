/*
 * Dream Team U, Light and Sound Project
 * Connections:
 * ---- LCD -----
 * SCL -> SCL or A5 (I2C Clock)
 * SDA -> SDA or A4 (I2C Data))
 * Vin -> 5V DC
 * GND -> GND
 * ------------------
 * ---- RTC DS3231 -----
 * SCL -> SCL or A5 (I2C Clock)
 * SDA -> SDA or A4 (I2C Data)
 * Vin -> 3.3-5.5V DC
 * GND -> GND
 * ------------------
 * ---- SD CARD -----
 * Ground, VCC (5V)
 * MISO -> Digital 12
 * MOSI -> Digital 11
 * SCK -> DigitaL 13
 * cs -> Digital 10
 * ------------------
 * ---- Lux Sensor -----
 * SCL -> SCL or A5 (I2C Clock)
 * SDA -> SDA or A4 (I2C Data)
 * Vin -> 3.3-5V DC
 * GND -> GND
 * ------------------
 * ---- Sound Sensor -----
 * Envelope -> A0 (Analog)
 * Vin -> 3.3-5.5V DC
 * GND -> GND
 * The Gate output is a binary indication 
 * that is high when sound is present, and 
 * low when conditions are quiet.
 * ------------------ 
 * 
 */

#include <Wire.h>                 // Needed for I2C (SDA,SCL)
#include <LiquidCrystal_I2C.h>    // LCD library
LiquidCrystal_I2C lcd(0x27,20,4); // set the LCD address to 0x27 for a 16 chars and 2 line display
#include <SPI.h>                  // Needed for the microSD card module
#include <SD.h>
#include <Adafruit_Sensor.h>      // Lux sensor
#include "Adafruit_TSL2591.h"     // Lux sensor
#include "RTClib.h"
#define PIN_ANALOG_IN A0 

RTC_DS3231 rtc;
Adafruit_TSL2591 tsl = Adafruit_TSL2591(2591); // Lux sensor identifier
File myFile;

/*********************************************************/
//void save_values(int light, int sound) {
//  DateTime rightNow = rtc.now();
//  myFile = SD.open("values.csv", FILE_WRITE);
//  myFile.print(rightNow.hour(), DEC);
//  myFile.print(':');
//  myFile.print(rightNow.minute(), DEC);
//  myFile.print(':');
//  myFile.print(rightNow.second(), DEC);
//  myFile.print(",");
//  myFile.print(light); 
//  myFile.print(","); 
//  myFile.println(sound); 
//  myFile.close();
//}

/***************** Configure Lux Sensor **************************/
void configureSensor(void)
{
  // You can change the gain on the fly, to adapt to brighter/dimmer light situations
  //tsl.setGain(TSL2591_GAIN_LOW);    // 1x gain (bright light)
  tsl.setGain(TSL2591_GAIN_MED);      // 25x gain
  //tsl.setGain(TSL2591_GAIN_HIGH);   // 428x gain
  
  // Changing the integration time gives you a longer time over which to sense light
  // longer timelines are slower, but are good in very low light situtations!
  //tsl.setTiming(TSL2591_INTEGRATIONTIME_100MS);  // shortest integration time (bright light)
  // tsl.setTiming(TSL2591_INTEGRATIONTIME_200MS);
  tsl.setTiming(TSL2591_INTEGRATIONTIME_300MS);
  // tsl.setTiming(TSL2591_INTEGRATIONTIME_400MS);
  // tsl.setTiming(TSL2591_INTEGRATIONTIME_500MS);
  // tsl.setTiming(TSL2591_INTEGRATIONTIME_600MS);  // longest integration time (dim light)

  /* Display the gain and integration time for reference sake */  
  Serial.println(F("------------------------------------"));
  Serial.print  (F("Gain:         "));
  tsl2591Gain_t gain = tsl.getGain();
  switch(gain)
  {
    case TSL2591_GAIN_LOW:
      Serial.println(F("1x (Low)"));
      break;
    case TSL2591_GAIN_MED:
      Serial.println(F("25x (Medium)"));
      break;
    case TSL2591_GAIN_HIGH:
      Serial.println(F("428x (High)"));
      break;
    case TSL2591_GAIN_MAX:
      Serial.println(F("9876x (Max)"));
      break;
  }
  Serial.print  (F("Timing:       "));
  Serial.print((tsl.getTiming() + 1) * 100, DEC); 
  Serial.println(F(" ms"));
  Serial.println(F("------------------------------------"));
  Serial.println(F(""));
}

void printError(String error) {
  // We will reserve the fourth row of the LCD for any errors
  lcd.setCursor(0,3);
  lcd.print("Error: " + error);
}

void setup()
{
  Serial.begin(9600);  
  // --------------- Set up the Lux Sensor ------------------
  if (!tsl.begin()) 
    printError("Lux Sensor");
  configureSensor();
  //--------------------------------------------------

  // --------------- Set up the LCD ------------------
  lcd.init();             // Initialize the lcd
  lcd.backlight();        // Open the backlight 
  lcd.setCursor ( 0, 0 ); // Go to the top left corner
  lcd.print("Date: ");    // Write this string on the top row
  lcd.setCursor ( 0, 1 );
  lcd.print("Light: ");
  lcd.setCursor ( 0, 2 );
  lcd.print("Sound: ");
  //--------------------------------------------------


  // --------------- Set up the RTC ------------------
  if (!rtc.begin())
    printError("Couldn't find RTC");
  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  //  if (rtc.lostPower()) {
  //    Serial.println("RTC lost power, lets set the time!");
  //    // Following line sets the RTC to the date & time this sketch was compiled
  //    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  //  }
  //--------------------------------------------------

  
/***************** Configure MicroSD Card **************************/
  if (!SD.begin(10)) // This begins use of the SPI bus. Parameter is CS pin 10
    printError("SD Card Initialization Failed!"); 
  myFile = SD.open("values.txt", FILE_WRITE);
  if (!myFile)
    printError("SD Card Open");
/******************************************************************/

}

void loop() 
{
  DateTime rightNow = rtc.now();
  String year_ = String(rightNow.year());
  String month_ = String(rightNow.month());
  String day_ = String(rightNow.day());
  String date = year_ + "_" + month_ + "_" + day_;

//  bool save = false;
  DateTime one_day (rightNow + TimeSpan(1,0,0,0)); //1 day 0 hr 0 min 0sec
//  save = rtc.setAlarm1(one_day, DS3231_A1_Day);

// Sound sensor
  int soundLevel;
  soundLevel = analogRead(PIN_ANALOG_IN);

  // Lux sensor
  // Read 32 bits with top 16 bits IR, bottom 16 bits full spectrum
  uint32_t lum = tsl.getFullLuminosity();
  uint16_t ir, full;
  ir = lum >> 16;
  full = lum & 0xFFFF;
  int lux = tsl.calculateLux(full, ir);

  // Print to LCD screen
  lcd.clear();
  lcd.setCursor(0,0);   // Go to the top left corner
  lcd.print("Date: ");  // Write this string on the top row
  lcd.print(rightNow.month(), DEC);
  lcd.print('/');
  lcd.print(rightNow.day(), DEC);
  lcd.print('/');
  lcd.print(rightNow.year(), DEC);
      
  lcd.setCursor (0,1);
  lcd.print("Light Level: ");
  lcd.print(lux);
  lcd.print(" Lux"); 
  
  lcd.setCursor (0,2);
  lcd.print("Sound Level: ");
  lcd.print(soundLevel); 


//  save_values(lux, soundLevel);
  myFile.print(rightNow.hour(), DEC);
  myFile.print(':');
  myFile.print(rightNow.minute(), DEC);
  myFile.print(':');
  myFile.print(rightNow.second(), DEC);
  myFile.print(",");
  myFile.print(lux); 
  myFile.print(","); 
  myFile.println(soundLevel); 
  myFile.close();

  
  // Print to serial monitor (for debugging)
  Serial.print("Status: ");
  Serial.println(soundLevel);
  
  delay(500); //half a second

//  Serial.println("END");
//    lcd.setCursor ( 0, 3 );            // go to the fourth row
//  lcd.print(" DONE ");
  
}
/************************************************************/

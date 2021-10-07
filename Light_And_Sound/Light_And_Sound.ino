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
 * A0 -> A0 (Analog)
 * Vin -> 3.3-5.5V DC
 * GND -> GND
 * ------------------ 
 * 
 */

#include <Wire.h> // Needed for I2C (SDA,SCL)
#include <LiquidCrystal_I2C.h> // LCD library
LiquidCrystal_I2C lcd(0x27,20,4);  // set the LCD address to 0x27 for a 16 chars and 2 line display
#include <SPI.h> // Needed for the microSD card module
#include <SD.h> // SD card library
#include "RTClib.h"
#include <Adafruit_Sensor.h>
#include "Adafruit_TSL2591.h"

#define PIN_GATE_IN 2
#define IRQ_GATE_IN  0
#define PIN_LED_OUT 9
#define PIN_ANALOG_IN A0

RTC_DS3231 rtc;
uint8_t currentDay;

Adafruit_TSL2591 tsl = Adafruit_TSL2591(2591); // pass in a number for the sensor identifier (for your use later)

//#define soundSensor A0
//#define lightSensor A1 Lux sensor uses library function to calculate lux from ir and full
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
//void soundISR()
//  int pin_val;
//
//  pin_val = digitalRead(PIN_GATE_IN);
//  digitalWrite(PIN_LED_OUT, pin_val);   
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

void createFile(DateTime rightNow)
{
   myFile = SD.open(String(rightNow.day()) + "_" + String(rightNow.month()) + "_" + String(rightNow.year()) +".csv", FILE_WRITE);
}

void setup()
{
  Serial.begin(9600);  

  currentDay = rtc.now().dayOfTheWeek();
  createFile(rtc.now());


  // --------------- Set up the Lux Sensor ------------------
  Serial.println(F("Starting Adafruit TSL2591 Test!"));
  if (tsl.begin()) 
  {
    Serial.println(F("Found a TSL2591 sensor"));
  } 
  else 
  {
    Serial.println(F("No sensor found ... check your wiring?"));
    while (1);
  }
  /* Configure the sensor */
  configureSensor();
  //--------------------------------------------------
  // Sound Sensor using Gate and Envelope
  //  Configure LED pin as output
//  pinMode(PIN_LED_OUT, OUTPUT);

  // configure input to interrupt
  pinMode(PIN_GATE_IN, INPUT);
//  attachInterrupt(IRQ_GATE_IN, soundISR, CHANGE);

  // Display status
  Serial.println("Initialized");
  //

  // --------------- Set up the LCD ------------------
  lcd.init();  //initialize the lcd
  lcd.backlight();  //open the backlight 
  
  lcd.setCursor ( 0, 0 );              // go to the top left corner
  lcd.print("Date: "); // write this string on the top row
  lcd.setCursor ( 0, 1 );              // go to the 2nd row
  lcd.print("Light: ");   // pad string with spaces for centering
  lcd.setCursor ( 0, 2 );              // go to the third row
  lcd.print("Sound: ");   // pad with spaces for centering
//lcd.setCursor ( 0, 3 );            // go to the fourth row
//lcd.print("Last Row");
  //--------------------------------------------------


  // --------------- Set up the RTC ------------------
  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }
  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
//  if (rtc.lostPower()) {
//    Serial.println("RTC lost power, lets set the time!");
//    // Following line sets the RTC to the date & time this sketch was compiled
//    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
//  }
  //--------------------------------------------------
 
  Serial.print("Initializing SD card...");
  if (!SD.begin(10)) {
  Serial.println("initialization failed!");
  while (1);
  }
  Serial.println("initialization done.");
  
  // as far as my understanding goes, this will only be needed if the clock is completely disconnected from power
  // so, this should be run on the very first usage of the clock but should not be needed again
  if (!rtc.begin()) { //(!rtc.initialized()) {
      // following line sets the RTC to the date & time this sketch was compiled
      rtc.adjust(DateTime(F(__DATE__), F(__TIME__))); //example: to set January 27 2017 at 12:56, input rtc.adjust(DateTime(2017, 1, 27, 12, 56, 0));
      
//  myFile = SD.open("values_rtc.csv", FILE_WRITE);
//  DateTime rightNow = rtc.now();
//  myFile.print(rightNow.year(), DEC);
//  myFile.print('/');
//  myFile.print(rightNow.month(), DEC);
//  myFile.print('/');
//  myFile.println(rightNow.day(), DEC);
//  myFile.close();
  }
}

///***************** Lux Sensor Read Config **************************/
//void advancedRead(void)
//{
//  // More advanced data read example. Read 32 bits with top 16 bits IR, bottom 16 bits full spectrum
//  // That way you can do whatever math and comparisons you want!
//  uint32_t lum = tsl.getFullLuminosity();
//  uint16_t ir, full;
//  ir = lum >> 16;
//  full = lum & 0xFFFF;
//  Serial.print(F("[ ")); Serial.print(millis()); Serial.print(F(" ms ] "));
//  Serial.print(F("IR: ")); Serial.print(ir);  Serial.print(F("  "));
//  Serial.print(F("Full: ")); Serial.print(full); Serial.print(F("  "));
//  Serial.print(F("Visible: ")); Serial.print(full - ir); Serial.print(F("  "));
//  Serial.print(F("Lux: ")); Serial.println(tsl.calculateLux(full, ir), 6);
//}

void loop() 
{

  rtc.begin();
  DateTime rightNow = rtc.now();
  String year_ = String(rightNow.year());
  String month_ = String(rightNow.month());
  String day_ = String(rightNow.day());
  String date = year_ + "_" + month_ + "_" + day_;

//  bool save = false;
  DateTime one_day (rightNow + TimeSpan(1,0,0,0)); //1 day 0 hr 0 min 0sec
//  save = rtc.setAlarm1(one_day, DS3231_A1_Day);

// Sound Sensor
  int soundLevel;
  // Check the envelope input
  soundLevel = analogRead(PIN_ANALOG_IN);

  // Convert envelope value into a message
  Serial.print("Status: ");
  Serial.println(soundLevel);
  

//  soundLevel = analogRead(soundSensor);

  // More advanced data read example. Read 32 bits with top 16 bits IR, bottom 16 bits full spectrum
  uint32_t lum = tsl.getFullLuminosity();
  uint16_t ir, full;
  ir = lum >> 16;
  full = lum & 0xFFFF;
  Serial.print(F("[ ")); Serial.print(millis()); Serial.print(F(" ms ] "));
  Serial.print(F("IR: ")); Serial.print(ir);  Serial.print(F("  "));
  Serial.print(F("Full: ")); Serial.print(full); Serial.print(F("  "));
  Serial.print(F("Visible: ")); Serial.print(full - ir); Serial.print(F("  "));
  Serial.print(F("Lux: ")); Serial.println(tsl.calculateLux(full, ir), 6);
  int lux = tsl.calculateLux(full, ir);

//{
//  advancedRead();
//  delay(500);
  
//  delay(1000); //1 sec
//  delay(1000); //1 sec
//  delay(1000); //1 sec

  lcd.clear();

  lcd.setCursor ( 0, 0 );            // go to the top left corner
  lcd.print("Date: " ); // write this string on the top row
  lcd.print(rightNow.hour(), DEC);
  lcd.print('/');
  lcd.print(rightNow.minute(), DEC);
  lcd.print('/');
  lcd.print(rightNow.second(), DEC);
      
  lcd.setCursor ( 0, 1 );            // go to the 2nd row
  lcd.print("Light Level: "); // pad string with spaces for centering
  lcd.print(lux);
  lcd.print(" Lux"); 
  
  lcd.setCursor ( 0, 2 );            // go to the third row
  lcd.print("Sound Level: "); // pad with spaces for centering
  lcd.print(soundLevel); 


//  save_values(lux, soundLevel);

  // Create a new file if the day has changed
  if(currentDay != rightNow.dayOfTheWeek())
  {
    myFile.close();
    currentDay = rightNow.dayOfTheWeek();
    createFile(rightNow);
  }

  // Save values to file
  myFile.print(rightNow.hour(), DEC);
  myFile.print(':');
  myFile.print(rightNow.minute(), DEC);
  myFile.print(':');
  myFile.print(rightNow.second(), DEC);
  myFile.print(",");
  myFile.print(lux); 
  myFile.print(","); 
  myFile.println(soundLevel); 

 
  
  delay(500); //half a second

//  Serial.println("END");
//    lcd.setCursor ( 0, 3 );            // go to the fourth row
//  lcd.print(" DONE ");
  
}
/************************************************************/

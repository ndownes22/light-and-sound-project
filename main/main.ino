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
String error = "";
int index = 0;
uint8_t currentDay;

volatile byte lcd_toggle = 0; // Used in Backlight_ISR to toggle the LCD backlight
volatile int patientIndex = 0; // Used in PatientSelect ISR to index the circular patient array
unsigned long lastInterrupt = 0;
unsigned long lastLightInterrupt = 0;
//volatile String patientArray[3] = {"Patient1", "Patient2", "Patient3"}; // circular array used to select patients

/*********************************************************/
void save_values(DateTime rightNow, int lux, int soundLevel) {
  //myFile = SD.open("patient_1/values.csv", FILE_WRITE);
  myFile.print(rightNow.hour(), DEC);
  myFile.print(':');
  myFile.print(rightNow.minute(), DEC);
  myFile.print(':');
  myFile.print(rightNow.second(), DEC);
  myFile.print(",");
  myFile.print(lux); 
  myFile.print(","); 
  myFile.println(soundLevel); 
  //myFile.close();
}

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

void printError() {
  // We will reserve the fourth row of the LCD for any errors
  lcd.setCursor(0,3);
  if (index + 20 >= error.length()) {
    lcd.print(error.substring(index));
    if (index + 1 == error.length()) {
      index = 0;
    } else {
      index = index + 1;
    }
  } else {
    lcd.print(error.substring(index,index + 20));
    index = index + 1;
  }
}

void BacklightISR() {
  if(millis() - lastLightInterrupt > 75){
    if (lcd_toggle == 1){
      lcd_toggle = 0;
    }
    else if (lcd_toggle == 0){
      lcd_toggle = 1;
    }
    lastLightInterrupt = millis();

  }
}

void PatientSelect(){
  if(millis() - lastInterrupt > 75){    

    patientIndex = (patientIndex+1) % 3;

    lastInterrupt = millis();

    }

    for (unsigned int i = 0; i < 20; i++)
      delayMicroseconds(15000);
  
  //patientIndex = (patientIndex+1) % 3;
}

void createFile(DateTime rightNow)
{
  String temp = "Patient" + String(patientIndex+1) + "/" + String(rightNow.month()) + "_" + String(rightNow.day()) + ".csv"; // There seems to be a limit to the file name length
  myFile = SD.open(temp, FILE_WRITE);
}

void setup()
{
  Serial.begin(9600);  
  
  // --------------- Set up the Lux Sensor ------------------
  if (!tsl.begin()) 
    error = error + "Lux Sensor ";
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
    error = error + "Couldn't find RTC ";
  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  //rtc.adjust(DateTime(2021, 11, 18, 23, 59, 0)); //testing day change
  currentDay = rtc.now().dayOfTheWeek();
  //  if (rtc.lostPower()) {
  //    Serial.println("RTC lost power, lets set the time!");
  //    // Following line sets the RTC to the date & time this sketch was compiled
  //    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  //  }
  //--------------------------------------------------

  
/***************** Configure MicroSD Card **************************/
  if (!SD.begin(10)) // This begins use of the SPI bus. Parameter is CS pin 10
    error = error + "SD Card Initialization Failed! "; 
  createFile(rtc.now());
  if (!myFile)
    error = error + "Could Not Open File ";
/******************************************************************/

  // -------- Set up the Buttons/Interrupts ----------
  pinMode(2, INPUT);  // using digital pin 2 as the backlight switch
  pinMode(3, INPUT);  // using digital pin 3 as the patient select
  attachInterrupt(digitalPinToInterrupt(2), BacklightISR, FALLING);
  attachInterrupt(digitalPinToInterrupt(3), PatientSelect, FALLING);
  //--------------------------------------------------
}

void loop() 
{
  DateTime rightNow = rtc.now();
  String year_ = String(rightNow.year());
  String month_ = String(rightNow.month());
  String day_ = String(rightNow.day());
  String date = year_ + "_" + month_ + "_" + day_;

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
  lcd.print("Patient: ");
  lcd.print(patientIndex+1); 

      
  lcd.setCursor (0,1);
  lcd.print("Light Level: ");
  lcd.print(lux);
  lcd.print(" Lux"); 
  
  lcd.setCursor (0,2);
  lcd.print("Sound Level: ");
  lcd.print(soundLevel); 

  printError();
  
   // Create a new file if the day has changed
  if(currentDay != rightNow.dayOfTheWeek())
  {
    myFile.close();
    currentDay = rightNow.dayOfTheWeek();
    createFile(rightNow);
  }

  save_values(rightNow, lux, soundLevel);

  // turns the backlight off at 7PM and back on at 7AM
  // will be turned off/on regardless of the button press for the backlight
  if(String(rightNow.hour()) == "19" && String(rightNow.minute()) == "00" || lcd_toggle == 1){
    lcd.noBacklight();
  }
  if(String(rightNow.hour()) == "07" && String(rightNow.minute()) == "00" || lcd_toggle == 0){
    lcd.backlight();
  }

  for (unsigned int i = 0; i < 20; i++){
    // delayMicroseconds is needed for interrupts
    delayMicroseconds(15000); // delays 15us 20 times = 300us
  }

  
  // Print to serial monitor (for debugging)
  /*
  Serial.print("Sound Level: ");
  Serial.print(soundLevel);
  Serial.print("    Lux Level: ");
  Serial.println(lux);
  Serial.println(String(rightNow.day()) + "_" + String(rightNow.month()) + "_" + String(rightNow.year()) +".csv");
  */
  int prevIndex = patientIndex;
  Serial.print("Index: ");
  Serial.println(prevIndex);
}
/************************************************************/

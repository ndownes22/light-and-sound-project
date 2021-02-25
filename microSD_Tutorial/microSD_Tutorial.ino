/* 
 * Tutorial from https://create.arduino.cc/projecthub/electropeak/sd-card-module-with-arduino-how-to-read-write-data-37f390 
 * Uses SPI to write and read data to microSD card
 * ---- SD CARD -----
 * Ground, VCC (5V)
 * MISO -> Digital 12
 * MOSI -> Digital 11
 * SCK -> DigitaL 13
 * cs -> Digital 10
 * ------------------
 */
#include <SPI.h>
#include <SD.h>
File myFile;
void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  // Note: We should not wait for Serial in our Light and Sound project. 
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  Serial.print("Initializing SD card...");
  if (!SD.begin(10)) {
    Serial.println("initialization failed!");
    while (1);
  }
  Serial.println("initialization done.");
  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  myFile = SD.open("test.txt", FILE_WRITE);
  // if the file opened okay, write to it:
  if (myFile) {
    Serial.print("Writing to test.txt...");
    myFile.println("This is a test file :)");
    myFile.println("testing 1, 2, 3.");
    for (int i = 0; i < 20; i++) {
      myFile.println(i);
    }
    // close the file:
    myFile.close();
    Serial.println("done.");
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening test.txt");
  }
}
void loop() {
  // nothing happens after setup
}

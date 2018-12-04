// Tyler Brown and Marie Benavides 2018
// Sample code from (c) Michael Schoeffler 2017, http://www.mschoeffler.de

//SD card setup
#include <SPI.h>
#include <SD.h> // include SD card library

//Load cell and LCD screen setup
#include <HX711_ADC.h>         // https://github.com/olkal/HX711_ADC
#include <Wire.h>
#include <LiquidCrystal_I2C.h> // LiquidCrystal_I2C library
HX711_ADC LoadCell(4, 5);      // parameters for load cell
LiquidCrystal_I2C lcd(0x27, 16,2); // 0x27 is the i2c address of the LCM1602 IIC v1 module (might differ)


// Global Variables declared here for timing purposes
unsigned long senseInterval = 5;   // Time between readings in seconds (10 mins)
unsigned long currMillis = 0;      // How long has passed since sketch started
unsigned long lastMillis = 0;      // Time when last reading was taken

int counter = 0;     // Used to count loop iterations
int chipSelect = 10; //chipSelect pin for the SD card Reader
float prevMass = 0;  // Used when deciding if enough mass has been added to record new mass

void setup() {
  Serial.begin(9600); //Start the program
  SD.begin(10);       // because SD is in digitial input 10 ***CHECK THIS***
  while (!Serial) {   // so that nothing is missed during initialization
    ;
  }
  
  // Initialize the SD card
  Serial.print("Initializing SD card..."); 
  pinMode(10, OUTPUT);         //Declare 10 an output and reserve it
  digitalWrite(10, HIGH);      //Provide power to SD card

  //If initialization fails, tell us
  if(!SD.begin(chipSelect)) {
    Serial.println("Card Failed");  
    while (1);
  }
  Serial.println("Card Ready"); //SD card is good to go!
 
  Serial.println("Cumulative Mass (g)");  //Creates a header
  LoadCell.begin();                       // start connection to HX711
  LoadCell.start(2000);                   // load cell gets 2 seconds to stabilize
  LoadCell.setCalFactor(900);             // calibration factor for load cell (dependent on individual setup)
  lcd.init();                             // begins connection to the LCD module
  lcd.backlight();                        // turns on the backlight
  delay(2000);                            // Delay because sensor may be slow to boot up
}

void loop() {
  currMillis = millis();            // Variable for total run time in ms
  LoadCell.update();                // Load cell takes a reading
  float mass = LoadCell.getData();  // set variable equal to load cell output
  lcd.setCursor(0, 0);              // set cursor to first row
  lcd.print("Mass [g]:");           // print out to LCD
  lcd.setCursor(0, 1);              // set cursor to secon row
  lcd.print(mass);                  // print out the retrieved value to the second row
 
  // Check to see if any reads failed and exit the loop early to try again
  if (isnan(mass)) {
    Serial.println("Failed to read from Load Cell!"); 
    return; // This exits the main loop and starts the next iteration
  }
    
  //if it is time to scan and record
  if ((currMillis - lastMillis) >= (senseInterval * 1000)) {
    Serial.println("Mass:" + String(mass) + " g");  //Print Output in serial window
    lastMillis = currMillis;                        //reset for later comparison
    counter ++;                                     //Count iterations
    
    // write to the SD card if at intervals of at least 2 grams
    if ((mass - prevMass) >= 2) {
      File myFile = SD.open("data.txt", FILE_WRITE); //open text file
      
      if (myFile) {                          // If the file is successfully opened
       myFile.print(currMillis/1000);        // Write timestamp in seconds
       myFile.print(",");                    // Comma delimiter
       myFile.println(mass);                 // recorded mass, followed by tab delimiter
       myFile.close();                       // Close the file
       Serial.println("Data written to SD"); // Tell us the data was successfully saved
      }
       else{                         // If file does not open
       Serial.println("fail");       // Tell us data was not saved
      }
    
    prevMass = mass;               // Reassign mass comparison variable for next loop
    }
  }
}

//CartnickLabs

// import libraries
#include "TimeLib.h" // for time keeping
#include <Arduino_GFX_Library.h>
#include "max6675.h"


// list SPI teensy pins
int MOSI_0_PIN = 11;  // screenMOSI
int MISO_0_PIN = 12;  // screenMISO
int SCLK_0_PIN = 13;  // screenSCLK

int MOSI_1_PIN = 26;
int MISO_1_PIN = 39;  // screenCS
int SCLK_1_PIN = 27;  

int MOSI_2_PIN = 50;  // not sure how to actually access these...
int MISO_2_PIN = 54;
int SCLK_2_PIN = 45;


// initialize variables for MAX6675s
int meatCLK = 35;
int meatCS = 34;
int meatDO = 33;
MAX6675 meatThermocouple(meatCLK, meatCS, meatDO);

// initialize variables for the screen
int screenRST = 40;
int screenBL = 22;
int screenDC = 41;
int screenCS = 39;
int screenSCLK = SCLK_0_PIN;  // pin 13
int screenMOSI = MOSI_0_PIN;  // pin 11
int screenMISO = MISO_0_PIN;  // pin 12

Arduino_DataBus *bus = create_default_Arduino_DataBus();
Arduino_GFX *gfx = new Arduino_ILI9488_18bit(bus, screenRST, 1);

// initialize variables for the timer
int startTime = 0;
int runTime = 0;

void setup() {
  // setup Serial
  Serial.begin(115200);
  while(!Serial);
  delay(1000);

  // setup MAX6675s
  Serial.print("Meat Thermometer Test Reading:\t");
  Serial.print(meatThermocouple.readFahrenheit());
  Serial.print(" F\n");

  // setup screen
  gfx->begin();
  gfx->fillScreen(BLACK);
  pinMode(screenBL, OUTPUT);
  digitalWrite(screenBL, HIGH);
  gfx->setTextSize(5);
  gfx->setCursor(10, 10);
  gfx->setTextColor(RED);
  gfx->println("Running...");
  Serial.println("Screen changed BLUE");
  
  // setup timer
  setSyncProvider(getTeensy3Time);
  startTime = now();
  Serial.print("Time started: ");
  Serial.print(startTime);
  Serial.print(" s.");

}

void loop() {
  // loop timer
  if (Serial.available()) {
    time_t t = processSyncMessage();
    if (t != 0) {
      Teensy3Clock.set(t); // set the RTC
      setTime(t);
    }
  }
  runTime = now()-startTime;
  Serial.print(runTime);
  Serial.print('\t');

  // loop for MAX6675s
  Serial.println(meatThermocouple.readFahrenheit());

  // loop screen
  gfx->fillRect(0, 0, 400, 200, BLACK);
  gfx->setCursor(10, 10);
  gfx->setTextColor(WHITE);
  gfx->print(meatThermocouple.readFahrenheit());
  gfx->print(" F");
  gfx->setCursor(10, 100);
  gfx->setTextColor(WHITE);
  gfx->println(runTime);
  
  // loop delay
  delay(1000);

}



// timer functions
time_t getTeensy3Time() {
  return Teensy3Clock.get();
}
#define TIME_HEADER  "T"   // Header tag for serial time sync message
unsigned long processSyncMessage() {
  unsigned long pctime = 0L;
  const unsigned long DEFAULT_TIME = 1357041600; // Jan 1 2013 
  if(Serial.find(TIME_HEADER)) {
     pctime = Serial.parseInt();
     return pctime;
     if( pctime < DEFAULT_TIME) { // check the value is a valid time (greater than Jan 1 2013)
       pctime = 0L; // return 0 to indicate that the time is not valid
     }
  }
  return pctime;
}

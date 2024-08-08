//CartnickLabs

// import libraries
#include "TimeLib.h" // for time keeping
#include <Arduino_GFX_Library.h>
#include "max6675.h"


// list SPI teensy pins
int MOSI_0_PIN = 11;
int MISO_0_PIN = 12;
int SCLK_0_PIN = 13;

int MOSI_1_PIN = 26;
int MISO_1_PIN = 39;  // meatDO
int SCLK_1_PIN = 27;  // meatCLK

int MOSI_2_PIN = 50;  // not sure how to actually access these...
int MISO_2_PIN = 54;
int SCLK_2_PIN = 45;


// initialize variables for MAX6675s
int meatCLK = SCLK_1_PIN;  // pin 27
int meatCS = 25;
int meatDO = MISO_1_PIN;  // pin 39
MAX6675 meatThermocouple(meatCLK, meatCS, meatDO);

// initialize variables for the screen
int screenBL = 8;
int screenDC = 9;
int screenCS = 10;
int screenSCLK = SCLK_0_PIN;  // pin 13
int screenMOSI = MOSI_0_PIN;  // pin 11
int screenMISO = MISO_0_PIN;  // pin 12
Arduino_DataBus *bus = new Arduino_HWSPI(screenDC, screenCS, screenSCLK, screenMOSI, screenMISO);
Arduino_GFX *gfx = new Arduino_ILI9341(bus, 17 /* RST */);


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
  Serial.print(" F\n")

  // setup screen
  gfx->fillScreen(BLUE);
  pinMode(screenBL, OUTPUT);
  digitalWrite(screenBL, HIGH);
  gfx->setCursor(10, 10);
  gfx->setTextColor(RED);
  gfx->println("Running...");
  Serial.println("Screen changed BLUE");
  
  // setup timer
  setSyncProvider(getTeensy3Time);
  startTime = now();
  Serial.print("Time started: ");
  Serial.print(startTime);
  Serial.print(" s.")

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
  Serial.println(thermocouple.readFahrenheit());

  // loop screen
  gfx->fillScreen(BLACK);
  gfx->setCursor(10, 10);
  gfx->setTextColor(WHITE);
  gfx->println(thermocouple.readFahrenheit());
  gfx->setCursor(10, 100);
  gfx->setTextColor(WHITE);
  gfx->println(runTime);
  
  // loop delay
  delay(1000);

}

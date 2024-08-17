// CartnickLabs

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
int thermoCLK = 37;
int thermoDO = 33;
int meatCS = 36;
int ovenCS = 35;
int roomCS = 34;
float meatTemp = 0;
float ovenTemp = 0;
float roomTemp = 0;
String lastMeatTemp = "";
String lastOvenTemp = "";
String lastRoomTemp = "";

MAX6675 meatThermocouple(thermoCLK, meatCS, thermoDO);
MAX6675 ovenThermocouple(thermoCLK, ovenCS, thermoDO);
MAX6675 roomThermocouple(thermoCLK, roomCS, thermoDO);

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
int lastRunTime = 0;
elapsedMillis thermoTimer;
elapsedMillis serialOutTimer;
elapsedMillis secondsTimer;

// initialize other variables
float tempSet = 0;


void setup() {
  // setup Serial
  Serial.begin(115200);
  delay(1000);

  // screen
  screenInit();
  countdownDisplay(4);
  
  // timer
  rtcInit();
  startTime = now();
}


void loop() {
  // loop timer
  runTime = now()-startTime;

  // gather temperatures and update screen
  if (thermoTimer >= 500) {
    meatTemp = meatThermocouple.readFahrenheit();
    ovenTemp = ovenThermocouple.readFahrenheit();
    roomTemp = roomThermocouple.readFahrenheit();
    updateDisplayTemps();
    thermoTimer -= 500;
  }

  // update screen display
  if (secondsTimer >= 1000) {
    updateDisplaySeconds();
    secondsTimer -= 1000;
  }

  // print log 
  if (serialOutTimer >= 1000) {
    // serialOutTemps();
    serialOutTimer -= 1000;
  }

}


/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 |  
 | Important junk below this block.
 |
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


String updateDisplayTemp(int x, int y, String lastTemp, float currentTemp) {
  String printTempVar = String(currentTemp) + " F";
  gfx->setCursor(x, y);                      
  gfx->setTextColor(BLACK);
  gfx->print(lastTemp);
  gfx->setCursor(x, y);
  gfx->setTextColor(WHITE);
  gfx->print(printTempVar);
  return printTempVar;
}


void updateDisplayTemps() {
  lastMeatTemp = updateDisplayTemp(10, 10, lastMeatTemp, meatTemp);
  lastOvenTemp = updateDisplayTemp(150, 10, lastOvenTemp, ovenTemp);
  lastRoomTemp = updateDisplayTemp(300, 10, lastRoomTemp, roomTemp);
}


void serialOutTemps() {
  Serial.print(runTime);
  Serial.print("\t");
  Serial.print(meatTemp);
  Serial.print("\t");
  Serial.print(ovenTemp);
  Serial.print("\t");
  Serial.println(roomTemp);
}


void updateDisplaySeconds() {
  gfx->setCursor(10, 300);                      
  gfx->setTextColor(BLACK);
  gfx->print(String(lastRunTime) + "s");
  gfx->setCursor(10, 300);
  gfx->setTextColor(WHITE);
  gfx->print(String(runTime) + "s");
  lastRunTime = runTime;
}

void screenInit(){
  gfx->begin();
  gfx->fillScreen(BLACK);
  pinMode(screenBL, OUTPUT);
  digitalWrite(screenBL, HIGH);
  gfx->setTextSize(2);
}

void countdownDisplay(int seconds){
    for (int i = seconds; i > 0; i--) {
      gfx->setCursor(10, 10);
      gfx->setTextColor(WHITE);
      gfx->print(i);
      delay(1000);
      gfx->setCursor(10, 10);
      gfx->setTextColor(BLACK);
      gfx->print(i);
  }
}


void rtcInit() {
  setSyncProvider(getTeensy3Time);
  if (Serial.available()) {
    time_t t = processSyncMessage();
    if (t != 0) {
      Teensy3Clock.set(t); // set the RTC
      setTime(t);
    }
  }
}

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

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

int roomTempList[300];
int timeList[300];

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
elapsedMillis graphResetTimer;
elapsedMillis plotTimer;

// initialize other variables
float tempSet = 0;
unsigned long int graphDomain = 300000;
unsigned long int plotPeriod = 1000;
int plotListIndex = 0;


void setup() {
  // setup Serial
  Serial.begin(115200);
  delay(1000);

  // screen
  screenInit();
  //countdownDisplay(4);
  
  // timer
  rtcInit();
  startTime = now();
  drawGraph(graphDomain / 1000);
}


void loop() {
  // loop timer
  runTime = now()-startTime;
  // gather temperatures and update screen
  if (thermoTimer >= 500) {
    gatherTemps();
    delay(250);
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
  // graph
  if (plotTimer >= plotPeriod) {
    plotPoint(runTime, round(meatTemp), YELLOW, graphDomain);
    roomTempList[plotListIndex] = round(meatTemp);
    timeList[plotListIndex] = runTime;
    plotListIndex++;
    plotTimer -= plotPeriod;
  }

  if (graphResetTimer >= graphDomain) {
    for (uint i = 0; i < (sizeof(timeList) / sizeof(timeList[0])); i++) {
      plotPoint(timeList[i], roomTempList[i], BLACK, graphDomain);
    }
    graphDomain *= 2;
    plotPeriod *= 2;
    drawGraph(graphDomain);
    for (uint i = 0; i < 150; i++) {
      plotPoint(timeList[i], roomTempList[i], BLUE, graphDomain);
    }
    plotListIndex = 150;
  } 

}


/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 |  
 | Important junk below this block.
 |
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

// Display functions  --------------------------------------------
void screenInit(){
  gfx->begin();
  gfx->fillScreen(BLACK);
  pinMode(screenBL, OUTPUT);
  digitalWrite(screenBL, HIGH);
  gfx->setTextSize(2);
}

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

void updateDisplaySeconds() {
  gfx->setCursor(10, 300);                      
  gfx->setTextColor(BLACK);
  gfx->print(String(lastRunTime) + "s");
  gfx->setCursor(10, 300);
  gfx->setTextColor(WHITE);
  gfx->print(String(runTime) + "s");
  lastRunTime = runTime;
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

void plotPoint(int xValue, int yValue, uint16_t color, int xDomain) {
  int x;
  int y;
  int yMin = 100;
  int yMax = 230;
  int boxSize[4] = {30, 60, 470, 190};
  if (yValue > 100 && yValue < 230) {
    x = map(xValue, 0, xDomain / 1000, boxSize[0], boxSize[2]);
    y = map(yValue, yMin, yMax, boxSize[3], boxSize[1]);
    if (x > boxSize[0] + 1) {
      gfx->fillRect(x - 1, y - 1, 3, 3, color);
    } else if (x == boxSize[0]) {
      gfx->fillRect(x + 1, y - 1, 1, 3, color);
    }
  } else if (yValue > 60 && yValue < 80) {
    yMin = 60;
    yMax = 80;
    boxSize[1] = 190;
    boxSize[3] = 230;
    x = map(xValue, 0, xDomain / 1000, boxSize[0], boxSize[2]);
    y = map(yValue, yMin, yMax, boxSize[3], boxSize[1]);
    if (x > boxSize[0] + 1) {
      gfx->fillRect(x - 1, y - 1, 3, 3, color);
    } else if (x == boxSize[0]) {
      gfx->fillRect(x + 1, y - 1, 1, 3, color);
    }
  }
}

void updateDisplayTemps() {
  gfx->setTextSize(2);
  gfx->setTextColor(WHITE);
  lastMeatTemp = updateDisplayTemp(10, 10, lastMeatTemp, meatTemp);
  lastOvenTemp = updateDisplayTemp(150, 10, lastOvenTemp, ovenTemp);
  lastRoomTemp = updateDisplayTemp(300, 10, lastRoomTemp, roomTemp);
}

void drawGraph(int timeBound) {
  drawYGridLines(25, 70, 470, 190, 6, 0xAD55);
  drawXGridLines(30, 60, 470, 235, 5, 0xAD55);
  drawAxes();
  labelYAxis(5, 70, 190, 6, 100, 220, WHITE);
  labelXAxis(30, 470, 240, 5, 0, timeBound, WHITE);
}

void drawAxes() {
  gfx->drawLine(10, 190, 470, 190, WHITE);  // graph separator
  gfx->drawLine(30, 60, 30, 250, WHITE);  // y-axis
  gfx->drawLine(10, 230, 470, 230, WHITE);  // x-axis
}

void labelYAxis(int x, int y, int y1, int n, int start, int end, uint16_t color) {
  gfx->setTextSize(1);
  gfx->setTextColor(color);
  int y2 = 0;
  String value = "";
  for (int i = 0; i < n; i++) {
    y2 = y + round(i * (y1-y)/n) - 4;
    gfx->setCursor(x, y2);
    value = String(trunc(end - i * (end-start)/n));
    gfx->print(value.substring(0, value.length() - 3));
  }
  gfx->setCursor(x + 4, 206);
  gfx->print("70");
}

void labelXAxis(int x, int x1, int y, int n, int start, int end, uint16_t color) {
  gfx->setTextSize(1);
  gfx->setTextColor(color);
  int x2 = 0;
  String value = "";
  for (int i = 1; i < n; i++) {
    x2 = x + round(i * (x1-x)/n) - 2;
    gfx->setCursor(x2, y);
    value = String(trunc(start + i * (end-start)/n));
    gfx->print(value.substring(0, value.length() - 3));
  }
}

void drawYGridLines(int x, int y, int x1, int y1, int n, uint16_t color){
  int y2 = 0;
  for (int i = 0; i < n; i++) {
    y2 = y + round(i * (y1-y)/n);
    gfx->drawLine(x, y2, x1, y2, color);
  }
  gfx->drawLine(x, 210, x1, 210, color);  // room temp line
}

void drawXGridLines(int x, int y, int x1, int y1, int n, uint16_t color){
  int x2 = 0;
  for (int i = 0; i < n; i++) {
    x2 = x + round(i * (x1-x)/n);
    gfx->drawLine(x2, y, x2, y1, color);
  }
}


// Information functions --------------------------------------------
void gatherTemps() {
  meatTemp = meatThermocouple.readFahrenheit();
  ovenTemp = ovenThermocouple.readFahrenheit();
  roomTemp = roomThermocouple.readFahrenheit();
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

// Timer functions  -------------------------------------------------
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

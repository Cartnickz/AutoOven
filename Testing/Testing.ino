gx1// CartnickLabs

// import libraries
#include "TimeLib.h" // for time keeping
#include <Arduino_GFX_Library.h>
#include <Adafruit_MAX31856.h>

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
int thermoCLK = 33;
int thermoDO = 34;
int thermoDI = 35;
int meatCS1 = 17;
int meatCS2 = 18;
int ovenCS = 19;
int roomCS = 20;
float meatTemp1 = 0;
float meatTemp2 = 0;
float ovenTemp = 0;
float roomTemp = 0;
String lastMeatTemp1 = "";
String lastMeatTemp2 = "";
String lastOvenTemp = "";
String lastRoomTemp = "";

int roomTempList[300];
int meatTempList1[300];
int meatTempList2[300];
int ovenTempList[300];
int timeList[300];

Adafruit_MAX31856 roomThermocouple(roomCS, thermoDI, thermoDO, thermoCLK);
Adafruit_MAX31856 ovenThermocouple(ovenCS, thermoDI, thermoDO, thermoCLK);
Adafruit_MAX31856 meatThermocouple1(meatCS1, thermoDI, thermoDO, thermoCLK);
Adafruit_MAX31856 meatThermocouple2(meatCS2, thermoDI, thermoDO, thermoCLK);

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

//initialize variables for the graphs
short int gx0 = 30;
short int gx1 = 470;
short int gy0 = 60;
short int gy1 = 190;
short int gy2 = 230;

short int yMinA = 100;
short int yMaxA = 325;
short int yMinB = 60;
short int yMaxB = 90; 

// initialize other variables
float tempSet = 0;
unsigned long int graphDomain = 300000;
unsigned long int plotPeriod = 1000;
int plotListIndex = 0;


void setup() {
  // setup Serial
  Serial.begin(115200);
  delay(2000);

  // screen
  screenInit();
  //countdownDisplay(4);
  
  // timer
  rtcInit();
  startTime = now();
  drawGraph(graphDomain / 1000);

  meatThermocouple1.begin();
  meatThermocouple1.setThermocoupleType(MAX31856_TCTYPE_K);

  meatThermocouple2.begin();
  meatThermocouple2.setThermocoupleType(MAX31856_TCTYPE_K);

  roomThermocouple.begin();
  roomThermocouple.setThermocoupleType(MAX31856_TCTYPE_K);

  ovenThermocouple.begin();
  ovenThermocouple.setThermocoupleType(MAX31856_TCTYPE_K);
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
    plotPoint(runTime, round(roomTemp), YELLOW, graphDomain / 1000);
    plotPoint(runTime, round(ovenTemp), BLUE, graphDomain / 1000);
    plotPoint(runTime, round(meatTemp1), RED, graphDomain / 1000);
    plotPoint(runTime, round(meatTemp2), ORANGE, graphDomain / 1000);
    roomTempList[plotListIndex] = round(roomTemp);
    meatTempList1[plotListIndex] = round(meatTemp1);
    meatTempList2[plotListIndex] = round(meatTemp2);
    ovenTempList[plotListIndex] = round(ovenTemp);
    timeList[plotListIndex] = runTime;
    plotListIndex++;
    plotTimer -= plotPeriod;
  }

  if (graphResetTimer >= graphDomain) {
    for (uint i = 0; i < (sizeof(timeList) / sizeof(timeList[0])); i++) {
      plotPoint(timeList[i], roomTempList[i], BLACK, graphDomain / 1000);
      plotPoint(timeList[i], meatTempList1[i], BLACK, graphDomain / 1000);
      plotPoint(timeList[i], meatTempList2[i], BLACK, graphDomain / 1000);
      plotPoint(timeList[i], ovenTempList[i], BLACK, graphDomain / 1000);
      if (i % 2 == 0) {
        roomTempList[i/2] = roomTempList[i];
        meatTempList1[i/2] = meatTempList1[i];
        meatTempList2[i/2] = meatTempList2[i];
        ovenTempList[i/2] = ovenTempList[i];
        timeList[i/2] = timeList[i]; 
      }
    }
    labelXAxis(gx0, gx1, gy2+10, 5, 0, graphDomain / 1000, BLACK);
    graphDomain *= 2;
    plotPeriod *= 2;
    drawGraph(graphDomain / 1000);
    for (uint i = 0; i < 150; i++) {
      plotPoint(timeList[i], roomTempList[i], YELLOW, graphDomain / 1000);
      plotPoint(timeList[i], ovenTempList[i], BLUE, graphDomain / 1000);
      plotPoint(timeList[i], meatTempList1[i], RED, graphDomain / 1000);
      plotPoint(timeList[i], meatTempList2[i], ORANGE, graphDomain / 1000);
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

String updateDisplayTemp(int x, int y, String lastTemp, float currentTemp, uint16_t color) {
  String printTempVar = String(currentTemp) + " F";
  gfx->setCursor(x, y);                      
  gfx->setTextColor(BLACK);
  gfx->print(lastTemp);
  gfx->setCursor(x, y);
  gfx->setTextColor(color);
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
  if (yValue > yMinA && yValue < yMaxA) {
    x = map(xValue, 0, xDomain, gx0, gx1);
    y = map(yValue, yMinA, yMaxA, gy1, gy0);
    if (x > gx0 + 1) {
      gfx->fillRect(x - 1, y - 1, 3, 3, color);
    } else if (x == gx0) {
      gfx->fillRect(x + 1, y - 1, 1, 3, color);
    }
  } else if (yValue > yMinB && yValue < yMaxB) {
    x = map(xValue, 0, xDomain, gx0, gx1);
    y = map(yValue, yMinB, yMaxB, gy2, gy1);
    if (x > gx0 + 1) {
      gfx->fillRect(x - 1, y - 1, 3, 3, color);
    } else if (x == gx0) {
      gfx->fillRect(x + 1, y - 1, 1, 3, color);
    }
  }
}

void updateDisplayTemps() {
  gfx->setTextSize(2);
  lastMeatTemp1 = updateDisplayTemp(10, 10, lastMeatTemp1, meatTemp1, RED);
  lastMeatTemp2 = updateDisplayTemp(120, 10, lastMeatTemp2, meatTemp2, ORANGE);
  lastOvenTemp = updateDisplayTemp(230, 10, lastOvenTemp, ovenTemp, BLUE);
  lastRoomTemp = updateDisplayTemp(340, 10, lastRoomTemp, roomTemp, YELLOW);
}

void drawGraph() {
  drawYGridLines(gx0-5, gy0+10, gx1, gy1,   6, 0xAD55);
  drawXGridLines(gx0,   gy0,    gx1, gy2+5, 5, 0xAD55);
  drawAxes();
  labelYAxis(gx0-25, gy0+10, gy1,     6, yMinA, yMaxA, WHITE);
  labelXAxis(gx0   , gx1   , gy2+10, 5, 0, graphDomain / 1000, WHITE);
}

void drawAxes() {
  gfx->drawLine(gx0-20, gy1, gx1, gy1, WHITE);  // graph separator
  gfx->drawLine(gx0, gy0, gx0, gy2+20, WHITE);  // y-axis
  gfx->drawLine(gx0-20, gy2, gx1, gy2, WHITE);  // x-axis
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
  gfx->setCursor(x + 4, (gy1+gy2)/2 - 4;
  gfx->print(String((yMinB+yMaxB)/2);
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
  gfx->drawLine(x, (gy1+gy2)/2, x1, (gy1+gy2)/2, color);  // room temp line
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
  //meatTemp = meatThermocouple.readFahrenheit();
  meatTemp1 = meatThermocouple1.readThermocoupleTemperature() * 9 / 5 + 32;
  meatTemp2 = meatThermocouple2.readThermocoupleTemperature() * 9 / 5 + 32;
  ovenTemp = ovenThermocouple.readThermocoupleTemperature() * 9 / 5 + 32;
  roomTemp = roomThermocouple.readThermocoupleTemperature() * 9 / 5 + 32;
}

void serialOutTemps() {
  Serial.print(runTime);
  Serial.print("\t");
  Serial.print(meatTemp1);
  Serial.print("\t");
  Serial.print(meatTemp2);
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

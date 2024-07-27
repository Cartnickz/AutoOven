#include <Arduino_GFX_Library.h>

#define GFX_BL DF_GFX_BL // defines default backlight pin
Arduino_DataBus *bus = create_default_Arduino_DataBus();
Arduino_GFX *gfx = new Arduino_ILI9488_18bit(bus, DF_GFX_RST, 1 /* rotation */, false /* IPS */);

const int dirPin = 5;
const int stepPin = 4;
const int sleepPin = 3;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  init_screen();
  delay(5000); // 5 seconds
  pinMode(stepPin, OUTPUT);
	pinMode(dirPin, OUTPUT);
  pinMode(sleepPin, OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  digitalWrite(sleepPin, HIGH);

  printScreen("TESTING MY IDEA");
  delay(2000);

  printScreen("Rotating Clockwise...");
  cw(100, 2000);
  delay(1000);
  printScreen("Rotating Counter-Clockwise");
  ccw(200, 2000);
  delay(1000);
  printScreen("Done.");

  digitalWrite(sleepPin, LOW);
  delay(2000);

}

void init_screen(){
  // Init Display
  if (!gfx->begin()) {
    Serial.println("gfx->begin() failed!");
  }
  gfx->fillScreen(BLACK); // start the screen black

  #ifdef GFX_BL
    pinMode(GFX_BL, OUTPUT);
    digitalWrite(GFX_BL, HIGH);
  #endif

}

char printScreen(char log[]){
  gfx->fillRect(0, 290, 480, 30, BLUE);
  gfx->setTextColor(WHITE);
  gfx->setCursor(10, 300);
  gfx->println(log);
}

int cw(int steps, int delayMicro) {
  digitalWrite(dirPin, HIGH);
  for(int x = 0; x < steps; x++) {
      digitalWrite(stepPin, HIGH);
      delayMicroseconds(delayMicro);
      digitalWrite(stepPin, LOW);
      delayMicroseconds(delayMicro);
  }
}

int ccw(int steps, int delayMicro) {
  digitalWrite(dirPin, LOW);
    for(int x = 0; x < steps; x++) {
      digitalWrite(stepPin, HIGH);
      delayMicroseconds(delayMicro);
      digitalWrite(stepPin, LOW);
      delayMicroseconds(delayMicro);
  }
}

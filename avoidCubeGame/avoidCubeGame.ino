#include <Adafruit_GFX.h>
#include <MCUFRIEND_kbv.h>
MCUFRIEND_kbv tft;
#include <TouchScreen.h>

#if defined(__SAM3X8E__)
#undef __FlashStringHelper::F(string_literal)
#define F(string_literal) string_literal
#endif

uint8_t YP = A1;
uint8_t XM = A2;
uint8_t YM = 7;
uint8_t XP = 6;
uint8_t SwapXY = 0;

uint16_t TS_LEFT = 880;
uint16_t TS_RT = 170;
uint16_t TS_TOP = 950;
uint16_t TS_BOT = 180;
char* name = "Unknown controller";

TouchScreen ts = TouchScreen(XP, YP, XM, YM, 250);
TSPoint tp;

#define MINPRESSURE 20
#define MAXPRESSURE 1000

#define SWAP(a, b) \
  { \
    uint16_t tmp = a; \
    a = b; \
    b = tmp; \
  }

uint16_t identifier;

// Assign human-readable names to some common 16-bit color values
#define BLACK 0x0000
#define RED 0xF800
#define WHITE 0xFFFF

// your starting position
int xpos = 160;
int ypos = 240;
// the old posiion of the red square to remove it
int oldXpos, oldYpos;

// bulletsize Position and Speed
int bulletSize = 24;
int bulletPosX = 4;
int bulletPosY = 4;
double speedX = 3;
double speedY = 3;

// timer
unsigned long time;
unsigned long seconds, minutes, hours;
extern volatile unsigned long timer0_millis;
unsigned long new_value = 0;

void setup(void) {
  // code by the library
  uint16_t tmp;
  tft.begin(9600);
  tft.reset();
  identifier = tft.readID();
  Serial.begin(9600);
  ts = TouchScreen(XP, YP, XM, YM, 300);  //call the constructor AGAIN with new values.
  tft.begin(identifier);
  tft.fillScreen(BLACK);
}

void drawBullet() {
  // right wall
  if (320 <= bulletPosX + bulletSize) speedX *= -1;
  // top wall
  if (480 <= bulletPosY + bulletSize) speedY *= -1;
  // left wall
  if (0 >= bulletPosX) speedX *= -1;
  // bottom wall
  if (0 >= bulletPosY) speedY *= -1;
  // move the bullet
  tft.fillRect(bulletPosX, bulletPosY, bulletSize, bulletSize, BLACK);
  bulletPosX += speedX;
  bulletPosY += speedY;
  tft.fillRect(bulletPosX, bulletPosY, bulletSize, bulletSize, WHITE);
  // change speed
  if (seconds != 0 && speedX < 20 && seconds % 10 == 0) speedX++;
  if (seconds != 0 && speedX > -20 && seconds % 10 == 0) speedX++;
  Serial.print("speedX: ");
  Serial.println(speedX);
  if (seconds != 0 && speedY < 20 && seconds % 10 == 0) speedY++;
  if (seconds != 0 && speedY > -20 && seconds % 10 == 0) speedY++;
  Serial.print("speedY: ");
  Serial.println(speedY);
}

void loop() {
  tft.drawRect(0, 0, 320, 480, WHITE);
  getPos();
  drawPlayer(xpos, ypos);
  drawBullet();
  // is player not touching the border
  if (xpos > 1 && xpos < 304 && ypos > 1 && ypos < 464) {
    drawTimer();
    // is player touching the bullet
    if (inBullet(xpos, ypos, bulletPosX, bulletPosY, bulletSize)) {
      loose(seconds, minutes, hours);
    }
  } else {
    loose(seconds, minutes, hours);
  }
}

void getPos() {
  // Serial.print("Pressure: ");
  // Serial.println(tp.z);
  tp = ts.getPoint(); 
  pinMode(XM, OUTPUT);
  pinMode(YP, OUTPUT);
  pinMode(XP, OUTPUT);
  pinMode(YM, OUTPUT);

  if (tp.z > MINPRESSURE && tp.z < MAXPRESSURE) {
    xpos = map(tp.x, TS_LEFT, TS_RT, 0, tft.width());
    ypos = map(tp.y, TS_TOP, TS_BOT, 0, tft.height());
  }
}

void drawPlayer(int xpos, int ypos) {
  // the Player
  tft.fillRect(oldXpos, oldYpos, 15, 15, BLACK);
  oldXpos = xpos;
  oldYpos = ypos;
  tft.fillRect(xpos, ypos, 15, 15, RED);
}

bool inBullet(int xpos, int ypos, int bulletPosX, int bulletPosY, int bulletSize) {
  bool rightTo = xpos >= bulletPosX - bulletSize / 2;
  // if (rightTo) Serial.println("rightTo");
  bool under = ypos >= bulletPosY - bulletSize / 2;
  // if (under) Serial.println("under");
  bool leftTo = xpos <= bulletPosX + bulletSize;
  // if (leftTo) Serial.println("leftTo");
  bool above = ypos <= bulletPosY + bulletSize;
  // if (above) Serial.println("above");
  if (rightTo && under && leftTo && above) {
    return true;
  }
  return false;
}

void loose(int seconds, int minutes, int hours) {
  delay(700);
  tft.fillScreen(BLACK);
  tft.fillRect(100, 240, 120, 50, RED);
  xpos = 153;
  ypos = 400;
  while (1) {
    tft.setCursor(30, 30);
    tft.setTextSize(2);
    tft.println("You loose :(");
    tft.setCursor(30, 60);
    tft.println("Your Time: ");
    tft.setCursor(30, 90);
    if (String(hours).length() == 1) tft.print("0");
    tft.print(hours);
    tft.print(":");
    if (String(minutes).length() == 1) tft.print("0");
    tft.print(minutes);
    tft.print(":");
    if (String(seconds).length() == 1) tft.print("0");
    tft.println(seconds);
    tft.fillRect(100, 240, 120, 50, RED);
    tft.setCursor(112, 257);
    tft.println("Restart?");
    getPos();
    drawPlayer(xpos, ypos);
    if ((xpos > 95 && xpos < 225) && (ypos > 235 && ypos < 285)) {
      tft.fillScreen(BLACK);
      bulletPosX = 4;
      bulletPosY = 4;
      speedX = 5;
      speedY = 5;
      Serial.println(seconds);
      delay(500);
      setMillis(1000);  // reset timer
      return;
    }
  }
}

void drawTimer() {
  tft.setCursor(10, 10);
  tft.setTextSize(2);
  time = millis();
  seconds = time / 1000;
  minutes = seconds / 60;
  hours = minutes / 60;
  time %= 1000;
  int oldSecond = seconds;
  seconds %= 60;
  if (oldSecond == seconds) tft.fillRect(10, 10, 100, 15, BLACK);
  int oldMinutes = minutes;
  minutes %= 60;
  if (oldMinutes == minutes) tft.fillRect(10, 10, 100, 15, BLACK);
  int oldHours = hours;
  hours %= 60;
  if (oldHours == hours) tft.fillRect(10, 10, 100, 15, BLACK);
  if (String(hours).length() == 1) tft.print("0");
  tft.print(hours);
  tft.print(":");
  if (String(minutes).length() == 1) tft.print("0");
  tft.print(minutes);
  tft.print(":");
  if (String(seconds - 1).length() == 1) tft.print("0");
  tft.println(seconds - 1);
}

void setMillis(unsigned long new_millis) {
  uint8_t oldSREG = SREG;
  cli();
  timer0_millis = new_millis;
  SREG = oldSREG;
}

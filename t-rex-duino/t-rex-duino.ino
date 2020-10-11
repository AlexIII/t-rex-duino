/*
 * Project name: T-rex-duino
 * Description: T-rex game from Chrome browser rewritten for Arduino
 * Project page: https://github.com/AlexIII/t-rex-duino
 * Author: github.com/AlexIII
 * E-mail: endoftheworld@bk.ru
 * License: MIT
 * -------Hardware------
 * Board: Arduino Uno / Nano / Pro / pro mini
 * Display: OLED SSD1309 SPI 128x64  *OR*  SH1106/SSD1306 I2C 128x64 (130x64) (132x64)
*/ 

/* Hardware Connections */
// -- Buttons --
#define JUMP_BUTTON 6
#define DUCK_BUTTON 5

// -- Display Selection (uncomment ONE of the options) -- 
#define LCD_SSD1309
//#define LCD_SH1106      //If you see abnormal vertical line at the left edge of the display, select LCD_SSD1306
//#define LCD_SSD1306     //If you see abnormal vertical line at the right edge of the display, select LCD_SH1106

// -- Display Connection for SSD1309 --
#define LCD_SSD1309_CS 2
#define LCD_SSD1309_DC 3
#define LCD_SSD1309_RESET 4
//LCD_SSD1309_SDA -> 11 (SPI SCK)
//LCD_SSD1309_SCL -> 13 (SPI MOSI)

// -- Display Connection for SH1106/SSD1306 --
//LCD_SH1106_SDA -> A4 (I2C SDA)
//LCD_SH1106_SCL -> A5 (I2C SCL)

/* Misc. Settings */
//#define AUTO_PLAY //uncomment to enable auto-play mode
//#define RESET_HI_SCORE //uncomment to reset HI score, flash your device, than comment it back and flash again
//#define PRINT_DEBUG_INFO

/* Game Balance Settings */
#define PLAYER_SAFE_ZONE_WIDTH 32 //minimum distance between obstacles (px)
#define CACTI_RESPAWN_RATE 50 //lower -> more frequent, max 255
#define GROUND_CACTI_SCROLL_SPEED 3 //pixels per game cycle
#define PTERODACTY_SPEED 5 //pixels per game cycle
#define PTERODACTY_RESPAWN_RATE 255 //lower -> more frequent, max 255
#define INCREASE_FPS_EVERY_N_SCORE_POINTS 256 //better to be power of 2
#define LIVES_START 3
#define LIVES_MAX 5
#define SPAWN_NEW_LIVE_MIN_CYCLES 800
#define DAY_NIGHT_SWITCH_CYCLES 1024 //better to be power of 2
#define TARGET_FPS_START 23
#define TARGET_FPS_MAX 48 //gradually increase FPS to that value to make the game faster and harder

/* Display Settings */
#define LCD_HEIGHT 64U
#define LCD_WIDTH 128U

/* Render Settings */
#ifdef LCD_SSD1309
  //#define VIRTUAL_HEIGHT_BUFFER_ROWS_BY_8_PIXELS 1
  #define VIRTUAL_WIDTH_BUFFER_COLS 16
#else
  //VIRTUAL_HEIGHT_BUFFER_ROWS_BY_8_PIXELS is not supported
  #define VIRTUAL_HEIGHT_BUFFER_ROWS_BY_8_PIXELS 4
#endif

/* Includes */
#include <EEPROM.h>
#include "array.h"
#include "TrexPlayer.h"
#include "Ground.h"
#include "Cactus.h"
#include "Pterodactyl.h"
#include "HeartLive.h"

/* Defines and globals */
#define EEPROM_HI_SCORE 16 //2 bytes
#define LCD_BYTE_SZIE (LCD_WIDTH*LCD_HEIGHT/8)

#ifdef VIRTUAL_WIDTH_BUFFER_COLS
  #define LCD_IF_VIRTUAL_WIDTH(TRUE_COND, FALSE_COND) TRUE_COND
  #define LCD_PART_BUFF_WIDTH VIRTUAL_WIDTH_BUFFER_COLS
  #define LCD_PART_BUFF_HEIGHT LCD_HEIGHT
#else
  #define LCD_IF_VIRTUAL_WIDTH(TRUE_COND, FALSE_COND) FALSE_COND
  #ifdef VIRTUAL_HEIGHT_BUFFER_ROWS_BY_8_PIXELS
    #define LCD_PART_BUFF_WIDTH LCD_WIDTH
    #define LCD_PART_BUFF_HEIGHT (VIRTUAL_HEIGHT_BUFFER_ROWS_BY_8_PIXELS*8)
  #else
    #define LCD_PART_BUFF_WIDTH LCD_WIDTH
    #define LCD_PART_BUFF_HEIGHT LCD_HEIGHT
  #endif
#endif
#define LCD_PART_BUFF_SZ ((LCD_PART_BUFF_HEIGHT/8)*LCD_PART_BUFF_WIDTH)

#ifdef LCD_SSD1309
  #include <SPI.h>
  #include "SSD1309.h"
  static SSD1309<SPIClass> lcd(SPI, LCD_SSD1309_CS, LCD_SSD1309_DC, LCD_SSD1309_RESET, LCD_BYTE_SZIE);
#else
  #include "I2C.h"
  #include "SH1106.h"
  I2C i2c;
  SH1106<I2C> lcd(i2c, LCD_BYTE_SZIE);
#endif

static uint16_t hiScore = 0;
static bool firstStart = true;

/* Misc Functions */

bool isPressedJump() {
  return !digitalRead(JUMP_BUTTON);
}

bool isPressedDuck() {
  return !digitalRead(DUCK_BUTTON);
}

uint8_t randByte() {
  static uint16_t c = 0xA7E2;
  c = (c << 1) | (c >> 15);
  c = (c << 1) | (c >> 15);
  c = (c << 1) | (c >> 15);
  c = analogRead(A2) ^ analogRead(A3) ^ analogRead(A4) ^ analogRead(A5) ^ analogRead(A6) ^ analogRead(A7) ^ c;
  return c;
}

/* Main Functions */

void renderNumber(BitCanvas& canvas, Point2Di8 point, const uint16_t number) {
  uint16_t base = 10000;
  while(base) {
    const uint8_t digit = (number/base)%10;
    canvas.render(numbers.getSprite(digit, point));
    base /= 10;
    point.x += numbers.getWidth() + 1;
  }
}

void gameLoop(uint16_t &hiScore) {
  uint8_t lcdBuff[LCD_PART_BUFF_SZ];
  VirtualBitCanvas bitCanvas(
    LCD_IF_VIRTUAL_WIDTH(VirtualBitCanvas::VIRTUAL_WIDTH, VirtualBitCanvas::VIRTUAL_HEIGHT), 
    lcdBuff, 
    LCD_PART_BUFF_HEIGHT,
    LCD_PART_BUFF_WIDTH,
    LCD_IF_VIRTUAL_WIDTH(LCD_WIDTH, LCD_HEIGHT) //virtual size in selected direction
  );

  SpawnHold spawnHolder;

  //dinamic sprites
  TrexPlayer trex;
  Ground ground1(-1);
  Ground ground2(63);
  Ground ground3(127);
  Cactus cactus1(spawnHolder);
  Cactus cactus2(spawnHolder);
  Pterodactyl pterodactyl1(spawnHolder);
  HeartLive heartLive;
  const array<SpriteAnimated*, 8> sprites{{&ground1, &ground2, &ground3, &cactus1, &cactus2, &pterodactyl1, &heartLive, &trex}};
  const array<SpriteAnimated*, 3> enemies{{&cactus1, &cactus2, &pterodactyl1}};

  //static sprites
  const Sprite gameOverSprite(&game_overver_bm, {15, 12});
  const Sprite restartIconSprite(&restart_icon_bm, {55, 25});
  const Sprite hiSprite(&hi_score, {44, 0});
  Sprite heartsSprite(&hearts_5x_bm, {95, 8});

  //game variables
  uint32_t prvT = 0;
  bool gameOver = false;
  uint16_t score = 0;
  uint8_t targetFPS = TARGET_FPS_START;
  uint8_t lives = LIVES_START;
  bool night = false;
  lcd.setInverse(night);

  //main cycle
  while(1) {
    //render cycle
    while(1) {
      //score
      bitCanvas.render(hiSprite);
      renderNumber(bitCanvas, {60, 0}, hiScore);
      renderNumber(bitCanvas, {95, 0}, score);
      bitCanvas.render(heartsSprite);
      //game objects
      for(uint8_t i = 0; i < sprites.size(); ++i)
        bitCanvas.render(*sprites[i]);
      //game over
      if(gameOver) {
        bitCanvas.render(gameOverSprite);
        bitCanvas.render(restartIconSprite);
      }
      //update screen
      lcd.fillScreen(lcdBuff, LCD_PART_BUFF_SZ, LCD_IF_VIRTUAL_WIDTH(LCD_PART_BUFF_WIDTH, 0));
      if(bitCanvas.nextPart()) break;
    }

    //exit game on game over
    if(gameOver) {
      if(score > hiScore) hiScore = score;
      return;
    }

    //collision detection
    if(!trex.isBlinking() && CollisionDetector::check(trex, enemies.data, enemies.size())) {
      if(lives) {
        trex.blink();
        --lives;
      } else {
        trex.die();
        gameOver = true;
        continue;
      }
    }
    if(lives < LIVES_MAX && CollisionDetector::check(trex, heartLive)) {
      ++lives;
      heartLive.eat();
    }

#ifndef AUTO_PLAY
    //constrols
    if(isPressedJump()) trex.jump();
    trex.duck(isPressedDuck());
#else
    const int8_t trexXright = trex.bitmap->width + trex.position.x;
    //auto jump
    if(
      (cactus1.position.x <= trexXright + 5 && cactus1.position.x > trexXright) || 
      (cactus2.position.x <= trexXright + 5 && cactus2.position.x > trexXright) || 
      (pterodactyl1.position.y > 30 && pterodactyl1.position.x <= trexXright + 5 && pterodactyl1.position.x > trexXright)
    ) trex.jump();
    //auto duck
    trex.duck(
      (pterodactyl1.position.y <= 30 && pterodactyl1.position.y > 20 && pterodactyl1.position.x <= trexXright + 15 && pterodactyl1.position.x > trex.position.x)
    );
#endif

    //logic and animation step
    for(uint8_t i = 0; i < sprites.size(); ++i)
      sprites[i]->step();
    //score keeping
    if(score < 0xFFFE) ++score;
    //make game progressively faster
    if(!(score%INCREASE_FPS_EVERY_N_SCORE_POINTS) && targetFPS < TARGET_FPS_MAX) ++targetFPS;
    heartsSprite.limitRenderWidthTo = 6*lives + 1;
    //switch day and night
    if(!(score%DAY_NIGHT_SWITCH_CYCLES)) lcd.setInverse(night = !night);

    const uint8_t frameTime = 1000/targetFPS;
#ifdef PRINT_DEBUG_INFO
    //print CPU load statistics
    const uint32_t dt = millis() - prvT;
    uint32_t left = frameTime > dt? frameTime - dt : 0;
    Serial.print("CPU: ");
    Serial.print(100 - 100*left / frameTime);
    Serial.print("% ");
    Serial.println(dt);
#endif

    //throttle
    while(millis() - prvT < frameTime);
    prvT = millis();
  } 
}

void spalshScreen() {
  lcd.setAddressingMode(lcd.HorizontalAddressingMode);
  uint8_t buff[32];
  for(uint8_t i = 0; i < LCD_BYTE_SZIE/sizeof(buff); ++i) {
    memcpy_P(buff, splash_screen_bitmap + 2 + uint16_t(i) * sizeof(buff), sizeof(buff));
    lcd.fillScreen(buff, sizeof(buff));
  }
  for(uint8_t i = 50; i && !isPressedJump(); --i) delay(100);
}

void setup() {
  pinMode(JUMP_BUTTON, INPUT_PULLUP);
  pinMode(DUCK_BUTTON, INPUT_PULLUP);
  Serial.begin(250000);
  lcd.begin();
  spalshScreen();
  lcd.setAddressingMode(LCD_IF_VIRTUAL_WIDTH(lcd.VerticalAddressingMode, lcd.HorizontalAddressingMode));
  srand((randByte()<<8) | randByte());
#ifdef RESET_HI_SCORE
  EEPROM.put(EEPROM_HI_SCORE, hiScore);
#endif
  EEPROM.get(EEPROM_HI_SCORE, hiScore);
  if(hiScore == 0xFFFF) hiScore = 0;
}

void loop() {
  if(firstStart || isPressedJump()) {
    firstStart = false;
    gameLoop(hiScore);
    EEPROM.put(EEPROM_HI_SCORE, hiScore);
    //wait until the jump button is released
    while(isPressedJump()) delay(100);
    delay(500);
  }
}

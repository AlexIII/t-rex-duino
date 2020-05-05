/*
 * Project name: T-rex-duino
 * Description: T-rex game from Chrome brower rewritten for Arduino
 * Project page: https://github.com/AlexIII/t-rex-duino
 * Author: github.com/AlexIII
 * E-mail: endoftheworld@bk.ru
 * License: MIT
*/ 

#ifndef _TREX_PLAYER_H_
#define _TREX_PLAYER_H_

#include "assets.h"
#include "engine.h"

#define T_REX_JUMP_MOMENTUM 8 //jump height = 6 -> 21px, 7 -> 28px, 8 -> 36px
#define T_REX_JUMP_MOMENTUM_DUCKED 6
#define T_REX_START_POINT {5, 60}

static const BitmapMasked* const trex_sprites[] = {&trex_up_1, &trex_up_2, &trex_up_3, &trex_duck_1, &trex_duck_2, &trex_dead_1, &trex_dead_2};
#define T_REX_SPRITE_UP_START 0
#define T_REX_SPRITE_UP_END 3
#define T_REX_SPRITE_DUCK_START 3
#define T_REX_SPRITE_DUCK_END 5
#define T_REX_SPRITE_DEAD_UP 5
#define T_REX_SPRITE_DEAD_DUCK 6

struct TrexPlayer : SpriteAnimated {
  enum State : uint8_t {
    UP,
    DUCK,
    DEAD
  };
  State state = UP;
  TrexPlayer() : SpriteAnimated(trex_sprites[0], T_REX_START_POINT, ANCHOR_BOTTOM_LEFT) {}

  virtual void step() override {
    animationStep();
    motionStep();
  }

  void jump() {
      if(isJumping() || state == DEAD) return;
      vy = state == UP? T_REX_JUMP_MOMENTUM : T_REX_JUMP_MOMENTUM_DUCKED;
  }
  void duck(const bool toDuck) {
    if(toDuck) {
      if(state == UP && !isJumping()) state = DUCK;
    } else {
      if(state == DUCK) state = UP;
    }
  }
  void die() {
    bitmap = trex_sprites[state == DUCK? T_REX_SPRITE_DEAD_DUCK : T_REX_SPRITE_DEAD_UP];
    state = DEAD;
    vy = 0;
  }
  void blink() { blinkCnt = PLAYER_SAFE_ZONE_WIDTH; }
  bool isBlinking() { return blinkCnt; }

protected:
  //motion
  int8_t dy = 0;
  int8_t vy = 0;
  bool skipStep = false;
  bool isJumping() const { return dy != 0 || vy != 0; }
  void motionStep() {
    //dirty fix to prolong the jump
    if(abs(vy) <= 1 && !skipStep) {
      skipStep = true;
      return;
    }
    skipStep = false;
    
    dy += vy;
    position.y -= vy;
    if(dy) --vy;
    else vy = 0;
  }

  //animation
  uint8_t bitmapId = 0;
  uint8_t blinkCnt = 0;
  void animationStep() {
    if(blinkCnt) --blinkCnt;
    if(blinkCnt & 1) {
      bitmap = 0;
      return;
    }
    
    uint8_t start = 0, end = 0;

    if(state == UP)
      start = T_REX_SPRITE_UP_START, end = isJumping()? T_REX_SPRITE_UP_START : T_REX_SPRITE_UP_END;
    else if(state == DUCK)
      start = T_REX_SPRITE_DUCK_START, end = isJumping()? T_REX_SPRITE_DUCK_START : T_REX_SPRITE_DUCK_END;
    else if(state == DEAD)
      start = T_REX_SPRITE_DEAD_UP, end = T_REX_SPRITE_DEAD_UP;

    if(!(bitmapId >= start && bitmapId < end)) bitmapId = start;
    if(bitmapId + 1 < end) ++bitmapId;
    else bitmapId = start;
    
    bitmap = trex_sprites[bitmapId];
  }
};

#endif

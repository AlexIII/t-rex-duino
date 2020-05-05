/*
 * Project name: T-rex-duino
 * Description: T-rex game from Chrome brower rewritten for Arduino
 * Project page: https://github.com/AlexIII/t-rex-duino
 * Author: github.com/AlexIII
 * E-mail: endoftheworld@bk.ru
 * License: MIT
*/ 

#ifndef _CACTUS_H_
#define _CACTUS_H_

#include "assets.h"
#include "engine.h"

#define CACTUS_POSITION_Y 61

static const BitmapMasked* const cacti_sprites[] = {
  &cacti_sb, &cacti_sb,
  &cacti_bs, &cacti_bs,
  &cacti_2b, &cacti_2b,
  &cacti_3s, &cacti_3s,
};
static const uint8_t cacti_width[] = { 
  10, cacti_sb.width,
  14, cacti_bs.width,
  14, cacti_2b.width,
  18, cacti_3s.width
};

struct Cactus : SpireScrollingToLeft {
  Cactus(SpawnHold &spawnHolder) : SpireScrollingToLeft(cacti_sprites[0], GROUND_CACTI_SCROLL_SPEED, CACTUS_POSITION_Y, ANCHOR_BOTTOM_LEFT), spawnHolder(spawnHolder) { /*position.x = 59;*/ }

  virtual void step() override {
    SpireScrollingToLeft::step();
    if(!isActive()) {
      if(respawnWait) {
        --respawnWait;
        return;
      }
      
      if(!spawnHolder.tryAcquire(this, PLAYER_SAFE_ZONE_WIDTH))
        return;
            
      const uint16_t r = rand();
      const uint8_t i = r&7;
      bitmap = cacti_sprites[i];
      limitRenderWidthTo = cacti_width[i];
      respawnWait = scaleValue(r, CACTI_RESPAWN_RATE);
      rearm();
    }
  }

private:
  uint8_t respawnWait = 0;
  SpawnHold &spawnHolder;
};

#endif

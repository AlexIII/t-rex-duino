/*
 * Project name: T-rex-duino
 * Description: T-rex game from Chrome brower rewritten for Arduino
 * Project page: https://github.com/AlexIII/t-rex-duino
 * Author: github.com/AlexIII
 * E-mail: endoftheworld@bk.ru
 * License: MIT
*/ 

#ifndef _GROUND_H_
#define _GROUND_H_

#include "assets.h"
#include "engine.h"

#define GROUND_POSITION_Y 64
static const BitmapMasked* const ground_sprites[] = {&ground_1, &ground_2, &ground_3, &ground_4, &ground_5, &ground_1, &ground_2, &ground_3};

struct Ground : SpireScrollingToLeft {
  Ground(const int8_t startX) : SpireScrollingToLeft(ground_sprites[0], GROUND_CACTI_SCROLL_SPEED, GROUND_POSITION_Y, ANCHOR_BOTTOM_LEFT) { position.x = startX; }

  virtual void step() override {
    for(uint8_t i = 0; i < speed; ++i) {
      --position.x;
      if(!isActive()) {
        bitmap = ground_sprites[rand()&7];
        rearm();
      }
    }
  }
};

#endif

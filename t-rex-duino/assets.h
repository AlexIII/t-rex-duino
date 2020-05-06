/*
 * Project name: T-rex-duino
 * Description: T-rex game from Chrome brower rewritten for Arduino
 * Project page: https://github.com/AlexIII/t-rex-duino
 * Author: github.com/AlexIII
 * E-mail: endoftheworld@bk.ru
 * License: MIT
*/ 

#ifndef _ASSETS_H_
#define _ASSETS_H_

#include "engine.h"

/* T-rex */
#include "assets/trex-up-1.h"
#include "assets/trex-up-2.h"
#include "assets/trex-up-3.h"
#include "assets/trex-duck-1.h"
#include "assets/trex-duck-2.h"
#include "assets/trex-dead-1-no-outline.h"
#include "assets/trex-dead-2-no-outline.h"
static const BitmapMasked trex_up_1(trex_up_1s_bitmap, trex_up_1s_mask);
static const BitmapMasked trex_up_2(trex_up_2s_bitmap, trex_up_2s_mask);
static const BitmapMasked trex_up_3(trex_up_3s_bitmap, trex_up_3s_mask);
static const BitmapMasked trex_duck_1(trex_duck_1s_bitmap, trex_duck_1s_mask);
static const BitmapMasked trex_duck_2(trex_duck_2s_bitmap, trex_duck_2s_mask);
static const BitmapMasked trex_dead_1(trex_dead_1s_no_outline_bitmap, trex_dead_1s_no_outline_mask);
static const BitmapMasked trex_dead_2(trex_dead_2s_no_outline_bitmap, trex_dead_2s_no_outline_mask);

/* Ground */
#include "assets/ground-1.h"
#include "assets/ground-2.h"
#include "assets/ground-3.h"
#include "assets/ground-4.h"
#include "assets/ground-5.h"
static const BitmapMasked ground_1(ground_1_bitmap, 0);
static const BitmapMasked ground_2(ground_2_bitmap, 0);
static const BitmapMasked ground_3(ground_3_bitmap, 0);
static const BitmapMasked ground_4(ground_4_bitmap, 0);
static const BitmapMasked ground_5(ground_5_bitmap, 0);

/* Cacti */
#include "assets/cacti-big-big.h"
#include "assets/cacti-big-small.h"
#include "assets/cacti-small-big.h"
#include "assets/cacti-small-small-small.h"
static const BitmapMasked cacti_2b(cacti_big_big_bitmap, cacti_big_big_mask);
static const BitmapMasked cacti_bs(cacti_big_small_bitmap, cacti_big_small_mask);
static const BitmapMasked cacti_sb(cacti_small_big_bitmap, cacti_small_big_mask);
static const BitmapMasked cacti_3s(cacti_small_small_small_bitmap, cacti_small_small_small_mask);

/* Pterodactyl */
#include "assets/pterodactyl-1.h"
#include "assets/pterodactyl-2.h"
static const BitmapMasked pterodactyl_1(pterodactyl_1_bitmap, pterodactyl_1_mask);
static const BitmapMasked pterodactyl_2(pterodactyl_2_bitmap, pterodactyl_2_mask);

/* Font */
const uint8_t font5x8_digits_data[] PROGMEM = {
  0x3E, 0x51, 0x49, 0x45, 0x3E,// 0 0x30
  0x00, 0x42, 0x7F, 0x40, 0x00,// 1 0x31
  0x42, 0x61, 0x51, 0x49, 0x46,// 2 0x32
  0x21, 0x41, 0x45, 0x4B, 0x31,// 3 0x33
  0x18, 0x14, 0x12, 0x7F, 0x10,// 4 0x34
  0x27, 0x45, 0x45, 0x45, 0x39,// 5 0x35
  0x3C, 0x4A, 0x49, 0x49, 0x30,// 6 0x36
  0x01, 0x71, 0x09, 0x05, 0x03,// 7 0x37
  0x36, 0x49, 0x49, 0x49, 0x36,// 8 0x38
  0x06, 0x49, 0x49, 0x29, 0x1E // 9 0x39  
};
static Symbol numbers(font5x8_digits_data, 5, 8, 10);

/* Text and icons */
#include "assets/splash-screen.h"
#include "assets/game-over.h"
static const BitmapMasked game_overver_bm(game_over_bitmap, game_over_mask);
#include "assets/restart-icon.h"
static const BitmapMasked restart_icon_bm(restart_icon_bitmap, restart_icon_mask);
#include "assets/hearts-5x.h"
static const BitmapMasked hearts_5x_bm(hearts_5x_bitmap, 0);

const uint8_t hi_score_bitmap[] PROGMEM = {
  11, //width
  8,  //height
  0x7F, 0x08, 0x08, 0x08, 0x7F,// H
  0x00,
  0x00, 0x41, 0x7F, 0x41, 0x00,// I
};
static const BitmapMasked hi_score(hi_score_bitmap, 0);

#endif

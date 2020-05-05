/*
 * Project name: T-rex-duino
 * Description: T-rex game from Chrome brower rewritten for Arduino
 * Project page: https://github.com/AlexIII/t-rex-duino
 * Author: github.com/AlexIII
 * E-mail: endoftheworld@bk.ru
 * License: MIT
*/ 

#ifndef _ENGINE_TYPES_H_
#define _ENGINE_TYPES_H_
#include <stdint.h>

/* General */

struct flash_uint8_t {
private:
  flash_uint8_t();
  uint8_t data;
};
static inline uint8_t flashByte(const flash_uint8_t* ptr, const uint16_t offset) {
  return pgm_read_byte(ptr + offset);
}

/* Geometry */

template<typename T>
struct Point2D {
    T x;
    T y;
};

//using Point2Du8 = Point2D<uint8_t>;
using Point2Di8 = Point2D<int8_t>;

/* Bitmaps and sprites */

struct BitmapMasked {
  uint8_t width;
  uint8_t height;
  const flash_uint8_t* data;
  const flash_uint8_t* mask;
  BitmapMasked(const uint8_t* const flashBitmap, const uint8_t* const flashMask, const uint8_t width, const uint8_t height) :
    width(width), height(height), data((const flash_uint8_t*)flashBitmap), mask((const flash_uint8_t*)flashMask) {}
  BitmapMasked(const uint8_t* const flashBitmap, const uint8_t* const flashMask) : 
    BitmapMasked((const flash_uint8_t*)flashBitmap, (const flash_uint8_t*)flashMask) {}
  BitmapMasked(const flash_uint8_t* const flashBitmap, const flash_uint8_t* const flashMask) :
    width(flashByte(flashBitmap, 0)), height(flashByte(flashBitmap, 1)), data(flashBitmap+2), mask(flashMask? flashMask+2 : 0) {}
};

struct Sprite {
  const BitmapMasked* bitmap = 0;
  Point2Di8 position{};
  enum AnchorCorner : uint8_t {
    ANCHOR_TOP_LEFT,
    ANCHOR_BOTTOM_LEFT,
  };
  AnchorCorner anchor;
  uint8_t limitRenderWidthTo = 0xFF;
  Sprite() {}
  Sprite(const BitmapMasked* const bitmap, const Point2Di8 &position, const AnchorCorner anchor = ANCHOR_TOP_LEFT) :
    bitmap(bitmap), position(position), anchor(anchor) {}
};

struct SpriteAnimated : Sprite {
  virtual void step() = 0;
protected:
  SpriteAnimated() {}
  SpriteAnimated(const BitmapMasked* const bitmap, const Point2Di8 &position, const AnchorCorner anchor = ANCHOR_TOP_LEFT) : Sprite(bitmap, position, anchor) {}
};

struct SpireScrollingToLeft : SpriteAnimated {
  SpireScrollingToLeft(const BitmapMasked* const bitmap, const uint8_t speed, const int8_t posinionY, const AnchorCorner anchor = ANCHOR_TOP_LEFT, const int8_t resetPosinionX = 127) : 
    SpriteAnimated(bitmap, {-127, posinionY}, anchor), speed(speed), resetPosinionX(resetPosinionX) {}

  virtual void step() override {
    for(uint8_t i = 0; isActive() && i < speed; ++i)
      --position.x;
  }
  bool isActive() const { return position.x > -(int8_t)bitmap->width;}
  void rearm() { position.x = resetPosinionX; }
protected:
  const uint8_t speed;
  const int8_t resetPosinionX;
};

struct Symbol {
  Symbol(const uint8_t* const flashBitmap, const uint8_t width, const uint8_t height, const uint8_t nSymbols) :
    cur(flashBitmap, 0, width, height), sprite(&cur, {0, 0}), bitmapBase((const flash_uint8_t*)flashBitmap), nSymbols(nSymbols) {}
  const Sprite& getSprite(uint8_t n, const Point2Di8 &position) {
    if(n >= nSymbols) n = 0;
    cur.data = bitmapBase + n*cur.width*((cur.height+7)/8);
    sprite.position = position;
    return sprite;
  }
  uint8_t getWidth() const { return cur.width; }
private:
  BitmapMasked cur;
  Sprite sprite;
  const flash_uint8_t* bitmapBase;
  uint8_t nSymbols;
};

/* Canvas */
struct BitCanvas {
  uint8_t* const bitmap;
  const uint8_t height;
  const uint8_t width;
  uint8_t xOffset = 0; //minus
  uint8_t yOffset = 0; //minus

  BitCanvas(uint8_t* const bitmapBuffer, const uint8_t height, const uint8_t width) :
    bitmap(bitmapBuffer), height(height), width(width) { clear(); }

  void clear(const bool v = false) {
    memset(bitmap, v? 0xFF : 0, width * height/8);
  }

  void render(const Sprite &sprite) {
    if(!sprite.bitmap) return;
    
    //sprite rectangle
    const int16_t sX0 = (int16_t)sprite.position.x - xOffset, sX1 = sX0 + min(sprite.bitmap->width, sprite.limitRenderWidthTo);
    const int16_t sY0 = (int16_t)sprite.position.y - (sprite.anchor == Sprite::ANCHOR_BOTTOM_LEFT? sprite.bitmap->height : 0) - yOffset, sY1 = sY0 + sprite.bitmap->height;
    if(sX0 >= width || sX1 <= 0 || sY0 >= height || sY1 <= 0) return;

    //cycle on canvas
    const uint8_t Xto = min(sX1, width);
    const uint8_t Yto = min(sY1, height);

    const uint8_t bitOffset = (sY0%8) + (sY0%8 < 0? 8 : 0);
    const uint8_t bitmapByteHeight = (sprite.bitmap->height + 7) / 8;
    
    for(uint8_t y = max(sY0, 0); y < 8*((Yto + 7)/8); y += 8) {
      const uint8_t yByte = y/8;
      const uint8_t sy = y - sY0;
      const uint8_t syByte = (sy + 7)/8; //allowed to be equal to bitmapByteHeight

      uint8_t* const bitmap_yw = bitmap + yByte*width;
      
      const flash_uint8_t* const sMask_yw =   //curren row mask, maybe null
        sprite.bitmap->mask && syByte < bitmapByteHeight  //mask exists and syByte is not out of array bounds
          ? sprite.bitmap->mask + syByte*sprite.bitmap->width 
          : 0;
      const flash_uint8_t* const sMask_yw_1 =   //previous row mask, maybe null
        sprite.bitmap->mask && syByte && (syByte-1) < bitmapByteHeight  //mask exists and syByte>0 and syByte-1 is not out of array bounds
          ? sprite.bitmap->mask + (syByte-1)*sprite.bitmap->width 
          : 0;
      const flash_uint8_t* const sData_yw =  //curren row data, maybe null
        syByte < bitmapByteHeight //syByte is not out of array bounds
          ? sprite.bitmap->data + syByte*sprite.bitmap->width
          : 0;
      const flash_uint8_t* const sData_yw_1 =  //previous row data, maybe null
        syByte && (syByte-1) < bitmapByteHeight //syByte>0 and syByte-1 is not out of array bounds
          ? sprite.bitmap->data + (syByte-1)*sprite.bitmap->width
          : 0;

      for(uint8_t x = max(sX0, 0); x < Xto; ++x) {
        //render 8 pixels
        const uint8_t sx = x - sX0;
        uint8_t& canvasByte = bitmap_yw[x];
        
        //apply mask
        if(sMask_yw_1) canvasByte &= ~(flashByte(sMask_yw_1, sx) >> (8-bitOffset));
        if(sMask_yw) canvasByte &= ~(flashByte(sMask_yw, sx) << bitOffset);
        //apply sprite bitmap data
        if(sData_yw_1) canvasByte |= flashByte(sData_yw_1, sx) >> (8-bitOffset);
        if(sData_yw) canvasByte |= flashByte(sData_yw, sx) << bitOffset;
      }
    }
  }
};

struct VirtualBitCanvas : BitCanvas {
  enum Mode : uint8_t {
    VIRTUAL_HEIGHT,
    VIRTUAL_WIDTH
  };
  VirtualBitCanvas(const Mode mode, uint8_t* const bitmapBuffer, const uint8_t bufferHeight, const uint8_t bufferWidth, const uint8_t virtualSize) :
    BitCanvas(bitmapBuffer, bufferHeight, bufferWidth), 
    virtualSize(virtualSize), 
    bufferSize(mode == VIRTUAL_HEIGHT? bufferHeight : bufferWidth), 
    offsetCoord(mode == VIRTUAL_HEIGHT? yOffset : xOffset) {}

  //returns true on wrap
  bool nextPart() {
    clear();
    if(offsetCoord + bufferSize >= virtualSize) {
      offsetCoord = 0;
      return true;
    }
    offsetCoord += bufferSize;
    return false;
  }
  
private:
  const uint8_t virtualSize;
  const uint8_t bufferSize;
  uint8_t &offsetCoord;
};

struct CollisionDetector {
  template<class DerivedFromSprite>
  static bool check(const Sprite &s, const DerivedFromSprite* const* ss, uint8_t size) {
    while(size--)
      if(check(s, **ss++)) return true;
    return false;
  }
    
  static bool check(const Sprite &s1, const Sprite &s2) {
    if(!s1.bitmap || !s2.bitmap) return false;
    
    const uint8_t width = s1.bitmap->width;
    const uint8_t height = s1.bitmap->height;
    //sprite 2 rectangle in sprite 1 frame
    const int16_t sX0 = (int16_t)s2.position.x - s1.position.x, sX1 = sX0 + min(s2.bitmap->width, s2.limitRenderWidthTo);
    const int16_t sY0 = (int16_t)s2.position.y - (s2.anchor == Sprite::ANCHOR_BOTTOM_LEFT? s2.bitmap->height : 0) - s1.position.y + (s1.anchor == Sprite::ANCHOR_BOTTOM_LEFT? s1.bitmap->height : 0), sY1 = sY0 + s2.bitmap->height;  
    if(sX0 >= width || sX1 <= 0 || sY0 >= height || sY1 <= 0) return false; //no rectangle intersection
    
    //cycle on sprite 1
    const uint8_t Xto = min(sX1, width);
    const uint8_t Yto = min(sY1, height);
    
    for(uint8_t y = max(sY0, 0); y < Yto; ++y) {
      const uint8_t yByte = y/8;
      const uint8_t yBit = y%8;
      const uint8_t sy = y - sY0;
      const uint8_t syByte = sy/8;
      const uint8_t syBit = sy%8;

      const flash_uint8_t* const s1data = s1.bitmap->data + yByte*width;
      const flash_uint8_t* const s2data = s2.bitmap->data + syByte*s2.bitmap->width;
      
      for(uint8_t x = max(sX0, 0); x < Xto; ++x) {
        const uint8_t sx = x - sX0;
        if((flashByte(s1data, x) & (1<<yBit)) && (flashByte(s2data, sx) & (1<<syBit))) return true;
      }
    }
    
    return false;
  }
private:
  CollisionDetector();
};

struct SpawnHold {
  bool tryAcquire(const void* me, const uint8_t countBeforeSpawn) {
    //Counter is free, acquire it and wait
    if(owner == 0) {
      counter = countBeforeSpawn;
      owner = me;
      return false;
    }
    //we're owning this counter, decrement or free it
    if(owner == me) {
      if(counter) {
        --counter;
        return false;
      } else {
        owner = 0;
        return true;
      }
    }
    return false; //someone else is owning the counter
  }
private:
  const void* owner = 0;
  uint8_t counter = 0;
};

static uint8_t scaleValue(const uint8_t val, const uint8_t limit) {
  return (uint16_t(val)*limit) >> 8;
}

#endif

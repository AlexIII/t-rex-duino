/*
 * Project name: T-rex-duino
 * Description: T-rex game from Chrome brower rewritten for Arduino
 * Project page: https://github.com/AlexIII/t-rex-duino
 * Author: github.com/AlexIII
 * E-mail: endoftheworld@bk.ru
 * License: MIT
*/ 

#ifndef _I2C_CALSS_H_
#define _I2C_CALSS_H_

#include <compat/twi.h>

struct I2C {
  I2C() {}
  
  static void init(const uint32_t clock) {
    TWSR = 0;                         // no prescaler
    TWBR = ((F_CPU/clock)-16)/2;  // must be > 10 for stable operation
  }
  
  static void deinit(void) {
    TWBR = 0;
    TWCR = 0;
  }

  static uint8_t start(uint8_t address) {
    uint8_t   twst, i;
    // send START condition
    TWCR = (1<<TWINT) | (1<<TWSTA) | (1<<TWEN);
  
    // wait until transmission completed
    //while(!(TWCR & (1<<TWINT)));
    for(i = 255; i; --i) {
      if(TWCR & (1<<TWINT)) break;
      asm volatile("nop");
      asm volatile("nop");
    }
    if(!i) return 0xFF;
  
    // check value of TWI Status Register. Mask prescaler bits.
    twst = TW_STATUS & 0xF8;
    if ( (twst != TW_START) && (twst != TW_REP_START)) return 1;
  
    // send device address
    TWDR = address;
    TWCR = (1<<TWINT) | (1<<TWEN);
  
    // wail until transmission completed and ACK/NACK has been received
    //while(!(TWCR & (1<<TWINT)));
    for(i = 255; i; --i) {
      if(TWCR & (1<<TWINT)) break;
      asm volatile("nop");
      asm volatile("nop");
    }
    if(!i) return 0xFF;
    
    // check value of TWI Status Register. Mask prescaler bits.
    twst = TW_STATUS & 0xF8;
    if ( (twst != TW_MT_SLA_ACK) && (twst != TW_MR_SLA_ACK) ) return 1;
  
    return 0;
  }

  static void stop(void) {
    //send stop condition
    TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWSTO);
    
    // wait until stop condition is executed and bus released
    //while(TWCR & (1<<TWSTO));
    for(uint8_t i = 255; i; --i) {
      if(!(TWCR & (1<<TWSTO))) break;
      asm volatile("nop");
      asm volatile("nop");
    }
  }
  
  static uint8_t write(uint8_t data) {  
    uint8_t   twst;
      
    // send data to the previously addressed device
    TWDR = data;
    TWCR = (1<<TWINT) | (1<<TWEN);
  
    // wait until transmission completed
    //while(!(TWCR & (1<<TWINT)));
    uint8_t i;
    for(i = 255; i; --i) {
      if(TWCR & (1<<TWINT)) break;
      asm volatile("nop");
      asm volatile("nop");
    }
    if(!i) return 0xFF;
    
    // check value of TWI Status Register. Mask prescaler bits
    twst = TW_STATUS & 0xF8;
    if( twst != TW_MT_DATA_ACK) return twst;
    return 0;
  }

  static uint8_t readAck(void) {
    TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWEA);
    //while(!(TWCR & (1<<TWINT)));
    for(uint8_t i = 255; i; --i) {
      if(TWCR & (1<<TWINT)) break;
      asm volatile("nop");
      asm volatile("nop");
    }
    return TWDR;
  }
  
  static uint8_t readNak(void) {
    TWCR = (1<<TWINT) | (1<<TWEN);
    //while(!(TWCR & (1<<TWINT)));
    for(uint8_t i = 255; i; --i) {
      if(TWCR & (1<<TWINT)) break;
      asm volatile("nop");
      asm volatile("nop");
    }
    return TWDR;
  }
};


#endif

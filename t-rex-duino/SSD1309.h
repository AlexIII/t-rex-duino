/*
 * Project name: T-rex-duino
 * Description: T-rex game from Chrome brower rewritten for Arduino
 * Project page: https://github.com/AlexIII/t-rex-duino
 * Author: github.com/AlexIII
 * E-mail: endoftheworld@bk.ru
 * License: MIT
*/ 

#ifndef _SSD1309_H_
#define _SSD1309_H_

template<class SPI_TYPE>
class SSD1309 {
  SPI_TYPE &spi;
  const uint8_t cs;
  const uint8_t dc;
  const uint8_t res;
  const uint16_t screenBufferSize;
public:
  enum AddressingMode {
    HorizontalAddressingMode = 0x00,
    VerticalAddressingMode = 0x01,
    PageAddressingMode = 0x02,
  };
  SSD1309(SPI_TYPE &spi, const uint8_t cs, const uint8_t dc, const uint8_t res, const uint16_t screenBufferSize): 
    spi(spi), cs(cs), dc(dc), res(res), screenBufferSize(screenBufferSize) {}
  void begin() {
    pinMode(cs, OUTPUT);
    digitalWrite(cs, HIGH);
    pinMode(dc, OUTPUT);
    pinMode(res, OUTPUT);
    digitalWrite(res, LOW);
    delay(50);
    digitalWrite(res, HIGH);
    spi.begin();
    init();
  }
  void fillScreen(const uint8_t* buffer) {
    sendd(buffer, screenBufferSize);
  }
  void fillScreen(const uint8_t* buffer, const uint16_t size, const uint8_t stride = 0) {
    sendd(buffer, size, stride);
  }
  void setInverse(const bool v) {
    sendc(v? 0xA7 : 0xA6);
  }
  void setAddressingMode(const AddressingMode addressingMode) {
    sendc(0x20, addressingMode);
  }
private:
  /* lcd control */
  void init() {
    sendc(0xAF);   //Set Display ON
    //sendc(0xA5, 1); //Entire Display ON
    //sendc(0xA7);  //Inverse Mode

    //sendc(0x20, addressingMode); //Set Addressing Mode
  }
  /* send bytes */
  void sendc(const uint8_t c1) {
    sendBytes(&c1, 1, false);
  }
  void sendc(const uint8_t c1, const uint8_t c2) { 
    const uint8_t c[] = {c1, c2}; 
    sendBytes(c, sizeof(c), false);
  }
  void sendc(const uint8_t c1, const uint8_t c2, const uint8_t c3) { 
    const uint8_t c[] = {c1, c2, c3}; 
    sendBytes(c, sizeof(c), false);
  }
  void sendd(const uint8_t* d, uint16_t sz, const uint8_t stride = 0) {
    sendBytes(d, sz, true, stride);
  }
  void sendBytes(const uint8_t* d, uint16_t sz, const bool data, const uint8_t stride = 0) {
    digitalWrite(dc, data);
    digitalWrite(cs, LOW);
    if(!stride)
      while(sz--) spi.transfer(*d++);
    else
      for(uint8_t i = 0; i < stride; ++i)
        for(uint16_t j = i; j < sz; j += stride)
          spi.transfer(d[j]);
    digitalWrite(cs, HIGH);
  }
};

#endif

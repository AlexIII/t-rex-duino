/*
 * Project name: T-rex-duino
 * Description: T-rex game from Chrome brower rewritten for Arduino
 * Project page: https://github.com/AlexIII/t-rex-duino
 * Author: github.com/AlexIII
 * E-mail: endoftheworld@bk.ru
 * License: MIT
*/ 

#ifndef ARRAY_H_
#define ARRAY_H_

template <typename T, uint8_t SIZE>
struct array {
  T data[SIZE];
  constexpr uint8_t size() const {return SIZE;}
  constexpr operator T*() {return data;}
  constexpr operator const T*() const {return data;}
  template <typename IndexType>
  constexpr const T& operator [](const IndexType idx) const { return data[idx]; }
};

#endif /* ARRAY_H_ */

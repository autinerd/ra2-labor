#ifndef RGB_H
#define RGB_H

#include <stdint.h>

typedef struct rgbw {
  uint32_t red:8;
  uint32_t green:8;
  uint32_t blue:8;
  uint32_t white:8;
} rgbw;

typedef struct hsv {
    // hue = 0..191
  uint8_t hue;
  // sat = 0..255
  uint8_t sat;
  // val = 0..255
  uint8_t val;
} hsv;

#endif

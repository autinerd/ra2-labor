#ifndef COLOR_CONV_H
#define COLOR_CONV_H
#include "rgb.h"

#define HSV_SECTION_3 (0x40)
#define HSV_HUE_RANGE (HSV_SECTION_3 * 3)

rgbw hsv2rgb(hsv hsv);

#endif

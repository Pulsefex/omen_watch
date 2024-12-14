#ifndef __SSD1306_FONTS_H__
#define __SSD1306_FONTS_H__

#include "oled_logo.h"

#ifdef SSD1306_INCLUDE_FONT_6x8
extern const SSD1306_Font_tx Font_6x8;
#endif
#ifdef SSD1306_INCLUDE_FONT_7x10
extern const SSD1306_Font_tx Font_7x10x;
#endif
#ifdef SSD1306_INCLUDE_FONT_11x18
extern const SSD1306_Font_tx Font_11x18x;
#endif
#ifdef SSD1306_INCLUDE_FONT_16x26
extern const SSD1306_Font_tx Font_16x26x;
#endif
#ifdef SSD1306_INCLUDE_FONT_16x24
extern const SSD1306_Font_tx Font_16x24;
#endif
#ifdef SSD1306_INCLUDE_FONT_16x15
/** Generated Roboto Thin 15 
 * @copyright Google https://github.com/googlefonts/roboto
 * @license This font is licensed under the Apache License, Version 2.0.
*/
extern const SSD1306_Font_tx Font_16x15;
#endif

#endif // __SSD1306_FONTS_H__

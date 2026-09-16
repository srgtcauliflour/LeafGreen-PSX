#pragma once
#include <stdint.h>
typedef struct { uint16_t x,y,w,h; } LGRect;
typedef struct { uint8_t glyph_width, glyph_height, spacing; } LGFontMetrics;
void lg_text_init(void);
void lg_text_draw(const char *ascii, int x, int y);

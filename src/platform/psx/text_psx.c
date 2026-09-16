#include "lg/platform.h"
#include "lg/text.h"
#include <psxgpu.h>
#ifdef LGPSX_FONT_DEMO
#include "font_data.h"

static bool draw_glyph(void *context, uint8_t glyph, int x, int y, uint8_t width) {
    POLY_FT4 quad;
    (void)context;
    if (x < 0 || y < 0 || x > 320 - width || y > 240 - 14) return false;
    if (glyph == 0) return true;
    setPolyFT4(&quad);
    setShadeTex(&quad, 1);
    setXYWH(&quad, x, y, width, 14);
    setUVWH(&quad, (glyph % 16) * 16, (glyph / 16) * 16, width, 14);
    setTPage(&quad, 0, 0, 640, 0);
    setClut(&quad, 640, 256);
    /* DrawPrim writes the packet synchronously; no stack packet is queued. */
    DrawPrim((const uint32_t *)&quad);
    return true;
}
#endif

void lg_platform_text_init(void) {
#ifdef LGPSX_FONT_DEMO
    RECT pixels = {640, 0, 64, 256};
    RECT palette = {640, 256, 16, 1};
    LoadImage(&pixels, lg_font_pixels);
    DrawSync(0);
    LoadImage(&palette, (const uint32_t *)lg_font_palette);
    DrawSync(0);
#endif
}

void lg_platform_draw_text_demo(void) {
#ifdef LGPSX_FONT_DEMO
    static const char text[] = "LeafGreen-PSX\nNative text from verified ROM.\nABCDEFGHIJKLMNOPQRSTUVWXYZ\nabcdefghijklmnopqrstuvwxyz\n0123456789 !?.-";
    uint8_t encoded[160];
    /* Finish any queued SDK HUD drawing before issuing immediate primitives. */
    DrawSync(0);
    size_t size = lg_text_encode_ascii(text, encoded, sizeof encoded);
    if (size) (void)lg_text_layout(encoded, size, lg_font_widths, 16, 144, draw_glyph, 0);
#endif
}

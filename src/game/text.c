#include "lg/text.h"
#include <limits.h>

LGTextResult lg_text_layout(const uint8_t *text, size_t size,
                           const uint8_t widths[256], int x, int y,
                           LGGlyphSink sink, void *context) {
    int start = x;
    if (!text || !widths || !sink) return LG_TEXT_INVALID;
    for (size_t i = 0; i < size; ++i) {
        uint8_t glyph = text[i];
        if (glyph == 0xff) return LG_TEXT_END;
        if (glyph == 0xfe) {
            if (y > INT_MAX - 16) return LG_TEXT_INVALID;
            y += 16;
            x = start;
            continue;
        }
        if (glyph >= 0xf7) return LG_TEXT_UNSUPPORTED;
        uint8_t width = widths[glyph];
        if (width == 0 || width > 16 || x > INT_MAX - width)
            return LG_TEXT_INVALID;
        if (!sink(context, glyph, x, y, width)) return LG_TEXT_FULL;
        x += width;
    }
    return LG_TEXT_TRUNCATED;
}

size_t lg_text_encode_ascii(const char *text, uint8_t *out, size_t capacity) {
    size_t n = 0;
    if (!text || !out || !capacity) return 0;
    for (; *text; ++text) {
        uint8_t glyph;
        if (n + 1 >= capacity) return 0;
        if (*text >= 'A' && *text <= 'Z') glyph = (uint8_t)(0xbb + *text - 'A');
        else if (*text >= 'a' && *text <= 'z') glyph = (uint8_t)(0xd5 + *text - 'a');
        else if (*text >= '0' && *text <= '9') glyph = (uint8_t)(0xa1 + *text - '0');
        else switch (*text) {
            case ' ': glyph = 0; break;
            case '\n': glyph = 0xfe; break;
            case '!': glyph = 0xab; break;
            case '?': glyph = 0xac; break;
            case '.': glyph = 0xad; break;
            case '-': glyph = 0xae; break;
            default: return 0;
        }
        out[n++] = glyph;
    }
    out[n++] = 0xff;
    return n;
}

#include "lg/text.h"
#include <assert.h>
#include <limits.h>
#include <string.h>

typedef struct { unsigned count; int x[4], y[4]; } Seen;
static bool capture(void *context, uint8_t glyph, int x, int y, uint8_t width) {
    Seen *s = context;
    (void)glyph; (void)width;
    if (s->count == 4) return false;
    s->x[s->count] = x; s->y[s->count] = y; ++s->count;
    return true;
}
int main(void) {
    uint8_t widths[256], encoded[32];
    Seen seen = {0};
    memset(widths, 6, sizeof widths);
    widths[0xbc] = 8;
    size_t n = lg_text_encode_ascii("AB\nC", encoded, sizeof encoded);
    assert(n == 5 && encoded[0] == 0xbb && encoded[1] == 0xbc);
    assert(lg_text_layout(encoded, n, widths, 10, 20, capture, &seen) == LG_TEXT_END);
    assert(seen.count == 3 && seen.x[1] == 16 && seen.x[2] == 10 && seen.y[2] == 36);
    assert(lg_text_encode_ascii("AB", encoded, 2) == 0);
    assert(lg_text_encode_ascii("@", encoded, sizeof encoded) == 0);
    const uint8_t truncated[] = {0xbb};
    seen.count = 0;
    assert(lg_text_layout(truncated, 1, widths, 0, 0, capture, &seen) == LG_TEXT_TRUNCATED);
    for (unsigned c = 0xf7; c <= 0xfd; ++c) {
        uint8_t control[] = {(uint8_t)c};
        assert(lg_text_layout(control, 1, widths, 0, 0, capture, &seen) == LG_TEXT_UNSUPPORTED);
    }
    assert(lg_text_layout(truncated, 1, widths, INT_MAX, 0, capture, &seen) == LG_TEXT_INVALID);
    const uint8_t newline[] = {0xfe};
    assert(lg_text_layout(newline, 1, widths, 0, INT_MAX, capture, &seen) == LG_TEXT_INVALID);
    seen.count = 4;
    assert(lg_text_layout(truncated, 1, widths, 0, 0, capture, &seen) == LG_TEXT_FULL);
    widths[0xbb] = 17;
    assert(lg_text_layout(truncated, 1, widths, 0, 0, capture, &seen) == LG_TEXT_INVALID);
    return 0;
}

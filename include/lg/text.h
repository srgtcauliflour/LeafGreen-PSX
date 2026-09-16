#ifndef LG_TEXT_H
#define LG_TEXT_H
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef bool (*LGGlyphSink)(void *context, uint8_t glyph, int x, int y, uint8_t width);
typedef enum { LG_TEXT_END, LG_TEXT_TRUNCATED, LG_TEXT_UNSUPPORTED,
               LG_TEXT_FULL, LG_TEXT_INVALID } LGTextResult;
/* Bounded subset: ordinary Latin glyph bytes, FE newline, FF terminator.
   F7-FD require game services and are rejected without reading operands. */
LGTextResult lg_text_layout(const uint8_t *text, size_t size,
                           const uint8_t widths[256], int x, int y,
                           LGGlyphSink sink, void *context);
/* ASCII adapter for original diagnostic strings; zero means unsupported. */
size_t lg_text_encode_ascii(const char *text, uint8_t *out, size_t capacity);
#endif

#include "lg/dialogue.h"
#include <limits.h>

void lg_dialogue_init(LGDialogueState *d, const uint8_t *text, size_t size,
                       const uint8_t widths[256], int x, int y,
                       LGGlyphSink sink, void *sink_context) {
    if (!d) return;
    d->text = text;
    d->size = size;
    d->pos = 0;
    d->widths = widths;
    d->x = x;
    d->y = y;
    d->start_x = x;
    d->sink = sink;
    d->sink_context = sink_context;
    d->control = 0;
    d->status = (text && widths && sink) ? LG_DIALOGUE_RUNNING : LG_DIALOGUE_ERROR;
}

LGDialogueStatus lg_dialogue_step(LGDialogueState *d) {
    if (!d) return LG_DIALOGUE_ERROR;
    if (d->status == LG_DIALOGUE_DONE || d->status == LG_DIALOGUE_ERROR ||
        d->status == LG_DIALOGUE_TRUNCATED || d->status == LG_DIALOGUE_WAIT)
        return d->status;
    while (d->pos < d->size) {
        uint8_t glyph = d->text[d->pos];
        if (glyph == 0xff) {
            ++d->pos;
            d->status = LG_DIALOGUE_DONE;
            return d->status;
        }
        if (glyph == 0xfe) {
            if (d->y > INT_MAX - 16) {
                d->status = LG_DIALOGUE_ERROR;
                return d->status;
            }
            d->y += 16;
            d->x = d->start_x;
            ++d->pos;
            continue;
        }
        if (glyph >= 0xf7) {
            d->control = glyph;
            d->status = LG_DIALOGUE_WAIT;
            return d->status;
        }
        uint8_t width = d->widths[glyph];
        if (width == 0 || width > 16 || d->x > INT_MAX - width) {
            d->status = LG_DIALOGUE_ERROR;
            return d->status;
        }
        if (!d->sink(d->sink_context, glyph, d->x, d->y, width)) {
            d->status = LG_DIALOGUE_FULL;
            return d->status;
        }
        d->x += width;
        ++d->pos;
    }
    d->status = LG_DIALOGUE_TRUNCATED;
    return d->status;
}

void lg_dialogue_resume(LGDialogueState *d) {
    if (!d || d->status != LG_DIALOGUE_WAIT) return;
    ++d->pos;
    d->control = 0;
    d->status = LG_DIALOGUE_RUNNING;
}

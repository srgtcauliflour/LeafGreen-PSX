#include "lg/window.h"

static LGWindowStatus map_status(const LGWindowState *w, LGDialogueStatus status) {
    switch (status) {
        case LG_DIALOGUE_RUNNING:   return LG_WINDOW_RUNNING;
        case LG_DIALOGUE_WAIT:      return LG_WINDOW_AWAIT_ADVANCE;
        case LG_DIALOGUE_DONE:      return LG_WINDOW_DONE;
        case LG_DIALOGUE_TRUNCATED: return LG_WINDOW_TRUNCATED;
        case LG_DIALOGUE_FULL:      return w->awaiting_scroll ? LG_WINDOW_AWAIT_SCROLL
                                                                : LG_WINDOW_FULL;
        default:                    return LG_WINDOW_ERROR;
    }
}

static bool window_sink(void *context, uint8_t glyph, int x, int y, uint8_t width) {
    LGWindowState *w = context;
    if (!w->have_last_y || y != w->last_y) {
        if (w->max_lines > 0 && w->lines_seen + 1 > w->max_lines) {
            w->awaiting_scroll = true;
            return false;
        }
        w->last_y = y;
        w->have_last_y = true;
        ++w->lines_seen;
    }
    return w->sink(w->sink_context, glyph, x, y, width);
}

void lg_window_init(LGWindowState *w, const uint8_t *text, size_t size,
                     const uint8_t widths[256], int x, int y, int wrap_width,
                     int max_lines, LGGlyphSink sink, void *sink_context) {
    if (!w) return;
    w->start_y = y;
    w->max_lines = max_lines > 0 ? max_lines : 0;
    w->lines_seen = 0;
    w->have_last_y = false;
    w->awaiting_scroll = false;
    w->sink = sink;
    w->sink_context = sink_context;
    lg_dialogue_init(&w->dialogue, text, size, widths, x, y, wrap_width,
                      window_sink, w);
    if (!sink) w->dialogue.status = LG_DIALOGUE_ERROR;
}

LGWindowStatus lg_window_step(LGWindowState *w, bool advance_pressed) {
    if (!w) return LG_WINDOW_ERROR;
    if (w->awaiting_scroll) {
        if (!advance_pressed) return LG_WINDOW_AWAIT_SCROLL;
        w->dialogue.y = w->start_y;
        w->dialogue.status = LG_DIALOGUE_RUNNING;
        w->lines_seen = 0;
        w->have_last_y = false;
        w->awaiting_scroll = false;
    }
    LGDialogueStatus status = lg_dialogue_step(&w->dialogue);
    if (status == LG_DIALOGUE_WAIT && advance_pressed) {
        lg_dialogue_resume(&w->dialogue);
        status = lg_dialogue_step(&w->dialogue);
    }
    return map_status(w, status);
}

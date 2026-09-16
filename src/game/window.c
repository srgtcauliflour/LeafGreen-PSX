#include "lg/window.h"

static LGWindowStatus map_status(LGDialogueStatus status) {
    switch (status) {
        case LG_DIALOGUE_RUNNING:   return LG_WINDOW_RUNNING;
        case LG_DIALOGUE_WAIT:      return LG_WINDOW_AWAIT_ADVANCE;
        case LG_DIALOGUE_DONE:      return LG_WINDOW_DONE;
        case LG_DIALOGUE_TRUNCATED: return LG_WINDOW_TRUNCATED;
        case LG_DIALOGUE_FULL:      return LG_WINDOW_FULL;
        default:                    return LG_WINDOW_ERROR;
    }
}

void lg_window_init(LGWindowState *w, const uint8_t *text, size_t size,
                     const uint8_t widths[256], int x, int y, int wrap_width,
                     LGGlyphSink sink, void *sink_context) {
    if (!w) return;
    lg_dialogue_init(&w->dialogue, text, size, widths, x, y, wrap_width,
                      sink, sink_context);
}

LGWindowStatus lg_window_step(LGWindowState *w, bool advance_pressed) {
    if (!w) return LG_WINDOW_ERROR;
    LGDialogueStatus status = lg_dialogue_step(&w->dialogue);
    if (status == LG_DIALOGUE_WAIT && advance_pressed) {
        lg_dialogue_resume(&w->dialogue);
        status = lg_dialogue_step(&w->dialogue);
    }
    return map_status(status);
}

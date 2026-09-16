#ifndef LG_WINDOW_H
#define LG_WINDOW_H
#include "lg/dialogue.h"

/* Gates a dialogue's control-byte pauses behind an explicit per-frame
   "advance" signal (e.g. a confirmed button press), so a window renderer
   can show a waiting-for-input indicator instead of silently skipping the
   control. Also bounds the box to a fixed number of lines: once a glyph
   would start one line beyond that bound, it withholds that glyph and
   reports LG_WINDOW_AWAIT_SCROLL until advance_pressed, then resets to the
   box's first line and continues drawing from there. This is the portable
   piece LGPSX-011 needs on top of LGDialogueState; it lays out glyphs but
   does not draw a window box itself. */
typedef enum { LG_WINDOW_RUNNING, LG_WINDOW_AWAIT_ADVANCE, LG_WINDOW_AWAIT_SCROLL,
               LG_WINDOW_DONE, LG_WINDOW_TRUNCATED, LG_WINDOW_FULL, LG_WINDOW_ERROR
} LGWindowStatus;

typedef struct {
    LGDialogueState dialogue;
    int start_y;
    int max_lines;    /* 0 disables scroll gating */
    int lines_seen;   /* distinct line-start y values drawn since the last scroll */
    int last_y;
    bool have_last_y;
    bool awaiting_scroll;
    LGGlyphSink sink;
    void *sink_context;
} LGWindowState;

/* wrap_width has the same character-wrapping meaning as in LGDialogueState.
   max_lines bounds how many lines are drawn before pausing for a scroll.
   Pass 0 to either to disable that bound. */
void lg_window_init(LGWindowState *w, const uint8_t *text, size_t size,
                     const uint8_t widths[256], int x, int y, int wrap_width,
                     int max_lines, LGGlyphSink sink, void *sink_context);
/* Advances layout by one call. On a control byte, reports
   LG_WINDOW_AWAIT_ADVANCE on every call until advance_pressed is true, at
   which point it consumes the control byte and continues in the same call.
   On hitting the max_lines bound, reports LG_WINDOW_AWAIT_SCROLL the same
   way, then on advance_pressed resets to the box's first line and
   continues in the same call. */
LGWindowStatus lg_window_step(LGWindowState *w, bool advance_pressed);
#endif

#ifndef LG_WINDOW_H
#define LG_WINDOW_H
#include "lg/dialogue.h"

/* Gates a dialogue's control-byte pauses behind an explicit per-frame
   "advance" signal (e.g. a confirmed button press), so a window renderer
   can show a waiting-for-input indicator instead of silently skipping the
   control. This is the portable piece LGPSX-011 needs on top of
   LGDialogueState; it lays out glyphs but does not draw a window itself. */
typedef enum { LG_WINDOW_RUNNING, LG_WINDOW_AWAIT_ADVANCE, LG_WINDOW_DONE,
               LG_WINDOW_TRUNCATED, LG_WINDOW_FULL, LG_WINDOW_ERROR
} LGWindowStatus;

typedef struct {
    LGDialogueState dialogue;
} LGWindowState;

void lg_window_init(LGWindowState *w, const uint8_t *text, size_t size,
                     const uint8_t widths[256], int x, int y,
                     LGGlyphSink sink, void *sink_context);
/* Advances layout by one call. When a control byte is hit, reports
   LG_WINDOW_AWAIT_ADVANCE on every call until advance_pressed is true, at
   which point it consumes the control byte and continues in the same
   call (so a caller polling once per frame naturally sees the indicator
   for as many frames as the control stays unacknowledged). */
LGWindowStatus lg_window_step(LGWindowState *w, bool advance_pressed);
#endif

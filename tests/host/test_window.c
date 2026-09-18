#include "lg/window.h"
#include <assert.h>
#include <string.h>

typedef struct { unsigned count; int x[8], y[8]; } Seen;
static bool capture(void *context, uint8_t glyph, int x, int y, uint8_t width) {
    Seen *s = context;
    (void)glyph; (void)width;
    if (s->count == 8) return false;
    s->x[s->count] = x;
    s->y[s->count] = y;
    ++s->count;
    return true;
}

int main(void) {
    uint8_t widths[256];
    memset(widths, 6, sizeof widths);
    Seen seen = {0};
    LGWindowState w;

    /* "Hi" (0xbb,0xbc), a control byte, then "!" (0xab), then end. */
    const uint8_t text[] = {0xbb, 0xbc, 0xf9, 0xab, 0xff};
    lg_window_init(&w, text, sizeof text, widths, 0, 0, 0, 0, 0, capture, &seen);
    assert(lg_window_step(&w, false) == LG_WINDOW_AWAIT_ADVANCE);
    assert(seen.count == 2);
    /* Polling without an advance signal keeps waiting and draws nothing new. */
    assert(lg_window_step(&w, false) == LG_WINDOW_AWAIT_ADVANCE);
    assert(lg_window_step(&w, false) == LG_WINDOW_AWAIT_ADVANCE);
    assert(seen.count == 2);
    /* An advance signal consumes the control and resumes within the same call. */
    assert(lg_window_step(&w, true) == LG_WINDOW_DONE);
    assert(seen.count == 3);

    /* A string with no control bytes runs straight through. */
    seen.count = 0;
    const uint8_t plain[] = {0xbb, 0xff};
    lg_window_init(&w, plain, sizeof plain, widths, 0, 0, 0, 0, 0, capture, &seen);
    assert(lg_window_step(&w, false) == LG_WINDOW_DONE);
    assert(seen.count == 1);

    /* Missing terminator truncates, matching the underlying dialogue state. */
    const uint8_t truncated[] = {0xbb};
    lg_window_init(&w, truncated, sizeof truncated, widths, 0, 0, 0, 0, 0, capture, &seen);
    assert(lg_window_step(&w, false) == LG_WINDOW_TRUNCATED);

    /* wrap_width forwards through to the underlying dialogue layout. */
    seen.count = 0;
    const uint8_t abc[] = {0xbb, 0xbc, 0xbd, 0xff};
    lg_window_init(&w, abc, sizeof abc, widths, 0, 0, 12, 0, 0, capture, &seen);
    assert(lg_window_step(&w, false) == LG_WINDOW_DONE);
    assert(seen.count == 3);

    /* max_lines withholds a glyph that would start a line beyond the bound
       and reports AWAIT_SCROLL until advance_pressed, then resets to the
       box's first line and continues in the same call. */
    seen.count = 0;
    const uint8_t twolines[] = {0xbb, 0xbc, 0xfe, 0xbd, 0xbe, 0xff}; /* AB\nCD */
    lg_window_init(&w, twolines, sizeof twolines, widths, 0, 0, 0, 1, 0, capture, &seen);
    assert(lg_window_step(&w, false) == LG_WINDOW_AWAIT_SCROLL);
    assert(seen.count == 2 && seen.y[0] == 0 && seen.y[1] == 0);
    /* Polling without a scroll signal keeps waiting and draws nothing new. */
    assert(lg_window_step(&w, false) == LG_WINDOW_AWAIT_SCROLL);
    assert(seen.count == 2);
    assert(lg_window_step(&w, true) == LG_WINDOW_DONE);
    assert(seen.count == 4);
    assert(seen.x[2] == 0 && seen.y[2] == 0);
    assert(seen.x[3] == 6 && seen.y[3] == 0);

    /* max_lines == 0 disables scroll gating (the earlier cases already
       exercise multi-line text without it triggering). */

    /* reveal_per_step paces drawing to that many glyphs per call (a
       typewriter effect): LG_WINDOW_RUNNING while paced glyphs remain,
       resuming from where it paused on the next call, with no signal
       needed from the caller (unlike AWAIT_ADVANCE/AWAIT_SCROLL). */
    seen.count = 0;
    const uint8_t five[] = {0xbb, 0xbc, 0xbd, 0xbe, 0xbf, 0xff}; /* five glyphs */
    lg_window_init(&w, five, sizeof five, widths, 0, 0, 0, 0, 2, capture, &seen);
    assert(lg_window_step(&w, false) == LG_WINDOW_RUNNING);
    assert(seen.count == 2);
    assert(lg_window_step(&w, false) == LG_WINDOW_RUNNING);
    assert(seen.count == 4);
    /* The fifth glyph and the terminator land in the same call: nothing
       left to pause on after it. */
    assert(lg_window_step(&w, false) == LG_WINDOW_DONE);
    assert(seen.count == 5);

    /* Invalid setup is rejected up front and on every subsequent call. */
    lg_window_init(&w, 0, 0, widths, 0, 0, 0, 0, 0, capture, &seen);
    assert(lg_window_step(&w, false) == LG_WINDOW_ERROR);
    assert(lg_window_step(0, false) == LG_WINDOW_ERROR);

    return 0;
}

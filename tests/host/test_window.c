#include "lg/window.h"
#include <assert.h>
#include <string.h>

typedef struct { unsigned count; } Seen;
static bool capture(void *context, uint8_t glyph, int x, int y, uint8_t width) {
    (void)glyph; (void)x; (void)y; (void)width;
    ((Seen *)context)->count++;
    return true;
}

int main(void) {
    uint8_t widths[256];
    memset(widths, 6, sizeof widths);
    Seen seen = {0};
    LGWindowState w;

    /* "Hi" (0xbb,0xbc), a control byte, then "!" (0xab), then end. */
    const uint8_t text[] = {0xbb, 0xbc, 0xf9, 0xab, 0xff};
    lg_window_init(&w, text, sizeof text, widths, 0, 0, 0, capture, &seen);
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
    lg_window_init(&w, plain, sizeof plain, widths, 0, 0, 0, capture, &seen);
    assert(lg_window_step(&w, false) == LG_WINDOW_DONE);
    assert(seen.count == 1);

    /* Missing terminator truncates, matching the underlying dialogue state. */
    const uint8_t truncated[] = {0xbb};
    lg_window_init(&w, truncated, sizeof truncated, widths, 0, 0, 0, capture, &seen);
    assert(lg_window_step(&w, false) == LG_WINDOW_TRUNCATED);

    /* wrap_width forwards through to the underlying dialogue layout. */
    seen.count = 0;
    const uint8_t abc[] = {0xbb, 0xbc, 0xbd, 0xff};
    lg_window_init(&w, abc, sizeof abc, widths, 0, 0, 12, capture, &seen);
    assert(lg_window_step(&w, false) == LG_WINDOW_DONE);
    assert(seen.count == 3);

    /* Invalid setup is rejected up front and on every subsequent call. */
    lg_window_init(&w, 0, 0, widths, 0, 0, 0, capture, &seen);
    assert(lg_window_step(&w, false) == LG_WINDOW_ERROR);
    assert(lg_window_step(0, false) == LG_WINDOW_ERROR);

    return 0;
}

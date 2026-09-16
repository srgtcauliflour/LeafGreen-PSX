#include "lg/dialogue.h"
#include <assert.h>
#include <limits.h>
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
    LGDialogueState d;
    Seen seen = {0};

    /* "Hi" (0xbb,0xbc) then a wait control (0xf9), then "!" (0xab), then end. */
    const uint8_t text[] = {0xbb, 0xbc, 0xf9, 0xab, 0xff};
    lg_dialogue_init(&d, text, sizeof text, widths, 10, 20, capture, &seen);
    assert(lg_dialogue_step(&d) == LG_DIALOGUE_WAIT);
    assert(d.control == 0xf9);
    assert(seen.count == 2 && seen.x[0] == 10 && seen.x[1] == 16);
    /* Re-stepping without resuming re-reports the same wait. */
    assert(lg_dialogue_step(&d) == LG_DIALOGUE_WAIT);
    assert(seen.count == 2);
    lg_dialogue_resume(&d);
    assert(lg_dialogue_step(&d) == LG_DIALOGUE_DONE);
    assert(seen.count == 3 && seen.x[2] == 22);

    /* Newline resets x and advances y, independent of control handling. */
    seen.count = 0;
    const uint8_t multiline[] = {0xbb, 0xfe, 0xbc, 0xff};
    lg_dialogue_init(&d, multiline, sizeof multiline, widths, 5, 0, capture, &seen);
    assert(lg_dialogue_step(&d) == LG_DIALOGUE_DONE);
    assert(seen.count == 2 && seen.x[1] == 5 && seen.y[1] == 16);

    /* Missing terminator truncates without erroring. */
    const uint8_t truncated[] = {0xbb};
    lg_dialogue_init(&d, truncated, sizeof truncated, widths, 0, 0, capture, &seen);
    seen.count = 0;
    assert(lg_dialogue_step(&d) == LG_DIALOGUE_TRUNCATED);
    assert(lg_dialogue_step(&d) == LG_DIALOGUE_TRUNCATED);

    /* A full sink can be retried once the caller has drained it. */
    const uint8_t two[] = {0xbb, 0xbc, 0xff};
    lg_dialogue_init(&d, two, sizeof two, widths, 0, 0, capture, &seen);
    seen.count = 8;
    assert(lg_dialogue_step(&d) == LG_DIALOGUE_FULL);
    seen.count = 0;
    assert(lg_dialogue_step(&d) == LG_DIALOGUE_DONE);
    assert(seen.count == 2);

    /* Overflowing width/position is an error, matching lg_text_layout. */
    widths[0xbb] = 17;
    lg_dialogue_init(&d, two, sizeof two, widths, 0, 0, capture, &seen);
    assert(lg_dialogue_step(&d) == LG_DIALOGUE_ERROR);
    widths[0xbb] = 6;
    const uint8_t nl[] = {0xfe};
    lg_dialogue_init(&d, nl, sizeof nl, widths, 0, INT_MAX, capture, &seen);
    assert(lg_dialogue_step(&d) == LG_DIALOGUE_ERROR);

    /* Missing arguments are rejected up front. */
    lg_dialogue_init(&d, 0, 0, widths, 0, 0, capture, &seen);
    assert(d.status == LG_DIALOGUE_ERROR);
    assert(lg_dialogue_step(&d) == LG_DIALOGUE_ERROR);
    assert(lg_dialogue_step(0) == LG_DIALOGUE_ERROR);
    lg_dialogue_resume(0);

    return 0;
}

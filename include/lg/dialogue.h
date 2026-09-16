#ifndef LG_DIALOGUE_H
#define LG_DIALOGUE_H
#include "lg/text.h"

/* Portable, resumable dialogue layout. Lays out glyphs like lg_text_layout
   but can pause mid-string: on a control byte (0xF7-0xFD) it stops without
   consuming it and reports LG_DIALOGUE_WAIT so a game service (wait for a
   button, scroll the window, ...) can act on the byte before the caller
   calls lg_dialogue_resume() to skip that single byte and continue.
   Multi-byte control operands are not handled: no verified ROM evidence
   yet establishes their length or meaning. */
typedef enum { LG_DIALOGUE_RUNNING, LG_DIALOGUE_WAIT, LG_DIALOGUE_DONE,
               LG_DIALOGUE_TRUNCATED, LG_DIALOGUE_FULL, LG_DIALOGUE_ERROR
} LGDialogueStatus;

typedef struct {
    const uint8_t *text;
    size_t size, pos;
    const uint8_t *widths;
    int x, y, start_x;
    int wrap_width; /* 0 disables; otherwise max pixels from start_x per line */
    LGGlyphSink sink;
    void *sink_context;
    LGDialogueStatus status;
    uint8_t control; /* valid only while status == LG_DIALOGUE_WAIT */
} LGDialogueState;

/* wrap_width bounds each line to that many pixels from x, breaking before
   whichever glyph would exceed it (character wrapping; it does not yet
   avoid breaking mid-word). Pass 0 to disable and lay out on one line
   until an explicit FE newline, as before. */
void lg_dialogue_init(LGDialogueState *d, const uint8_t *text, size_t size,
                       const uint8_t widths[256], int x, int y, int wrap_width,
                       LGGlyphSink sink, void *sink_context);
/* Lays out glyphs until the string ends, the sink is full, a control byte
   is hit, or an error occurs. Calling again after LG_DIALOGUE_WAIT without
   lg_dialogue_resume() re-reports the same control byte. */
LGDialogueStatus lg_dialogue_step(LGDialogueState *d);
/* Skips the pending single-byte control code and clears the wait. */
void lg_dialogue_resume(LGDialogueState *d);
#endif

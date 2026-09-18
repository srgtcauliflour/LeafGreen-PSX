#ifndef LG_LOOP_H
#define LG_LOOP_H
#include "lg/service.h"
#include "lg/window.h"

typedef enum { LG_LOOP_RUNNING, LG_LOOP_DONE, LG_LOOP_ERROR } LGLoopStatus;

/* Looks up the byte span for a portable text id (see LGScriptTextFn) so the
   loop can open an LGWindowState for it. Returns false for an id with no
   known text, which the loop treats as an error. */
typedef bool (*LGLoopTextLookupFn)(void *context, uint8_t text_id,
                                    const uint8_t **text, size_t *size);

/* Ties LGScriptVM + LGGameService + LGWindowState into one per-frame
   update, so a script can actually drive movement, dialogue and warps
   across real frames instead of each module being exercised in
   isolation. Still platform-neutral: widths/sink are whatever font
   backend the caller supplies (a synthetic one in host tests, the real
   PS1 backend once built), and nothing here touches PS1 hardware.

   OP_MOVE's BLOCKED wait is resolved immediately (movement is
   synchronous today -- no multi-frame tile slide exists yet). OP_WARP's
   BLOCKED wait is also resolved immediately: LGGameService already
   applied the resolved warp's destination x/y, and switched the active
   map/warp table too if a map table was registered with
   lg_game_service_set_map_table() (see service.h) -- otherwise the
   player moves but the map is unchanged, e.g. while real CD/resource
   loading for the destination doesn't exist yet. OP_TEXT's BLOCKED wait
   opens a real LGWindowState via text_lookup_fn and is only resolved
   once that window reports LG_WINDOW_DONE, since dialogue genuinely
   spans multiple frames. */
typedef struct {
    LGScriptVM *vm;
    LGGameService *service;
    const uint8_t *widths;
    LGGlyphSink sink;
    void *sink_context;
    LGLoopTextLookupFn text_lookup_fn;
    void *text_lookup_context;
    int text_x, text_y, text_wrap_width, text_max_lines;
    LGWindowState window;
    bool window_active;
} LGGameLoop;

void lg_game_loop_init(LGGameLoop *loop, LGScriptVM *vm, LGGameService *service,
                        const uint8_t widths[256], LGGlyphSink sink, void *sink_context,
                        LGLoopTextLookupFn text_lookup_fn, void *text_lookup_context,
                        int text_x, int text_y, int text_wrap_width, int text_max_lines);
/* Advances by one frame: drives an already-open dialogue window with
   advance_pressed, or steps the script VM otherwise, auto-resolving
   OP_MOVE/OP_WARP and opening a window for OP_TEXT as described above. */
LGLoopStatus lg_game_loop_step(LGGameLoop *loop, bool advance_pressed);
#endif

#include "lg/loop.h"

void lg_game_loop_init(LGGameLoop *loop, LGScriptVM *vm, LGGameService *service,
                        const uint8_t widths[256], LGGlyphSink sink, void *sink_context,
                        LGLoopTextLookupFn text_lookup_fn, void *text_lookup_context,
                        int text_x, int text_y, int text_wrap_width, int text_max_lines) {
    if (!loop) return;
    loop->vm = vm;
    loop->service = service;
    loop->widths = widths;
    loop->sink = sink;
    loop->sink_context = sink_context;
    loop->text_lookup_fn = text_lookup_fn;
    loop->text_lookup_context = text_lookup_context;
    loop->text_x = text_x;
    loop->text_y = text_y;
    loop->text_wrap_width = text_wrap_width;
    loop->text_max_lines = text_max_lines;
    loop->window_active = false;
}

LGLoopStatus lg_game_loop_step(LGGameLoop *loop, bool advance_pressed) {
    if (!loop || !loop->vm || !loop->service) return LG_LOOP_ERROR;

    if (loop->window_active) {
        LGWindowStatus ws = lg_window_step(&loop->window, advance_pressed);
        if (ws == LG_WINDOW_DONE) {
            loop->window_active = false;
            lg_game_service_clear_pending(loop->service);
            lg_script_unblock(loop->vm);
            return LG_LOOP_RUNNING;
        }
        if (ws == LG_WINDOW_ERROR || ws == LG_WINDOW_TRUNCATED || ws == LG_WINDOW_FULL) {
            loop->window_active = false;
            return LG_LOOP_ERROR;
        }
        return LG_LOOP_RUNNING; /* RUNNING, AWAIT_ADVANCE or AWAIT_SCROLL */
    }

    LGScriptStatus status = lg_script_step(loop->vm);
    if (status == LG_SCRIPT_DONE) return LG_LOOP_DONE;
    if (status == LG_SCRIPT_ERROR) return LG_LOOP_ERROR;
    if (status == LG_SCRIPT_BLOCKED) {
        if (loop->service->has_pending_text) {
            const uint8_t *text;
            size_t size;
            if (!loop->text_lookup_fn ||
                !loop->text_lookup_fn(loop->text_lookup_context,
                                       loop->service->pending_text_id, &text, &size)) {
                return LG_LOOP_ERROR;
            }
            lg_window_init(&loop->window, text, size, loop->widths, loop->text_x,
                            loop->text_y, loop->text_wrap_width, loop->text_max_lines,
                            loop->sink, loop->sink_context);
            loop->window_active = true;
        } else {
            /* OP_MOVE and a resolved OP_WARP (position and, if a map
               table is registered, the active map/warps) are both
               already applied synchronously inside LGGameService. */
            lg_game_service_clear_pending(loop->service);
            lg_script_unblock(loop->vm);
        }
    }
    return LG_LOOP_RUNNING;
}

#include "lg/loop.h"
#include <assert.h>
#include <string.h>

typedef struct { unsigned count; } Seen;
static bool capture(void *context, uint8_t glyph, int x, int y, uint8_t width) {
    (void)glyph; (void)x; (void)y; (void)width;
    ((Seen *)context)->count++;
    return true;
}

static bool lookup_text(void *context, uint8_t id, const uint8_t **text, size_t *size) {
    (void)context;
    static const uint8_t hi[] = {0xbb, 0xbc, 0xff}; /* two glyphs then end */
    if (id != 3) return false;
    *text = hi;
    *size = sizeof hi;
    return true;
}

int main(void) {
    /* 3x3 map: (1,2) carries warp id 7 to (dest 42, 5, 6). Player starts (1,1). */
    LGMapCell cells[9] = {0};
    cells[2 * 3 + 1].warp = 7;
    LGMap map = {3, 3, cells};
    LGWarp warps[] = {{7, 42, 5, 6}};
    LGPlayer player = {1, 1, 0, 0};
    LGGameService svc;
    LGScriptVM vm;
    lg_game_service_init(&svc, &player, &map, warps, 1);

    /* Move onto the warp tile, show text, then take the warp. */
    const uint8_t script[] = {5,0,1, 4,3, 6,7, 0};
    lg_script_init(&vm, script, sizeof script);
    lg_game_service_bind(&svc, &vm);

    uint8_t widths[256];
    memset(widths, 6, sizeof widths);
    Seen seen = {0};
    LGGameLoop loop;
    lg_game_loop_init(&loop, &vm, &svc, widths, capture, &seen, lookup_text, 0,
                       10, 20, 0, 0);

    /* Frame 1: OP_MOVE resolves synchronously; player moves onto the warp tile. */
    assert(lg_game_loop_step(&loop, false) == LG_LOOP_RUNNING);
    assert(player.x == 1 && player.y == 2);
    assert(!loop.window_active);

    /* Frame 2: OP_TEXT opens a real window for the looked-up text; nothing
       drawn yet this frame, just opened. */
    assert(lg_game_loop_step(&loop, false) == LG_LOOP_RUNNING);
    assert(loop.window_active);
    assert(seen.count == 0);

    /* Frame 3: the window has no control bytes, so it finishes in one step,
       drawing both glyphs, closing the window and unblocking the script. */
    assert(lg_game_loop_step(&loop, false) == LG_LOOP_RUNNING);
    assert(seen.count == 2);
    assert(!loop.window_active);

    /* Frame 4: OP_WARP resolves synchronously, applying the destination
       coordinates (but not swapping the map -- that's still the caller's
       job once real resource loading exists). */
    assert(lg_game_loop_step(&loop, false) == LG_LOOP_RUNNING);
    assert(player.x == 5 && player.y == 6);

    /* Frame 5: OP_END. */
    assert(lg_game_loop_step(&loop, false) == LG_LOOP_DONE);

    /* An id with no known text is a loop error, not a silent skip. */
    LGPlayer player2 = {1, 1, 0, 0};
    LGGameService svc2;
    LGScriptVM vm2;
    lg_game_service_init(&svc2, &player2, &map, warps, 1);
    const uint8_t bad_text_script[] = {4,9, 0};
    lg_script_init(&vm2, bad_text_script, sizeof bad_text_script);
    lg_game_service_bind(&svc2, &vm2);
    LGGameLoop loop2;
    lg_game_loop_init(&loop2, &vm2, &svc2, widths, capture, &seen, lookup_text, 0,
                       10, 20, 0, 0);
    assert(lg_game_loop_step(&loop2, false) == LG_LOOP_ERROR);

    /* A rejected move is a script error surfaced as a loop error too. */
    LGMapCell blocked_cells[9] = {0};
    blocked_cells[1 * 3 + 2].collision = 1;
    LGMap blocked_map = {3, 3, blocked_cells};
    LGPlayer player3 = {1, 1, 0, 0};
    LGGameService svc3;
    LGScriptVM vm3;
    lg_game_service_init(&svc3, &player3, &blocked_map, warps, 1);
    const uint8_t blocked_move_script[] = {5,1,0, 0};
    lg_script_init(&vm3, blocked_move_script, sizeof blocked_move_script);
    lg_game_service_bind(&svc3, &vm3);
    LGGameLoop loop3;
    lg_game_loop_init(&loop3, &vm3, &svc3, widths, capture, &seen, lookup_text, 0,
                       10, 20, 0, 0);
    assert(lg_game_loop_step(&loop3, false) == LG_LOOP_ERROR);

    /* Invalid setup is rejected. */
    LGGameLoop bad_loop;
    lg_game_loop_init(&bad_loop, 0, 0, widths, capture, &seen, lookup_text, 0,
                       0, 0, 0, 0);
    assert(lg_game_loop_step(&bad_loop, false) == LG_LOOP_ERROR);
    assert(lg_game_loop_step(0, false) == LG_LOOP_ERROR);

    return 0;
}

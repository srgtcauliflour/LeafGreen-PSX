#include "lg/service.h"
#include <assert.h>
#include <string.h>

int main(void) {
    /* 3x3 map: (2,1) is blocked, (1,2) carries warp id 7. */
    LGMapCell cells[9] = {0};
    cells[1 * 3 + 2].collision = 1; /* (2,1) */
    cells[2 * 3 + 1].warp = 7;      /* (1,2) */
    LGMap map = {3, 3, cells};
    LGWarp warps[] = {{7, 42, 5, 6}};
    LGPlayer player = {1, 1, 0, 0};
    LGGameService svc;
    LGScriptVM vm;

    lg_game_service_init(&svc, &player, &map, warps, 1);

    /* A script that moves down onto the warp tile, requests text, then
       requests that warp -- exercising OP_MOVE, OP_TEXT and OP_WARP all
       resolved through the same bound service. */
    const uint8_t script[] = {5,0,1, 4,3, 6,7, 0};
    lg_script_init(&vm, script, sizeof script);
    lg_game_service_bind(&svc, &vm);

    assert(lg_script_step(&vm) == LG_SCRIPT_BLOCKED); /* OP_MOVE 0,1 */
    assert(player.x == 1 && player.y == 2);
    /* Movement is resolved synchronously (no multi-frame slide yet), so
       the caller can unblock right away. */
    lg_script_unblock(&vm);

    assert(lg_script_step(&vm) == LG_SCRIPT_BLOCKED); /* OP_TEXT 3 */
    assert(svc.has_pending_text && svc.pending_text_id == 3);
    /* A real caller would drive its LGWindowState here and only unblock
       once it reports LG_WINDOW_DONE; this test resolves it immediately. */
    lg_game_service_clear_pending(&svc);
    lg_script_unblock(&vm);

    assert(lg_script_step(&vm) == LG_SCRIPT_BLOCKED); /* OP_WARP 7 */
    assert(svc.has_pending_warp);
    assert(svc.pending_warp->dest_map == 42);
    assert(svc.pending_warp->dest_x == 5 && svc.pending_warp->dest_y == 6);
    lg_game_service_clear_pending(&svc);
    lg_script_unblock(&vm);

    assert(lg_script_step(&vm) == LG_SCRIPT_DONE);
    assert(!svc.has_pending_text && !svc.has_pending_warp);

    /* A move into a blocked cell is rejected -- OP_MOVE's callback
       contract treats that as a script error, not a silent no-op. */
    player = (LGPlayer){1, 1, 0, 0};
    const uint8_t blocked_move[] = {5,1,0, 0};
    lg_script_init(&vm, blocked_move, sizeof blocked_move);
    lg_game_service_bind(&svc, &vm);
    assert(lg_script_step(&vm) == LG_SCRIPT_ERROR);
    assert(player.x == 1 && player.y == 1); /* never moved */

    /* A warp id with no matching table entry is likewise a script error. */
    const uint8_t bad_warp[] = {6,9, 0};
    lg_script_init(&vm, bad_warp, sizeof bad_warp);
    lg_game_service_bind(&svc, &vm);
    assert(lg_script_step(&vm) == LG_SCRIPT_ERROR);
    assert(!svc.has_pending_warp);

    return 0;
}

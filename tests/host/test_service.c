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
    /* The destination position is applied synchronously, immediately when
       the warp resolves -- not deferred to whoever unblocks the VM. */
    assert(player.x == 5 && player.y == 6);
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

    /* With a map table registered, a resolved warp whose dest_map matches
       an entry switches the active map/warp table too, not just position. */
    LGMapCell map_b_cells[4] = {0};
    map_b_cells[1 * 2 + 1].warp = 3; /* (1,1) */
    LGMap map_b = {2, 2, map_b_cells};
    LGWarp map_b_warps[] = {{3, 7, 1, 2}}; /* back to map A's warp tile */
    LGMapEntry table[] = {{42, &map_b, map_b_warps, 1}};

    LGPlayer player_x = {1, 1, 0, 0};
    LGGameService svc_x;
    LGScriptVM vm_x;
    lg_game_service_init(&svc_x, &player_x, &map, warps, 1);
    lg_game_service_set_map_table(&svc_x, table, 1);
    const uint8_t warp_script[] = {5,0,1, 6,7, 0}; /* move onto warp tile, warp */
    lg_script_init(&vm_x, warp_script, sizeof warp_script);
    lg_game_service_bind(&svc_x, &vm_x);
    assert(lg_script_step(&vm_x) == LG_SCRIPT_BLOCKED); /* OP_MOVE */
    lg_script_unblock(&vm_x);
    assert(lg_script_step(&vm_x) == LG_SCRIPT_BLOCKED); /* OP_WARP 7 -> map B */
    assert(player_x.x == 5 && player_x.y == 6);
    assert(svc_x.map == &map_b);
    assert(svc_x.warps == map_b_warps && svc_x.warp_count == 1);
    lg_game_service_clear_pending(&svc_x);
    lg_script_unblock(&vm_x);
    assert(lg_script_step(&vm_x) == LG_SCRIPT_DONE);

    /* A resolved warp whose dest_map has no map-table entry still moves
       the player, but leaves the active map/warps alone. */
    LGPlayer player_y = {1, 1, 0, 0};
    LGGameService svc_y;
    LGScriptVM vm_y;
    lg_game_service_init(&svc_y, &player_y, &map, warps, 1);
    lg_game_service_set_map_table(&svc_y, table, 1); /* has no entry for id 42... */
    LGWarp warps_no_match[] = {{7, 999, 3, 3}}; /* dest_map 999 isn't in table */
    svc_y.warps = warps_no_match;
    lg_script_init(&vm_y, warp_script, sizeof warp_script);
    lg_game_service_bind(&svc_y, &vm_y);
    assert(lg_script_step(&vm_y) == LG_SCRIPT_BLOCKED); /* OP_MOVE */
    lg_script_unblock(&vm_y);
    assert(lg_script_step(&vm_y) == LG_SCRIPT_BLOCKED); /* OP_WARP 7 */
    assert(player_y.x == 3 && player_y.y == 3);
    assert(svc_y.map == &map); /* unchanged: no matching table entry */

    return 0;
}

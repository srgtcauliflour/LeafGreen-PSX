#include "lg/service.h"

static bool service_move(void *context, int8_t dx, int8_t dy) {
    LGGameService *svc = context;
    int nx = svc->player->x + dx, ny = svc->player->y + dy;
    if (!lg_map_can_enter(svc->map, nx, ny)) return false;
    lg_player_step(svc->player, svc->map, dx, dy);
    return true;
}

static bool service_text(void *context, uint8_t text_id) {
    LGGameService *svc = context;
    svc->has_pending_text = true;
    svc->pending_text_id = text_id;
    return true;
}

static bool service_warp(void *context, uint8_t warp_id) {
    LGGameService *svc = context;
    for (size_t i = 0; i < svc->warp_count; ++i) {
        if (svc->warps[i].id == warp_id) {
            svc->has_pending_warp = true;
            svc->pending_warp = &svc->warps[i];
            return true;
        }
    }
    return false;
}

void lg_game_service_init(LGGameService *svc, LGPlayer *player, const LGMap *map,
                           const LGWarp *warps, size_t warp_count) {
    if (!svc) return;
    svc->player = player;
    svc->map = map;
    svc->warps = warps;
    svc->warp_count = warp_count;
    svc->has_pending_text = false;
    svc->pending_text_id = 0;
    svc->has_pending_warp = false;
    svc->pending_warp = 0;
}

void lg_game_service_bind(LGGameService *svc, LGScriptVM *vm) {
    if (!svc || !vm) return;
    lg_script_set_move_fn(vm, service_move, svc);
    lg_script_set_text_fn(vm, service_text, svc);
    lg_script_set_warp_fn(vm, service_warp, svc);
}

void lg_game_service_clear_pending(LGGameService *svc) {
    if (!svc) return;
    svc->has_pending_text = false;
    svc->has_pending_warp = false;
    svc->pending_warp = 0;
}

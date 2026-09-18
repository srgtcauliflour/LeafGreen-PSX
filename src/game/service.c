#include "lg/service.h"

static bool service_move(void *context, int8_t dx, int8_t dy) {
    LGGameService *svc = context;
    int nx = svc->player->x + dx, ny = svc->player->y + dy;
    if (!lg_map_can_enter_from(svc->map, svc->player->x, svc->player->y, nx, ny)) return false;
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
        if (svc->warps[i].id != warp_id) continue;
        lg_game_service_apply_warp(svc, &svc->warps[i]);
        return true;
    }
    return false;
}

void lg_game_service_apply_warp(LGGameService *svc, const LGWarp *warp) {
    if (!svc || !svc->player || !warp) return;
    svc->has_pending_warp = true;
    svc->pending_warp = warp;
    svc->player->x = warp->dest_x;
    svc->player->y = warp->dest_y;
    for (size_t j = 0; j < svc->map_table_count; ++j) {
        if (svc->map_table[j].map_id != warp->dest_map) continue;
        svc->map = svc->map_table[j].map;
        svc->warps = svc->map_table[j].warps;
        svc->warp_count = svc->map_table[j].warp_count;
        break;
    }
}

static bool service_item(void *context, uint8_t item_id, uint16_t quantity) {
    LGGameService *svc = context;
    if (!svc->inventory || !lg_inventory_add(svc->inventory, item_id, quantity)) return false;
    svc->has_pending_item = true;
    svc->pending_item_id = item_id;
    svc->pending_item_quantity = quantity;
    return true;
}

static bool service_item_take(void *context, uint8_t item_id, uint16_t quantity) {
    LGGameService *svc = context;
    if (!svc->inventory || !lg_inventory_remove(svc->inventory, item_id, quantity)) return false;
    svc->has_pending_item_take = true;
    svc->pending_item_take_id = item_id;
    svc->pending_item_take_quantity = quantity;
    return true;
}

void lg_game_service_init(LGGameService *svc, LGPlayer *player, const LGMap *map,
                           const LGWarp *warps, size_t warp_count) {
    if (!svc) return;
    svc->player = player;
    svc->map = map;
    svc->warps = warps;
    svc->warp_count = warp_count;
    svc->map_table = 0;
    svc->map_table_count = 0;
    svc->inventory = 0;
    svc->has_pending_text = false;
    svc->pending_text_id = 0;
    svc->has_pending_warp = false;
    svc->pending_warp = 0;
    svc->has_pending_item = false;
    svc->pending_item_id = 0;
    svc->pending_item_quantity = 0;
    svc->has_pending_item_take = false;
    svc->pending_item_take_id = 0;
    svc->pending_item_take_quantity = 0;
}

void lg_game_service_set_map_table(LGGameService *svc, const LGMapEntry *table,
                                    size_t count) {
    if (!svc) return;
    svc->map_table = table;
    svc->map_table_count = count;
}

void lg_game_service_set_inventory(LGGameService *svc, LGInventory *inv) {
    if (!svc) return;
    svc->inventory = inv;
}

void lg_game_service_bind(LGGameService *svc, LGScriptVM *vm) {
    if (!svc || !vm) return;
    lg_script_set_move_fn(vm, service_move, svc);
    lg_script_set_text_fn(vm, service_text, svc);
    lg_script_set_warp_fn(vm, service_warp, svc);
    lg_script_set_item_fn(vm, service_item, svc);
    lg_script_set_item_take_fn(vm, service_item_take, svc);
}

void lg_game_service_clear_pending(LGGameService *svc) {
    if (!svc) return;
    svc->has_pending_text = false;
    svc->has_pending_warp = false;
    svc->pending_warp = 0;
    svc->has_pending_item = false;
    svc->has_pending_item_take = false;
}

#ifndef LG_SERVICE_H
#define LG_SERVICE_H
#include "lg/inventory.h"
#include "lg/overworld.h"
#include "lg/script.h"

/* A destination a warp can switch to: dest_map (see LGWarp.dest_map)
   matched against map_id, plus that destination map's own cells, warp
   table and object events (LGObjectEvent, see overworld.h -- runner.h's
   LGOverworldRunner uses this to keep its interact list in sync with
   whichever map is actually active). This is a caller-supplied,
   statically-known table lookup, not resource loading -- there is no CD
   read here, so it's only as real as whatever maps/warps/events the
   caller already has in memory. events/event_count may be left 0/0 for
   a map with nothing to interact with. */
typedef struct {
    uint16_t map_id;
    const LGMap *map;
    const LGWarp *warps;
    size_t warp_count;
    const LGObjectEvent *events;
    size_t event_count;
} LGMapEntry;

/* Wires LGScriptVM's game-service callbacks
   (OP_MOVE/OP_TEXT/OP_WARP/OP_ITEM/OP_ITEM_TAKE) to the real portable
   overworld model, so a script can actually move the player and request
   dialogue/warps/item gain-or-loss instead of only exercising the VM in
   isolation. It stays platform-neutral: no PS1 GPU/controller/CD calls,
   no font/window rendering (that needs widths/a sink the caller owns).

   Movement, warp and item mutations are resolved synchronously here --
   there is no multi-frame tile slide, async CD warp or item-pickup
   animation yet, so it is correct for a caller to call
   lg_script_unblock() immediately after a BLOCKED
   OP_MOVE/OP_WARP/OP_ITEM/OP_ITEM_TAKE step returns. OP_TEXT is
   different: dialogue really does take multiple frames, so this only
   records the requested id; the caller must drive its own
   LGWindowState/font backend from pending_text_id and only unblock once
   that reports LG_WINDOW_DONE. */
typedef struct {
    LGPlayer *player;
    const LGMap *map;
    const LGWarp *warps;
    size_t warp_count;

    const LGMapEntry *map_table; /* optional; 0/0 disables cross-map warps */
    size_t map_table_count;

    LGInventory *inventory; /* optional; 0 leaves OP_ITEM unsupported */

    bool has_pending_text;
    uint8_t pending_text_id;

    bool has_pending_warp;
    const LGWarp *pending_warp; /* valid only while has_pending_warp is true */

    bool has_pending_item;
    uint8_t pending_item_id;
    uint16_t pending_item_quantity;

    bool has_pending_item_take;
    uint8_t pending_item_take_id;
    uint16_t pending_item_take_quantity;
} LGGameService;

void lg_game_service_init(LGGameService *svc, LGPlayer *player, const LGMap *map,
                           const LGWarp *warps, size_t warp_count);
/* Registers a table of warp destinations. On a resolved OP_WARP whose
   LGWarp.dest_map matches an entry, the service switches map/warps to
   that entry's (in addition to always applying dest_x/dest_y to the
   player) so a subsequent OP_MOVE/OP_WARP acts against the new map. Pass
   table=0/count=0 (the lg_game_service_init() default) to leave
   cross-map warps unresolved -- the player still moves to dest_x/dest_y,
   but svc->map is unchanged, e.g. while real CD/resource loading for the
   destination doesn't exist yet. */
void lg_game_service_set_map_table(LGGameService *svc, const LGMapEntry *table,
                                    size_t count);
/* Registers the LGInventory OP_ITEM mutates via lg_inventory_add(). Pass
   inv=0 (the lg_game_service_init() default) to leave OP_ITEM
   unsupported -- it becomes a script error, the same as any other
   missing service callback, rather than silently discarding the item. */
void lg_game_service_set_inventory(LGGameService *svc, LGInventory *inv);
/* Applies warp directly, the same way a resolved OP_WARP does: records
   has_pending_warp/pending_warp, moves the player to
   warp->dest_x/dest_y, and, if a registered LGMapEntry's map_id matches
   warp->dest_map, switches svc->map/warps/warp_count to it too. No-op if
   svc, svc->player or warp is NULL. This is the shared warp-application
   logic OP_WARP's callback uses internally; it also lets a caller trigger
   a warp without going through a script at all -- e.g. LGOverworldRunner
   stepping onto a map cell whose LGMapCell.warp is set (see
   lg_map_warp_at() in overworld.h), the same way LeafGreen walks the
   player through a door tile with no separate button press or script. */
void lg_game_service_apply_warp(LGGameService *svc, const LGWarp *warp);
/* Registers this service's move/text/warp/item callbacks on vm. The
   service must outlive the VM (or be re-bound after any
   lg_script_init()), since the VM only stores the callback pointers and
   this context pointer. */
void lg_game_service_bind(LGGameService *svc, LGScriptVM *vm);
/* Clears has_pending_text/has_pending_warp/has_pending_item/
   has_pending_item_take; call once the caller has consumed and fully
   resolved that request (e.g. after unblocking the VM), so a later
   request isn't confused with a stale one. */
void lg_game_service_clear_pending(LGGameService *svc);
#endif

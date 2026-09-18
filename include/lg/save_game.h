#ifndef LG_SAVE_GAME_H
#define LG_SAVE_GAME_H
#include "lg/inventory.h"
#include "lg/overworld.h"
#include "lg/save.h"
#include "lg/script.h"
#include <stdint.h>

/* Fixed slot count for the inventory embedded in LGSaveGamePayload. This is
   scaffolding, not a verified LeafGreen bag size -- real bag pockets and
   capacity still need ROM evidence (see lg/inventory.h, SCRIPT-VM.md). A
   live LGInventory must have exactly this many slots to be captured into or
   applied from a save; a different slot_count is a no-op, same as any other
   invalid argument here. */
#define LG_SAVE_INVENTORY_SLOTS 20

/* M0 gameplay save payload: player position/facing, the 256 script flags
   (mirrors LGScriptVM.flags), and a fixed-size inventory snapshot. This is
   the first concrete schema behind the versioned LGSV envelope reserved in
   save.h -- SAVE-FORMAT.md deferred exactly this until game-state models
   existed to serialize. Expect it to grow (and LG_SAVE_VERSION to bump)
   further as more M0 state -- party, more flags -- gets ported; this is
   not a complete LeafGreen save format. */
typedef struct {
    int16_t player_x, player_y;
    int8_t player_facing;
    uint8_t flags[32];
    LGInventorySlot inventory[LG_SAVE_INVENTORY_SLOTS];
} LGSaveGamePayload;

/* Writes an LGSV-enveloped copy of payload into out (header immediately
   followed by the payload bytes). Returns the total bytes written, or 0
   if out is too small or args are invalid. */
size_t lg_save_game_write(const LGSaveGamePayload *payload, uint64_t save_id,
                           uint8_t *out, size_t capacity);
/* Validates the header (magic/version/size/CRC32) and, on success, copies
   the payload out and reports the save_id. Returns 1 on success, 0 on any
   corruption/mismatch (wrong magic/version, truncated data, bad CRC, or a
   payload_size that doesn't match sizeof(LGSaveGamePayload)). */
int lg_save_game_read(const uint8_t *data, size_t size,
                       LGSaveGamePayload *out_payload, uint64_t *out_save_id);
/* Fills out from the live player/vm/inventory state (player x/y/facing,
   vm's 256 flags, inventory's slots). No-op if any argument is NULL, if
   inventory->slots is NULL, or if inventory->slot_count doesn't equal
   LG_SAVE_INVENTORY_SLOTS exactly -- this never partially fills out. */
void lg_save_game_capture(const LGPlayer *player, const LGScriptVM *vm,
                           const LGInventory *inventory, LGSaveGamePayload *out);
/* The inverse: writes payload's fields back onto player/vm/inventory. No-op
   if any argument is NULL, if inventory->slots is NULL, or if
   inventory->slot_count doesn't equal LG_SAVE_INVENTORY_SLOTS exactly.
   This only touches x/y/facing/flags/inventory slots -- it does not reset
   vm->pc/status/code or anything else about the VM's execution state,
   since resuming mid-script from a save isn't part of this schema yet
   (see LGSaveGamePayload's own doc comment). */
void lg_save_game_apply(const LGSaveGamePayload *payload, LGPlayer *player,
                         LGScriptVM *vm, LGInventory *inventory);
#endif

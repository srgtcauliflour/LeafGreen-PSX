#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
typedef enum { LG_SCRIPT_RUNNING=0, LG_SCRIPT_WAITING, LG_SCRIPT_BLOCKED, LG_SCRIPT_DONE, LG_SCRIPT_ERROR } LGScriptStatus;
/* Called for OP_TEXT with the raw 1-byte operand as a portable text id (not
   a LeafGreen text index). Returns false to reject an id the game service
   doesn't recognise, which the VM treats as a script error. On true, the VM
   blocks until the caller reports the text/dialogue finished via
   lg_script_unblock(); unlike OP_WAIT this never resumes on its own,
   because the game service's own timing decides when it's done, not one
   fixed VM step. */
typedef bool (*LGScriptTextFn)(void *context, uint8_t text_id);
/* Called for OP_MOVE with a single grid step (dx, dy each in {-1,0,1}, not
   both nonzero -- the same shape lg_player_step() accepts). Returns false
   to reject a step the game service can't take (e.g. blocked/out of
   bounds), which the VM treats as a script error. On true, the VM blocks
   the same way OP_TEXT does: the caller calls lg_script_unblock() once the
   movement (e.g. a multi-frame slide into the next tile) has finished. */
typedef bool (*LGScriptMoveFn)(void *context, int8_t dx, int8_t dy);
/* Called for OP_WARP with the raw 1-byte operand as a portable warp id (see
   lg_map_warp_at() in overworld.h -- LGWarp.id, not a LeafGreen warp
   index). Returns false to reject an id the game service can't resolve.
   On true, blocks like OP_TEXT/OP_MOVE: a warp can mean an async CD
   resource swap for the destination map/area bundle, so the caller calls
   lg_script_unblock() once that has actually finished. */
typedef bool (*LGScriptWarpFn)(void *context, uint8_t warp_id);
/* Called for OP_ITEM with a portable item id and a 16-bit quantity (add
   count, not a running total). Returns false to reject an id the game
   service doesn't recognise or a quantity it can't apply (e.g. bag full),
   which the VM treats as a script error. On true, blocks like
   OP_TEXT/OP_MOVE/OP_WARP: even though an inventory mutation itself is
   instant, a real caller very likely follows it with its own "got an
   item!" message/animation, so the caller decides when that's done and
   calls lg_script_unblock(). This has no game-service wiring yet (no
   inventory model exists in this codebase): it establishes the VM-side
   scaffolding SCRIPT-VM.md called for, matching OP_TEXT/OP_MOVE/OP_WARP's
   own history of VM opcode first, LGGameService wiring later. */
typedef bool (*LGScriptItemFn)(void *context, uint8_t item_id, uint16_t quantity);
/* Called for OP_ITEM_TAKE with the same operand shape as OP_ITEM (a
   portable item id and a 16-bit quantity), but for removing items instead
   of adding them -- e.g. a script that consumes a key item, pays an item
   cost, or triggers a trade. Returns false to reject an id the game
   service doesn't recognise or a quantity it can't remove (e.g. the bag
   doesn't have that many), which the VM treats as a script error, the
   same as OP_ITEM. On true, blocks the same way: the mutation itself is
   instant, but the caller likely follows it with its own message/
   animation before calling lg_script_unblock(). */
typedef struct {
    const uint8_t *code; size_t size, pc; uint16_t vars[32];
    uint8_t flags[32]; /* 256 single-bit flags; flags[id/8] bit (id%8) */
    LGScriptStatus status;
    LGScriptTextFn text_fn; void *text_context;
    LGScriptMoveFn move_fn; void *move_context;
    LGScriptWarpFn warp_fn; void *warp_context;
    LGScriptItemFn item_fn; void *item_context;
    LGScriptItemFn item_take_fn; void *item_take_context;
} LGScriptVM;
void lg_script_init(LGScriptVM *vm,const uint8_t *code,size_t size);
/* Registers the text/dialogue service callback; pass fn=0 to leave OP_TEXT
   unsupported (it errors instead of blocking forever with nothing able to
   resolve it). Never wires PS1 GPU/controller/CD calls in here directly:
   fn belongs to the platform-neutral game-service layer. */
void lg_script_set_text_fn(LGScriptVM *vm, LGScriptTextFn fn, void *context);
/* Registers the movement service callback; pass fn=0 to leave OP_MOVE
   unsupported, for the same reason as the text callback above. */
void lg_script_set_move_fn(LGScriptVM *vm, LGScriptMoveFn fn, void *context);
/* Registers the warp service callback; pass fn=0 to leave OP_WARP
   unsupported, for the same reason as the text callback above. */
void lg_script_set_warp_fn(LGScriptVM *vm, LGScriptWarpFn fn, void *context);
/* Registers the item service callback; pass fn=0 to leave OP_ITEM
   unsupported, for the same reason as the text callback above. */
void lg_script_set_item_fn(LGScriptVM *vm, LGScriptItemFn fn, void *context);
/* Registers the item-removal service callback; pass fn=0 to leave
   OP_ITEM_TAKE unsupported, for the same reason as the text callback
   above. */
void lg_script_set_item_take_fn(LGScriptVM *vm, LGScriptItemFn fn, void *context);
LGScriptStatus lg_script_step(LGScriptVM *vm);
/* Resolves a pending LG_SCRIPT_BLOCKED wait (e.g. the dialogue/window the
   last OP_TEXT opened has finished). No-op unless the VM is blocked. */
void lg_script_unblock(LGScriptVM *vm);

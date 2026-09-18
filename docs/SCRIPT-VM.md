# Script VM

This is a deliberately tiny host-testable VM scaffold for the future LGPSX-019 event system. Its prototype opcodes are **not** claimed to be LeafGreen bytecode. They exist to establish execution state, bounds checking, variables, yielding/waiting and deterministic tests before the exact game command set is mapped.

Production work must map required LeafGreen commands from the exact Rev 1 reference and implement them behind portable callbacks for text, movement, flags, warps, items and other game services. Never bake PS1 GPU/controller/CD calls into script opcode handlers.

## Text service callback (`OP_TEXT`)

`lg_script_set_text_fn(vm, fn, context)` registers a portable callback of
type `LGScriptTextFn` (`bool fn(void *context, uint8_t text_id)`). `OP_TEXT`
reads a 1-byte id operand and calls it: a rejecting or missing callback is a
script error (there is nothing able to resolve the wait), and an accepting
callback moves the VM to a new `LG_SCRIPT_BLOCKED` status. Unlike
`LG_SCRIPT_WAITING` (which `lg_script_step` clears automatically on the next
call, a one-frame wait), `LG_SCRIPT_BLOCKED` never clears on its own —
`lg_script_step` is a no-op while blocked. The caller must call
`lg_script_unblock(vm)` once the game service has actually finished (e.g.
the `LGWindowState`-driven dialogue reached `LG_WINDOW_DONE`), since only
that service knows when its own multi-frame work is done.

The `text_id` byte is our own scaffolding, not a LeafGreen text/dialogue
index: no ROM evidence maps real script bytecode ids to real dialogue text
yet. This establishes the blocking/callback machinery a real opcode set
will need for text, movement, warps and similar multi-frame game services.

## Movement service callback (`OP_MOVE`)

`lg_script_set_move_fn(vm, fn, context)` registers `LGScriptMoveFn`
(`bool fn(void *context, int8_t dx, int8_t dy)`). `OP_MOVE` reads two
signed 1-byte operands and validates the same step shape
`lg_player_step()` accepts (each of dx/dy in `{-1,0,1}`, never both
nonzero) before calling it -- an invalid shape, a missing callback, or a
rejecting callback are all script errors. An accepting callback blocks the
VM exactly like `OP_TEXT` does, since a real move is a multi-frame slide
into the next tile, not a one-step VM action; the caller calls
`lg_script_unblock()` once that finishes.

## Warp service callback (`OP_WARP`)

`lg_script_set_warp_fn(vm, fn, context)` registers `LGScriptWarpFn`
(`bool fn(void *context, uint8_t warp_id)`). `OP_WARP` reads a 1-byte warp
id (see `LGWarp.id` in overworld.h -- a portable indirection, not a
LeafGreen warp index) and calls it; a missing or rejecting callback (an id
absent from the service's warp table) is a script error. An accepting
callback blocks the VM the same way, since a warp can mean an async CD
resource swap for the destination map/area bundle, not a same-step action.

## Item service callback (`OP_ITEM`)

`lg_script_set_item_fn(vm, fn, context)` registers `LGScriptItemFn`
(`bool fn(void *context, uint8_t item_id, uint16_t quantity)`). `OP_ITEM`
reads a 1-byte item id and a 16-bit little-endian quantity (an add count,
not a running total) and calls it; a missing or rejecting callback (an
id/quantity the service can't apply, e.g. bag full) is a script error. An
accepting callback blocks the VM the same way as the others: the
inventory mutation itself is instant, but a real caller very likely
follows it with its own "got an item!" message/animation, so the caller
decides when that's done. Unlike `OP_TEXT`/`OP_MOVE`/`OP_WARP`, this has
no `LGGameService` wiring yet -- there is no inventory model anywhere in
this codebase. This establishes the VM-side scaffolding the way those
three did before their own service wiring arrived in later PRs.

## Flags (`OP_FLAG_SET`, `OP_JUMP_IF_FLAG`)

`LGScriptVM` carries 256 single-bit flags (`flags[32]`), separate from the
16-bit `vars`, needing no callback since they're pure VM state. `OP_FLAG_SET`
(id, value) sets or clears a flag; `OP_JUMP_IF_FLAG` (id, value, little-endian
absolute address) jumps there when the flag currently equals value, otherwise
falls through to the next instruction. The jump target is always bounds-
checked against the code size up front, even when the branch isn't taken --
consistent with how `OP_SET`/`OP_ADD` validate their var index regardless of
which branch executes. A jump only moves `pc`; the instruction at the target
executes on the following `lg_script_step()` call, not the same one.

## Game service integration (`lg/service.h`)

`LGGameService` (`src/game/service.c`) wires `OP_MOVE`/`OP_TEXT`/`OP_WARP`
to the real portable overworld model via `lg_game_service_bind(svc, vm)`:
`OP_MOVE` resolves through `lg_map_can_enter()`/`lg_player_step()`, `OP_WARP`
searches a caller-supplied `LGWarp` table for a matching id. Both are
resolved synchronously today (there is no multi-frame tile slide or async CD
warp yet), so a caller may call `lg_script_unblock()` immediately after a
`BLOCKED` step from either. `OP_TEXT` only records the requested id in
`pending_text_id`; the caller must still drive its own `LGWindowState`/font
backend and unblock only once that reports `LG_WINDOW_DONE`, since dialogue
genuinely spans multiple frames. See `tests/host/test_service.c` for a full
script run (move, request text, request warp) exercised end to end.

A resolved `OP_WARP` immediately applies the matched `LGWarp`'s
`dest_x`/`dest_y` to the player. If `lg_game_service_set_map_table(svc,
table, count)` registered an `LGMapEntry` whose `map_id` matches the
warp's `dest_map`, it also switches `svc->map`/`svc->warps` to that
entry's, so the next `OP_MOVE`/`OP_WARP` acts against the destination map
-- this is a caller-supplied, statically-known table lookup, not resource
loading, so it's only as real as whatever maps the caller already holds
in memory. A `dest_map` absent from the table still moves the player but
leaves the active map alone (e.g. until real CD/resource loading for that
destination exists). `LGMapEntry` also carries the destination map's own
`LGObjectEvent` table (`events`/`event_count`) -- `LGOverworldRunner`
(runner.h) uses this to keep its interact list following whichever map is
actually active; see WORK-HANDOFF.md's "Object events follow map
switches" entry.

This wiring is scaffolding, not a finished event system: it has no real M0
script bytecode, map data or PS1 runtime evidence behind it yet, so it does
not complete LGPSX-012 through LGPSX-019 on its own.

## Per-frame game loop (`lg/loop.h`)

`LGGameLoop` (`src/game/loop.c`) is the piece above `LGGameService` that
actually drives a script across real frames: `lg_game_loop_step(loop,
advance_pressed)` steps an active `LGWindowState` when one is open, or
otherwise steps the VM once and resolves whatever it blocked on --
`OP_MOVE` and `OP_WARP` immediately (both are synchronous today; a warp
additionally applies the resolved destination x/y to the player, but does
NOT swap the active map, since that needs real resource loading this
scaffolding doesn't have), and `OP_TEXT` by looking up the requested id's
byte span via a caller-supplied `LGLoopTextLookupFn` and opening a real
window for it, only resolving once that window reports `LG_WINDOW_DONE`.

`tests/host/test_loop.c` runs a full multi-frame script (move onto a warp
tile, open and finish a dialogue window, take the warp, end) against a
live `LGMap`/`LGPlayer`/font-widths-and-sink pair, plus the loop-level
error cases (an id with no known text, a rejected move, invalid setup).
Widths/sink/text lookup are all caller-supplied, so this stays fully
host-testable without PS1 hardware or real font/ROM data -- the same
inputs would come from the real font backend and a real dialogue-text
table once those exist. This is the closest this repo gets to a "working"
M0 loop without a PSn00bSDK build and real ROM assets: it is still
scaffolding until both of those, and real map/script data, exist.

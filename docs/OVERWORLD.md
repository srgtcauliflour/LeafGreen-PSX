# Overworld core

The overworld model is intentionally platform-neutral. Maps expose dimensions plus compact cells containing a metatile ID, collision class, elevation and a warp id. Player movement asks the map whether a destination is enterable; rendering and controller polling remain outside this module.

The first implementation provides deterministic grid movement/collision primitives that can be host-tested before PS1 integration. LeafGreen behaviour such as ledges, doors, object events, elevation interactions, running, biking, surfing and scripted movement will extend this model without putting PS1 hardware calls into game logic.

## Map connections/warps (LGPSX-018, portable half)

`lg_map_warp_at(map, x, y, warps, warp_count)` looks up the cell at `(x, y)`
and, if its `warp` field is nonzero, finds the matching entry (by id) in a
caller-supplied `LGWarp` table. It returns `NULL` for out-of-bounds
coordinates, a cell with no warp, or an id missing from the table (an
incomplete table is not a crash). `LGWarp.dest_map` is a logical resource id
(see resource.h), not a pointer, so a warp table can be authored/serialized
independently of how maps are linked in memory. Warp ids are a portable
indirection, not real LeafGreen warp indices: no ROM evidence maps actual
door/stairs ids yet, and this does not decide when a warp fires (map edge,
a specific tile, an object event) — that policy, and the real per-map warp
data, belong to a later, evidence-based step. Actually switching the
active map on a resolved warp is `LGGameService`'s job (an optional
caller-supplied `LGMapEntry` table), not this module's -- see
SCRIPT-VM.md's "Game service integration" section.

## Input-driven movement (`lg/input_control.h`)

Until now the only way to move `LGPlayer` at all was a script's `OP_MOVE`.
`lg_overworld_input_step(player, map, input)` bridges platform.h's portable
`LgInputState` to `lg_player_step()`: on a newly pressed direction
(`input->pressed`, not `held` -- so holding a button doesn't repeat a step
every single frame with nothing pacing it), it takes exactly one grid
step, using up/down/left/right priority when more than one direction is
pressed in the same frame. This is a simple placeholder policy, not
verified LeafGreen input handling (no turn-then-walk on the first press,
no running/biking) -- real semantics need ROM evidence, same caveat as
everywhere else in this module. See `tests/host/test_input_control.c`.

## Object events (`LGObjectEvent`)

`lg_object_event_facing(player, events, count)` finds the static
NPC/sign/object standing on the tile the player is facing (one step from
`player->x/y` toward `player->facing`, the same values `lg_player_step()`
assigns), or `NULL` if nothing is there. This is how a caller finds what
an "interact" button press should trigger; it does not decide when to
call it or run any script. `LGObjectEvent.script_id` is a portable
indirection into a caller's own script table (the same idea as
`LGScriptTextFn`'s `text_id`), not a LeafGreen object-event id -- no ROM
evidence maps real NPC/event ids yet.

## Interact orchestrator (`lg/interact.h`)

`lg_overworld_try_interact()` is the last piece connecting free-roam
input to scripted events: on a newly pressed A button, if the player is
facing an `LGObjectEvent` and a caller-supplied `LGInteractScriptLookupFn`
resolves its `script_id` to bytecode, it starts that script
(`lg_script_init()`) and binds an `LGGameService` to it
(`lg_game_service_bind()`) -- the caller then drives it with `LGGameLoop`
from the next frame on, same as any other script. No press, no facing
event, or a `script_id` the lookup doesn't know are all
`LG_INTERACT_NONE` (not errors -- a decorative object with no script yet
is not a bug); only invalid arguments are `LG_INTERACT_ERROR`. See
`tests/host/test_interact.c`.

This is preparatory work for LGPSX-012 through LGPSX-018; it is not marked complete until converted real M0 maps render and behave correctly.

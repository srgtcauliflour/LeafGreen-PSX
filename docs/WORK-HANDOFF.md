# Work handoff

## Current milestone
M0 — Pallet Town. Continue from `test/m0-emulator`, based on the still-open
foundation PR #1 (`bootstrap/m0-foundation`); `main` is not yet the foundation.

## Verified in Work on 2026-09-15

- User-provided ROM verified: 16,777,216 bytes, BPGE, revision 1,
  SHA-1 `7862c67bdecbe21d1d69ce082ce34327e1c6ed5e`.
- `python tools/check_host.py`: 12 Python tests, five C test executables,
  example-manifest validation and strict portable C compilation all pass.
- Fixed undefined evaluation order in script VM little-endian operand reads;
  added high-byte, arithmetic-wrap, truncated-input and variable-index checks.
- Moved boot display, GPU synchronization and debug HUD behind the PS1 platform
  boundary; `main.c` now compiles without PS1 headers.
- Fixed controller buffer signedness against the real SDK and corrected the
  extra literal backslash in SYSTEM.CNF's boot path to match the SDK template.
- Built and linked using official PSn00bSDK v0.24 Linux, GCC 12.3.0,
  CMake 4.4 and Ninja; mkpsxiso 2.03 successfully generated BIN/CUE.
- Final compilation emits no project compiler warnings. CMake's initial compiler
  probe emits an SDK/GNUInstallDirs developer warning with CMake 4.4.
- ELF static section sizes: text 16,944, data 2,148, BSS 5,124 bytes.
  These are not peak runtime memory measurements.
- Restored the missing `docs/AGENT-GUARDRAILS.md` onboarding document.

## Next commands and runtime gate

```sh
python tools/check_host.py
cmake --preset default
cmake --build build
```

See BUILD-LOCAL.md for SDK environment setup. Boot `build/leafgreen_psx.cue`
in a PS1 emulator/debugger or compatible hardware. Record BIOS-to-executable
boot, visible debug HUD, stable frame count and controller held/pressed/released
transitions. HLE emulator runtime is now validated below; retail BIOS and hardware remain unverified.
No LGPSX-010+ acceptance item is newly complete.

## Font progress (2026-09-15)

Normal Latin font bytes and widths were fully matched against the verified ROM.
`tools/font/extract_font.py` now extracts a checked 256-glyph atlas locally;
`src/game/text.c` provides bounded proportional layout and a diagnostic ASCII
adapter. An optional PS1 textured-quad demo is available. See TEXT-RENDERER.md.

```sh
python tools/font/extract_font.py /path/to/leafgreen.gba
cmake --preset default -DLGPSX_FONT_DEMO=ON
cmake --build build
```

15 Python tests and six C test programs pass. Font-enabled and asset-free PS1
builds compile/link and generate BIN/CUE. The font-enabled diagnostic now runs
in PCSX-ReARMed with HLE BIOS: 300 frames, steady counter increments, Cross/A
press/hold/release, and readable HUD plus extracted-font text. Captures confirm
pressed/released flags. See PS1-RUNTIME-VALIDATION.md for the pinned core,
observations, repeat command and remaining limitations.

Next: portable dialogue/control-code state handling, followed by PS1 window
rendering. Real BIOS/hardware, full text semantics and long-duration timing are
still unverified. LGPSX-010 remains open for full text behaviour.
The example general extraction manifest still has placeholder offsets; only
the dedicated font descriptor is verified. No map data is mapped yet.

## Dialogue state machine (2026-09-16)

`include/lg/dialogue.h` and `src/game/dialogue.c` add a resumable,
host-tested portable layer on top of the existing glyph layout: it lays out
ordinary glyphs and `FE` newlines exactly like `lg_text_layout`, but pauses
at any `F7`-`FD` control byte and reports `LG_DIALOGUE_WAIT` instead of
rejecting the string. A caller (eventually the script VM/window renderer)
resolves the control (wait for button, scroll, ...) and calls
`lg_dialogue_resume()` to skip that single byte and continue. Multi-byte
control operands are not yet handled: no verified ROM evidence establishes
their length or exact meaning, so only single-byte control bytes are
supported for now. This is scaffolding for LGPSX-011, not a finished
dialogue/window renderer; PS1 window rendering using the tested font
backend is still pending, and no PS1/emulator runtime evidence has been
collected for this module yet.

## Window advance-gating (2026-09-16)

`include/lg/window.h` / `src/game/window.c` add `LGWindowState`, a thin
layer over `LGDialogueState` that gates each control-byte pause behind an
explicit per-frame `advance_pressed` signal (e.g. a confirmed button
press), so a caller can poll it once per frame, show a waiting-for-input
indicator while `LG_WINDOW_AWAIT_ADVANCE` holds, and only resume layout
once the player has acknowledged it. Host-tested; no drawing of any kind.

`LGDialogueState`/`LGWindowState` also gained an optional `wrap_width`
parameter (0 disables it): character wrapping that breaks a line before
whichever glyph would exceed the bound, measured from the starting x. It
never breaks the first glyph on a line even if that glyph alone exceeds
the bound, and it is character wrapping only, not word wrapping — no
lookahead groups glyphs into words yet. See TEXT-RENDERER.md.

`LGWindowState` also gained `max_lines` (0 disables it): once a glyph would
start one line beyond that bound, it is withheld and `lg_window_step`
reports `LG_WINDOW_AWAIT_SCROLL` until the caller signals an advance, then
the box resets to its first line and continues, mirroring the same gating
pattern already used for control bytes. This is the LeafGreen-style
fill-a-box-then-scroll behaviour, gated the same way as `LG_WINDOW_AWAIT_ADVANCE`.

This portable half of LGPSX-011 is now in place. What remains, and needs a
session with the PSn00bSDK toolchain and a PS1 emulator/hardware (neither
is available in this sandbox), is the actual PS1 window renderer: a
background/border box plus the already-validated font backend driven by
`LGWindowState`, then an emulator run recording it, the same two-step
pattern used for the font demo (compiled first in PR #3, validated in
PR #4). Do not mark LGPSX-011 complete until that PS1-side rendering
exists and has runtime evidence.

## Script VM text callback (2026-09-17)

`LGScriptVM` (LGPSX-019 scaffold) gained `OP_TEXT` plus
`lg_script_set_text_fn()`: it calls a registered portable callback with a
1-byte text id and blocks (`LG_SCRIPT_BLOCKED`) until the caller resolves
it with `lg_script_unblock()`, rather than auto-resuming like the existing
one-frame `OP_WAIT`. This is the callback/blocking machinery a real event
system needs to drive `LGWindowState`-based dialogue from a script, not a
mapping of real LeafGreen bytecode (`text_id` is our own indirection). See
SCRIPT-VM.md.

`LGScriptVM` also gained `OP_MOVE`/`lg_script_set_move_fn()`: same
blocking pattern as `OP_TEXT`, but for a single grid step, validated
against the same dx/dy shape `lg_player_step()` accepts.

## Script VM warp/flags and overworld wiring (2026-09-17)

`LGScriptVM` gained `OP_WARP`/`lg_script_set_warp_fn()` (same blocking
pattern, resolved against a warp id table) and 256 pure-VM-state flags via
`OP_FLAG_SET`/`OP_JUMP_IF_FLAG` (no callback needed -- see SCRIPT-VM.md).

More importantly, `OP_MOVE`/`OP_TEXT`/`OP_WARP` are now actually wired to
the real overworld model: `include/lg/service.h` + `src/game/service.c`
add `LGGameService`, bound onto a VM with `lg_game_service_bind()`. It
resolves `OP_MOVE` through `lg_map_can_enter()`/`lg_player_step()` and
`OP_WARP` against a caller-supplied `LGWarp` table synchronously (both
unblock immediately -- no multi-frame slide or async CD warp exists yet),
while `OP_TEXT` only records the requested id for the caller's own
`LGWindowState`/font backend to resolve. `tests/host/test_service.c` runs
a real script (move, request text, request warp) end to end against a
live `LGMap`/`LGPlayer`, including rejection of a blocked move and an
unknown warp id.

This closes the callback/VM-state side of LGPSX-019's scaffolding, but it
is still scaffolding: no real M0 script bytecode, converted map data, font
widths, or PS1 runtime evidence exist behind any of it yet, so LGPSX-012
through LGPSX-019 remain open until those do.

## Per-frame game loop (2026-09-18)

`include/lg/loop.h` + `src/game/loop.c` add `LGGameLoop`, the piece above
`LGGameService` that drives a script across real frames instead of one
opcode call at a time: `lg_game_loop_step(loop, advance_pressed)` steps an
open `LGWindowState` if one exists, or steps the VM once and resolves
whatever it blocked on -- `OP_MOVE`/`OP_WARP` immediately (a warp also
applies the resolved destination x/y to the player, but does not swap the
active map, since that needs real resource loading this scaffolding
doesn't have), and `OP_TEXT` by looking up the id's byte span through a
caller-supplied `LGLoopTextLookupFn` and opening a real window, resolving
only once it reports `LG_WINDOW_DONE`.

`tests/host/test_loop.c` runs a full multi-frame script end to end (move
onto a warp tile, open and finish a dialogue window, take the warp, hit
`OP_END`) against a live map/player and a synthetic font widths/sink/text
lookup, plus loop-level error cases. This is the closest thing to a
"working" M0 loop this scaffolding can demonstrate without a PSn00bSDK
build and real ROM assets: widths/sink/text data are all caller-supplied
so it stays host-testable, but it is still scaffolding, not a completed
LGPSX-012 through LGPSX-020 -- those still need real converted map/script
data, a real font backend, and PS1/emulator runtime evidence.

## M0 save payload (2026-09-18)

`include/lg/save_game.h` + `src/game/save_game.c` add `LGSaveGamePayload`
(player x/y/facing plus the 256 script flags) as the first concrete
schema behind the `LGSV` envelope reserved in save.h -- SAVE-FORMAT.md
had explicitly deferred this until game-state models existed to
serialize, and now `LGPlayer`/`LGScriptVM.flags` do. `lg_save_game_write()`/
`lg_save_game_read()` round-trip it through a header with save
id/size/CRC32, rejecting any truncation, magic/version/size mismatch or
corruption. `tests/host/test_save_game.c` covers the round trip and every
rejection case. Still M0-only (no inventory/party/etc.) and in-memory
only -- no PS1 memory-card I/O exists yet.

## Input-driven overworld movement (2026-09-18)

Until now the only way to move `LGPlayer` at all was a script's
`OP_MOVE`; there was no free-roam walking. `include/lg/input_control.h` +
`src/overworld/input_control.c` add `lg_overworld_input_step(player, map,
input)`, bridging platform.h's portable `LgInputState` to
`lg_player_step()`: a newly pressed direction (`pressed`, not `held`, so
nothing repeats every frame without a timing model) takes exactly one
grid step, with up/down/left/right priority when multiple directions are
pressed at once. This is a placeholder input policy, not verified
LeafGreen behaviour (no turn-then-walk, no running/biking) -- see
OVERWORLD.md. `tests/host/test_input_control.c` covers pressed vs. held,
priority, a blocked direction (turns without moving, matching
`lg_player_step()`'s own contract) and NULL args.

## Warp map switching (2026-09-18)

`LGGameService` gained `lg_game_service_set_map_table()` (`LGMapEntry`:
`map_id`, `map`, `warps`, `warp_count`). A resolved `OP_WARP` already
applied the matched `LGWarp`'s destination x/y to the player; now, if the
warp's `dest_map` matches a registered table entry, it also switches
`svc->map`/`svc->warps` to that entry's, so a subsequent `OP_MOVE`/`OP_WARP`
acts against the destination map. This is a caller-supplied, statically-
known table lookup, not resource loading -- a `dest_map` with no table
entry still moves the player but leaves the map unchanged. `loop.c`
simplified accordingly (it no longer special-cases warp position, since
the service already applies it). `tests/host/test_service.c` covers both
the matched and unmatched cases. See SCRIPT-VM.md.

## Object events (2026-09-18)

`LGObjectEvent` (`x`, `y`, `script_id`) plus `lg_object_event_facing()`
add the missing piece for NPC/sign interaction: it finds the event on the
tile the player is facing (one step in the direction of `player->facing`),
so a caller can wire an "interact" button press to starting a script.
`script_id` is our own indirection into a caller's script table, not a
LeafGreen event id. It only finds the event; it doesn't decide when to
call it or run anything. `tests/host/test_overworld.c` covers all four
facings, a tile with nothing on it, an unrecognised facing value, and
NULL args.

## Resource byte-span resolution (2026-09-18)

`lg_resource_bytes(index, id, expected_kind, buffer, buffer_size,
&out_size)` (resource.h/c) resolves an id to a byte span within a
caller-loaded buffer: `NULL` if not found, the wrong kind, or the entry's
offset/size doesn't fit entirely within the buffer (never a partial span
from an overflowing or malformed entry). This is the piece that turns the
resource index from a lookup table into something a caller can actually
hand data through -- e.g. `LGGameLoop`'s `LGLoopTextLookupFn` -- once real
bundle data and CD loading exist. `tests/host/test_resource.c` covers the
happy path, wrong kind, missing id, an exact-fit boundary, one byte over,
an offset alone past the buffer, and invalid arguments.

## Interact orchestrator (2026-09-18)

`include/lg/interact.h` + `src/game/interact.c` add
`lg_overworld_try_interact()`, the last piece connecting free-roam input
to scripted events: on a newly pressed A button, if the player is facing
an `LGObjectEvent` (`lg_object_event_facing()`) and a caller-supplied
`LGInteractScriptLookupFn` resolves its `script_id` to bytecode, it starts
that script (`lg_script_init()`) and binds an `LGGameService` to it
(`lg_game_service_bind()`) -- the caller drives it with `LGGameLoop` from
the next frame on. No press, no facing event, or an unknown `script_id`
are all `LG_INTERACT_NONE`, not errors; only invalid arguments are
`LG_INTERACT_ERROR`. `tests/host/test_interact.c` covers all of these,
including that the started script actually runs.

This closes the chain built across this session: free-roam movement
(`lg_overworld_input_step`) -> facing an object
(`lg_object_event_facing`) -> starting its script
(`lg_overworld_try_interact`) -> driving it frame by frame
(`LGGameLoop`/`LGGameService`) -> dialogue/warps/flags within it
(`LGScriptVM`) -> persisting the result (`LGSaveGamePayload`). All of it
remains scaffolding until real M0 map/script/text data, a real font
backend, and PS1/emulator runtime evidence exist -- see the acceptance
gates in M0-ACCEPTANCE.md and M0-PALLET-TOWN.md, none of which this
closes on its own.

## Overworld runner (2026-09-18)

`include/lg/runner.h` + `src/game/runner.c` add `LGOverworldRunner`: the
one function a real per-frame game loop actually calls, since nothing
until now switched between free-roam input and a running script.
`lg_overworld_runner_step(runner, input, advance_pressed)` takes free-roam
movement input while idle, tries an interact first (starting a script on
a newly pressed A facing a scripted object), then once a script is
running drives it via `LGGameLoop` each frame until `LG_LOOP_DONE`
(`LG_RUNNER_STEP_SCRIPT_DONE`, back to idle) or `LG_LOOP_ERROR`
(`LG_RUNNER_STEP_ERROR`, also back to idle -- one broken script must not
wedge the whole overworld). The interact check, and the requirement that
`script_lookup_fn` be non-NULL, is skipped entirely when `event_count` is
0, so a runner for an event-free map needs no script lookup at all.
`tests/host/test_runner.c` exercises the full idle -> interact -> script
-> idle cycle plus an unresolvable script id, a runner with no events,
and invalid arguments.

This is the actual top-level entry point everything else in this
session's chain was built to serve; it is still scaffolding for the same
reasons as everything upstream of it.

## Save capture/apply (2026-09-18)

`lg_save_game_capture(player, vm, out)` / `lg_save_game_apply(payload,
player, vm)` (save_game.h/c) connect `LGSaveGamePayload` to the live
`LGPlayer`/`LGScriptVM` structs directly: capture fills a payload from a
player's x/y/facing and a VM's 256 flags, apply writes them back. Apply
only touches x/y/facing/flags, never `vm->pc`/`status`/`code` (resuming
mid-script from a save isn't part of this schema). Both are no-ops on any
NULL argument, never a partial copy. `tests/host/test_save_game.c` now
round-trips real player/VM state (via a script that sets a flag) through
capture -> write -> read -> apply, and checks every NULL-argument no-op.

## Object events follow map switches (2026-09-18)

`LGMapEntry` (service.h) gained `events`/`event_count`: the destination
map's own `LGObjectEvent` table, alongside the `map`/`warps` it already
carried. `LGOverworldRunner` (runner.c) uses this: after a script's warp
switches `service->map` via a registered table entry, once that script
finishes (`LG_RUNNER_STEP_SCRIPT_DONE`/`_ERROR`) the runner's own
`events`/`event_count` follow the matching entry's, so interacting stays
correct for whichever map is now active instead of still offering to talk
to NPCs on the map the player just left. If no table entry matches the
new map, events are left as they were -- the same "leave unresolved
rather than guess" choice `LGGameService` itself makes for `map`/`warps`
in that situation. `tests/host/test_runner.c` covers a full script-warp
round trip: the service's map switches immediately when the warp
resolves, but the runner's events only follow once the script actually
finishes.

## Script VM item callback (2026-09-18)

`LGScriptVM` gained `OP_ITEM`/`lg_script_set_item_fn()`: same blocking
pattern as `OP_TEXT`/`OP_MOVE`/`OP_WARP`, passing an item id and a 16-bit
quantity. This completes the exact list SCRIPT-VM.md named from the
start -- "text, movement, flags, warps, items" -- but has no
`LGGameService` wiring yet, since no inventory model exists anywhere in
this codebase; that's expected to arrive the same way `OP_TEXT`'s did,
in a later PR once there's something real to wire it to.
`tests/host/test_script.c` covers block/unblock, a rejecting callback, a
missing callback and a truncated operand.

## Movement animation hint (2026-09-18)

`LGPlayer.moving` had sat unused since M0's very first commit.
`lg_player_step()` now sets it to the new `LG_PLAYER_SLIDE_FRAMES`
whenever a step actually moves the player (not when it only turns to face
a blocked direction); the new `lg_player_animate_tick(player)` counts it
down by one, once per frame, never below 0. This is purely a presentation
hint for a future renderer to interpolate sprite position during a
tile-slide -- `x`/`y` already hold the destination the instant the step
succeeds, and nothing paces or blocks on `moving` being nonzero (adding
that would conflict with the one-opcode-per-frame pacing scripts already
rely on, tested across `test_service.c`/`test_loop.c`/`test_runner.c`).
`LG_PLAYER_SLIDE_FRAMES` is a placeholder duration with no
ROM/hardware-verified LeafGreen movement timing behind it yet.
`tests/host/test_overworld.c` covers set-on-move, unset-when-blocked,
counting down, and never underflowing past 0.

## Window typewriter reveal pacing (2026-09-18)

`LGWindowState` gained `reveal_per_step` (window.h/c): text in a box
previously appeared all at once in a single `lg_window_step()` call,
which no RPG actually does. A positive value paces drawing to that many
glyphs per call; unlike `AWAIT_ADVANCE`/`AWAIT_SCROLL`, pausing for it
needs no caller acknowledgement -- it reports plain `LG_WINDOW_RUNNING`
and resumes from the paused glyph on the next call. The actual speed
(frames per glyph) is entirely up to how often/how the caller invokes
`lg_window_step()`; no LeafGreen-verified typing speed is claimed.
`LGGameLoop` gained a matching `text_reveal_per_step` forwarded to every
window it opens for a script's `OP_TEXT` (0 keeps the prior instant
behavior). `tests/host/test_window.c` covers a multi-call reveal
sequence, including the last glyph and the terminator landing in the
same call once nothing is left to pause on.

## Inventory + OP_ITEM service wiring (2026-09-18)

`OP_ITEM` (added last session) had no `LGGameService` wiring yet -- there
was no inventory model anywhere in this codebase. `include/lg/inventory.h`
+ `src/game/inventory.c` add `LGInventory`: a generic fixed-slot bag
(`lg_inventory_add`/`_remove`/`_count`), not verified LeafGreen inventory
data -- real bag pockets, capacity, key items and stacking rules all
still need ROM evidence.

`LGGameService` gained `lg_game_service_set_inventory(svc, inv)` (pass 0
to leave `OP_ITEM` unsupported, same as any other optional callback) and
now binds an item callback too: a resolved `OP_ITEM` calls
`lg_inventory_add()` and records `has_pending_item`/`pending_item_id`/
`pending_item_quantity`, mirroring how `OP_TEXT`/`OP_WARP` record their
own pending state. A missing inventory or a full bag is a script error,
never a silently discarded item. `tests/host/test_inventory.c` covers the
bag model directly (claiming slots, topping up, clearing on removal,
overflow, invalid args); `tests/host/test_service.c` covers the
`OP_ITEM`-through-`LGGameService` wiring and its error paths.

## Save payload gains inventory (2026-09-18)

`LGSaveGamePayload` (`lg/save_game.h`) previously only covered player
position/facing and script flags; the doc comment already promised it
would grow as more M0 state got ported. It now embeds a fixed
`LG_SAVE_INVENTORY_SLOTS` (20, our own placeholder, not a verified
LeafGreen bag size) array of `LGInventorySlot`, bumping `LG_SAVE_VERSION`
from 1 to 2.

`lg_save_game_capture`/`lg_save_game_apply` both gained an `LGInventory *`
parameter and now copy its slots to/from the payload alongside
player/flags. Both stay "never partial": a NULL inventory, a NULL
`inventory->slots`, or an `inventory->slot_count` that doesn't exactly
equal `LG_SAVE_INVENTORY_SLOTS` makes the whole call a no-op, same as any
other invalid argument here -- a save can only round-trip an inventory
shaped exactly like its own schema. `tests/host/test_save_game.c` covers
the inventory round-trip and the mismatched-slot-count no-op case
alongside the existing player/flags/corruption coverage.

## OP_ITEM_TAKE opcode + service wiring (2026-09-18)

`lg/inventory.h` already had `lg_inventory_remove()`, but nothing in the
script VM or `LGGameService` called it -- `OP_ITEM` (and its service
wiring) only ever added items. Added `OP_ITEM_TAKE`: same 1-byte item id
+ 16-bit little-endian quantity operand shape as `OP_ITEM`, but resolved
through a separate `LGScriptVM.item_take_fn`/`item_take_context` pair
(registered via `lg_script_set_item_take_fn`), so a script can consume a
key item, pay an item cost, or complete a trade, not just receive items.

`LGGameService` wires it the same way `OP_ITEM` is wired: a resolved
`OP_ITEM_TAKE` calls `lg_inventory_remove()`, recording the outcome in
new `has_pending_item_take`/`pending_item_take_id`/
`pending_item_take_quantity` fields (kept separate from `OP_ITEM`'s own
pending fields so a pending grant and a pending removal never collide).
A missing inventory or removing more than the bag holds is a script
error, same as any other unresolvable service call.
`tests/host/test_script.c` covers the new opcode/callback at the VM
level; `tests/host/test_service.c` covers the `LGGameService` wiring and
its error paths.

## Elevation-gated movement (2026-09-18)

`LGMapCell.elevation` existed since M0's first commit but nothing ever
read it -- OVERWORLD.md already flagged "elevation interactions" as
unfinished. Added `lg_map_can_enter_from(map, from_x, from_y, to_x, to_y)`
(overworld.h/c): elevation 0 is a wildcard on either end, otherwise the
source and destination cells' elevation must match exactly, on top of
the existing bounds+collision check. `lg_player_step()` now uses it
instead of the plain `lg_map_can_enter()`, so an elevation-incompatible
step turns-without-moving, same as a collision-blocked one.

`LGGameService`'s `service_move` (the `OP_MOVE` backing) was updated to
pre-check with `lg_map_can_enter_from()` too, instead of the plain
check -- otherwise a script's `OP_MOVE` could think a move succeeded
(and block/unblock as if it had) while `lg_player_step()` silently
refused it internally for an elevation mismatch, letting the two paths
disagree. This is our own generic placeholder rule, not verified
LeafGreen elevation behaviour. `tests/host/test_overworld.c` covers the
new function directly and `lg_player_step()`'s elevation-blocked case.

## Tile-triggered warps (2026-09-18)

`lg_map_warp_at()` and `OP_WARP` both already existed, but nothing fired
a warp from free-roam movement alone -- OVERWORLD.md's warps section
explicitly deferred "a specific tile" as a later policy decision.
Extracted the warp-application logic `service_warp` (the `OP_WARP`
callback) already had into a new public `lg_game_service_apply_warp(svc,
warp)` (service.h/c), so it can be called from outside a script too.

`LGOverworldRunner`'s free-roam step now checks the cell the player just
moved onto via `lg_map_warp_at()`; if it resolves a warp, the runner
applies it immediately and returns a new `LG_RUNNER_STEP_WARPED` result
(added to `LGRunnerStepResult`) instead of `LG_RUNNER_STEP_IDLE`,
re-syncing `events`/`event_count` to the destination map the same way a
script-driven warp already does. `tests/host/test_runner.c` covers
stepping onto a warp tile (including following a registered
`LGMapEntry`) and that a plain tile still returns idle.

## Ledges (2026-09-18)

`LGMapCell` gained a `ledge` field (0 = none, else `facing + 1` for the
one direction it can be jumped in), closing another item OVERWORLD.md's
opening paragraph flagged as unfinished. `lg_map_can_enter_from()` now
refuses entering a ledge cell from the wrong direction (elevation isn't
considered for a ledge cell at all); `lg_player_step()` uses the same
check and, on a matching step, jumps straight over the ledge cell onto
the cell beyond it (skipping it as a landing spot), refusing the whole
step if that landing cell isn't enterable. `moving` is set to twice
`LG_PLAYER_SLIDE_FRAMES` for a ledge jump. `LGGameService`'s
`service_move` needed no change -- it already pre-checks with
`lg_map_can_enter_from()`, so it can't disagree with `lg_player_step()`
about a ledge, same as it already couldn't about elevation.
`tests/host/test_overworld.c` covers the jump, wrong-direction entry,
and a blocked landing cell.

## Running (2026-09-18)

`lg_overworld_input_step()`'s doc comment flagged "no running/biking" as
a gap. Added `lg_player_run_step()` (overworld.h/c): identical rules to
`lg_player_step()`, but a successful step sets `LGPlayer.moving` to a
new `LG_PLAYER_RUN_SLIDE_FRAMES` (half of `LG_PLAYER_SLIDE_FRAMES`)
instead, purely a faster presentation hint -- both functions now share
one internal `player_step_at_speed()` helper so the collision/elevation/
ledge logic isn't duplicated. `lg_overworld_input_step()` calls the run
variant while `LG_BUTTON_B` is held. There is still no biking, and no
ROM-verified running speed backs the new constant.
`tests/host/test_overworld.c` covers `lg_player_run_step()` directly
(including a running ledge jump); `tests/host/test_input_control.c`
covers the held-B path through `lg_overworld_input_step()`.

## OP_CHOICE opcode + service wiring (2026-09-18)

M0-ACCEPTANCE.md's "New game path" section requires "Oak intro
progresses through required dialogue/choices", but nothing in the
script VM could present a choice at all. Added `OP_CHOICE`
(script.h/vm.c): reads a 1-byte choice-prompt id (portable indirection,
same idea as `OP_TEXT`'s `text_id`) and a 1-byte var index (bounds-
checked against the VM's 32 vars before the new `LGScriptChoiceFn`
callback runs), then blocks like `OP_TEXT` -- the caller drives its own
menu/window, writes the selected option into `vm->vars[var_index]`
itself once the player confirms, and only then unblocks. There is no
"jump if var equals" opcode yet, so branching on the result is left to
the script/caller for now; this establishes the blocking mechanism, not
a full menu/branching system.

`LGGameService` wires it the same way as `OP_TEXT`: new
`has_pending_choice`/`pending_choice_id`/`pending_choice_var` fields
record the request without deciding when it's resolved.
`tests/host/test_script.c` covers the opcode/callback (including the
out-of-range var index being rejected before the callback ever runs);
`tests/host/test_service.c` covers the `LGGameService` wiring.

## OP_JUMP_IF_VAR opcode (2026-09-18)

`OP_CHOICE`'s own docs (last cycle) noted there was no way yet for a
script to branch on the selected option it writes into a var. Added
`OP_JUMP_IF_VAR` (index, 16-bit little-endian value, little-endian
absolute address): the same idea as the existing `OP_JUMP_IF_FLAG`, but
compares a full `vars[index]` value instead of a single flag bit. Same
rules apply -- the var index and jump target are both bounds-checked up
front regardless of whether the branch is taken, and a jump only moves
`pc` (the target instruction executes on the following step).
`tests/host/test_script.c` covers the taken/not-taken cases and all
three error paths (bad var index, bad jump target, truncated operand),
mirroring the existing `OP_JUMP_IF_FLAG` coverage.

Keep ROMs, BIOS files and generated proprietary assets outside Git. Memory-card
trading remains M10; GBA network/link emulation remains excluded.

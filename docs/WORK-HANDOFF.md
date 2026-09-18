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

Keep ROMs, BIOS files and generated proprietary assets outside Git. Memory-card
trading remains M10; GBA network/link emulation remains excluded.

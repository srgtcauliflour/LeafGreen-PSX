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

Keep ROMs, BIOS files and generated proprietary assets outside Git. Memory-card
trading remains M10; GBA network/link emulation remains excluded.

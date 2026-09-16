# Work handoff

## Current milestone
M0 — Pallet Town. Continue from `feat/m0-text`, based on the still-open
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
transitions. Neither emulator nor hardware runtime has been validated here.
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
builds compile/link and generate BIN/CUE. Host font preview inspected; native
rendering remains unverified. No emulator binary is installed in this workspace;
PCSX-Redux's GitHub latest-release endpoint returned 404 and its official Linux
download page requires additional setup. No retail BIOS has been supplied.

Next: boot the font-enabled CUE, check the five diagnostic lines below the HUD,
controller input and timing; fix any GPU/runtime issues before claiming LGPSX-010.
Then add explicit dialogue control-code handling and window rendering (LGPSX-011).
The example general extraction manifest still has placeholder offsets; only
the dedicated font descriptor is verified. No map data is mapped yet.

Keep ROMs, BIOS files and generated proprietary assets outside Git. Memory-card
trading remains M10; GBA network/link emulation remains excluded.

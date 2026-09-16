# PS1 emulator validation — 2026-09-15

## Result

The font-enabled diagnostic CUE loads and executes under PCSX-ReARMed with its
HLE BIOS. A 300-frame headless run passed counter and input-state checks.
Screenshots were inspected: the SDK HUD and five lines using the extracted
LeafGreen normal Latin font are readable, with transparent glyph backgrounds.
This advances LGPSX-010's native rendering evidence; it does not finish the full
LeafGreen text/control system or the M0 gameplay path.

## Environment and evidence

- Runtime source: `a81a482182d25647ecdb8bb65839d798d00d86e0` (PR #3).
- Toolchain: PSn00bSDK v0.24, GCC 12.3.0, font demo enabled.
- Emulator: [PCSX-ReARMed](https://github.com/libretro/pcsx_rearmed), commit
  `8625c395a24411f8c77e69802b516df9c613a712`.
- Core: Linux x86_64, interpreter (`DYNAREC=`), software GPU, `HAVE_CHD=0`.
- Frontend: `tools/runtime/libretro_smoke.c`, software RGB565, 320x240 captures.
- BIOS: built-in high-level emulation, using an empty system/save directory.
  No retail BIOS, GBA runtime emulation or extracted BIOS was used.
- The CUE references the native PS1 BIN containing the generated font demo.

Observed states (frontend frame numbers are zero-based):

| Frame | Game counter | Held buttons | Observation |
| --- | --- | --- | --- |
| 120 | 120 | `0000` | Idle; font visible |
| 181 | 181 | `0010` | PS1 Cross / game A pressed |
| 199 | 199 | `0010` | A held |
| 202 | 202 | `0000` | A released |
| 299 | 299 | `0000` | Counter continues |

The capture at frontend frame 181 shows game frame 180 with `pressed=0010`;
frame 182 shows the flag cleared while held remains set. Frame 201 shows game
frame 200 with `released=0010` and held/pressed both zero. This one-frame display
lag is consistent with the existing double-buffer presentation.
The automated runner checks every counter increment after frame 120 and the
held states above. Edge flags and glyph appearance were verified visually.

## Repeat

Build the external core in a directory outside this repository:

```sh
git clone https://github.com/libretro/pcsx_rearmed.git pcsx_rearmed
git -C pcsx_rearmed checkout 8625c395a24411f8c77e69802b516df9c613a712
make -C pcsx_rearmed -f Makefile.libretro -j4 platform=unix DYNAREC= HAVE_CHD=0
```

Generate the font and compile the game as described in TEXT-RENDERER.md. Then:

```sh
python tools/runtime/check_ps1.py \
  --core /absolute/path/pcsx_rearmed/pcsx_rearmed_libretro.so \
  --headers /absolute/path/pcsx_rearmed/deps/libretro-common/include \
  --nm /absolute/path/PSn00bSDK/bin/mipsel-none-elf-nm
```

Requires Linux, Python 3 and `cc`. The matching unstripped game ELF and CUE default
to `build/leafgreen_psx.elf` and `.cue`; override with `--elf` and `--cue`.
Use a new empty `--output` directory for each run. The wrapper extracts symbol
addresses from the ELF, strictly compiles the frontend, limits runtime to 45
seconds, records a log and hashes, and writes nine PPM frames. Do not provide a
mismatched ELF/CUE pair. Inspect captures manually; successful frame/state checks
alone do not prove glyph correctness. No claim of broad libretro-core support.

Captures include proprietary font pixels. Keep them and font-enabled images
under ignored build output; never commit them. The runner and documentation are
original source only. It makes no network requests and requires no user accounts.

## Remaining gates

- Real BIOS CD startup and original-hardware behaviour.
- More controllers, unplug/reconnect and all button mappings.
- Long-duration stability, frame-time and peak RAM/VRAM measurements.
- Full glyph/control-code coverage, extended/Japanese text and dialogue windows.
- Map rendering, movement and the Pallet Town acceptance path.

Next implementation: a portable dialogue state machine that explicitly yields
for wait/scroll controls, then a bounded PS1 window renderer using the now-tested
font backend. Do not mark LGPSX-010 fully complete until its intended text
behaviour is implemented and validated.

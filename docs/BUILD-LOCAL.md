# Local build/run checklist

This is the shortest handoff from repository-only work to an interactive development machine.

## Inputs
- a clone of this repository/branch
- PSn00bSDK toolchain configured for CMake
- the user's exact LeafGreen USA Rev 1 ROM (kept outside Git)
- a PS1 emulator/debugger or compatible hardware for runtime validation

## Verify target
```sh
python tools/romverify/verify_leafgreen.py /path/to/leafgreen.gba
```
The command must report PASS before extraction.

## Host checks
```sh
python tools/check_host.py
```
Requires Python 3 and a C11 compiler (`cc`, or set `CC`). The same command runs
in CI, uses temporary build products, and works from any working directory.

## Asset pipeline
`tools/assets/extract.py` only extracts ranges explicitly listed in a manifest and first verifies the entire ROM SHA-1. The committed example uses placeholder ranges and is **not** an authoritative LeafGreen asset map. Populate real source descriptors only from verified exact-revision reference metadata.

## PS1 build
Verified with the official PSn00bSDK v0.24 Linux release (GCC 12.3.0,
mkpsxiso 2.03), CMake 4.4 and Ninja. Install CMake and Ninja separately and put
the SDK's `bin` directory on PATH; set `PSN00BSDK_LIBS` to its `lib/libpsn00b`.

```sh
cmake --preset default
cmake --build build
```

Outputs: `build/leafgreen_psx.exe`, `.elf`, `.map`, `.bin` and `.cue`.
The current diagnostic build does not need the ROM and contains no extracted
LeafGreen assets. Load the CUE file to test the CD boot path.

Use the installed PSn00bSDK CMake toolchain to configure/build the repository. Do not silently substitute a host compiler for the PS1 target. Capture the first compiler/linker error exactly if the current skeleton needs SDK-version adjustments.

### Building the toolchain from source (`tools/psn00bsdk/setup.sh`)
Where a prebuilt PSn00bSDK release isn't reachable (e.g. no access to
GitHub release assets, only plain git/HTTPS), `tools/psn00bsdk/setup.sh`
builds a `mipsel-none-elf` GCC/binutils toolchain from source per
PSn00bSDK's own `doc/toolchain.md`, then builds and installs PSn00bSDK
itself. It's idempotent (each stage skips if its output already exists)
and installs to `/opt/mipsel-none-elf` and `/opt/psn00bsdk` by default.
Ubuntu's packaged `mipsel-linux-gnu` cross-compiler is **not** a
substitute -- PSn00bSDK explicitly does not support `mipsel-linux-gnu`
targets, only bare-metal `mipsel-none-elf`/`mipsel-unknown-elf`.
`.claude/hooks/session-start.sh` runs this automatically (async) for
Claude Code on the web sessions.

This path has been exercised end to end (GCC 12.2.0 self-built, binutils
2.40, current PSn00bSDK `main`): `leafgreen_psx.elf`/`.exe`/`.bin`/`.cue`
build cleanly with no compiler warnings (a `-Wsign-compare` warning this
GCC version raised in `inventory.c`, not caught by host `cc`, has been
fixed). This confirms the CMake target itself is sound, not that the M0
native-proof/engineering-gate checklist below is met.

`.github/workflows/psx-build.yml` now builds this toolchain and the
real PS1 target on every push/PR (caching the built toolchain across
runs, since building it from source takes 30-60 minutes on a cache
miss), asserting the output really is a `Sony Playstation executable`.
This guards against a future change silently breaking the PS1 build
while only `tools/check_host.py`'s host checks (a separate,
faster-running workflow) are being watched.

## Runtime gate
Boot the generated PS-EXE/BIN-CUE and record: whether BIOS->executable succeeds, visible framebuffer output, controller detection, frame stability and any emulator/debug console output. Those observations are the evidence needed to finish LGPSX-010+ rather than guessing at GPU/runtime behaviour.

`tools/runtime/check_ps1.py` automates a headless smoke run via an
external libretro core (see its own docstring). The only PS1 core
reachable via this environment's package manager is Beetle PSX
(`libretro-beetle-psx`, Mednafen's PSX core), which -- unlike some other
PS1 cores -- has no HLE BIOS fallback: it refuses to boot without an
actual PS1 BIOS ROM dump supplied at a `psx.bios_*` path under the
core's system directory. No BIOS dump is available or was sourced here
(that is Sony's copyrighted firmware, a separate concern from a game
ROM), so this smoke test currently fails at core init with `Error
opening file No such file or directory`, before ever reaching
`leafgreen_psx`'s own code. This is a real, currently-open gap, not a
build problem -- whoever runs this with their own legally-obtained BIOS
dump (or a core with HLE support) should get past this point.

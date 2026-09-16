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

## Runtime gate
Boot the generated PS-EXE/BIN-CUE and record: whether BIOS->executable succeeds, visible framebuffer output, controller detection, frame stability and any emulator/debug console output. Those observations are the evidence needed to finish LGPSX-010+ rather than guessing at GPU/runtime behaviour.

# Work handoff

This document separates work that can be completed through repository tooling from work requiring an interactive development machine / Work environment.

## Completed remotely

- project architecture and M0 specification
- PSn00bSDK/CMake skeleton
- ROM identity verifier and synthetic tests
- versioned asset manifest contract
- deterministic GBA 4bpp/palette -> PS1 TIM prototype
- host CI for conversion tooling
- text-renderer API/design scaffold
- platform-neutral overworld map/player/collision core and host test
- future memory-card trading architecture goal

## Requires an interactive build/runtime environment

- install/run PSn00bSDK MIPS toolchain and verify PS-EXE link
- build BIN/CUE with mkpsxiso and boot it in an emulator or hardware
- provide the user's local LeafGreen Rev 1 ROM to extraction tools
- build/use exact-revision upstream reference locally where required
- inspect generated proprietary assets without committing them
- validate TIM upload, VRAM/CLUT placement and GPU packet rendering
- capture frame timing/RAM/VRAM measurements
- test controller behaviour and eventual memory-card I/O on PS1-compatible runtime/hardware

## Next local command sequence

1. Clone this branch.
2. Run `python tools/romverify/verify_leafgreen.py <path-to-rom>`.
3. Run host tests from `.github/workflows/host-tools.yml` locally if desired.
4. Configure the project with the installed PSn00bSDK CMake toolchain.
5. Build the PS-EXE and CD image.
6. Boot the image in a PS1 emulator/debugger and record the first runtime failures in an issue/PR.

Do not commit the ROM, extracted assets or generated commercial game data.

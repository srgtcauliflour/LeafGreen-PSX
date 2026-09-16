# Testing strategy

## Host tests

Run `python tools/check_host.py` (also used by CI).
Pure conversion and game logic should compile/run without PS1 hardware. Current CI covers ROM-verifier metadata, asset manifests, GBA colour/tile primitives, TIM generation, grid movement/collision, script-VM bounds/yielding, the resumable dialogue state machine's control-byte pausing and LGTR record integrity.

## Reference tests
Where behaviour depends on LeafGreen, create deterministic fixtures/observations from the user's exact Rev 1 local reference without committing proprietary bulk data. Prefer numeric/state assertions over screenshots or copied asset blobs.

## PS1 integration tests
Validate boot, GPU/CLUT upload, controller, timing, CD reads, SPU and memory card in a suitable emulator/debugger, then periodically repeat milestone gates on original hardware or a hardware-faithful environment.

## M0 golden path
Boot -> title -> New Game -> Oak intro -> player setup -> bedroom -> downstairs -> Pallet Town -> movement/collision -> building transitions. Every regression release should preserve this path once achieved.

## Automated emulator smoke run

See `PS1-RUNTIME-VALIDATION.md` and `tools/runtime/check_ps1.py` for the pinned
headless PCSX-ReARMed HLE run, frame captures and controller checks. This is
separate from asset-free host CI and uses locally generated font assets.

# M0 — Pallet Town

## Acceptance path
PS1 boot -> title -> New Game -> Oak intro -> player setup -> bedroom -> downstairs -> Pallet Town -> movement/collision -> building transitions.

## Work items
- [x] LGPSX-001 — PSn00bSDK/CMake project skeleton.
- [x] LGPSX-002 — Bootable CD image definition.
- [x] LGPSX-003 — Initialise 320x240 PS1 GPU display environment.
- [x] LGPSX-004 — PS1 digital-controller translation layer.
- [x] LGPSX-005 — Frame/VSync timing primitive.
- [x] LGPSX-006 — Portable platform abstraction boundary.
- [ ] LGPSX-007 — Verify local LeafGreen Rev 1 ROM in developer workflow.
- [ ] LGPSX-008 — Define asset manifest/extraction boundary.
- [ ] LGPSX-009 — GBA palette/tile -> PS1 texture conversion prototype.
- [ ] LGPSX-010 — LeafGreen text/font renderer.
- [ ] LGPSX-011 — Dialogue/window renderer.
- [ ] LGPSX-012 — Overworld tile renderer.
- [ ] LGPSX-013 — Bedroom map conversion.
- [ ] LGPSX-014 — Render bedroom.
- [ ] LGPSX-015 — Player sprite rendering/animation.
- [ ] LGPSX-016 — Grid movement.
- [ ] LGPSX-017 — Collision.
- [ ] LGPSX-018 — Map connections/warps.
- [ ] LGPSX-019 — Minimum event/script interpreter required for M0.
- [ ] LGPSX-020 — Reach and walk around Pallet Town on the native PS1 runtime.

## Checkpoint at LGPSX-020
Record executable size, peak main RAM, VRAM allocation, frame time, CD read behaviour, and the proportion of upstream game code/data that is reused versus replaced.

## Definition of native
A PS1 emulator may be used as a development/debugging environment, but LeafGreen-PSX itself must not emulate the GBA CPU or GBA system to execute the game.

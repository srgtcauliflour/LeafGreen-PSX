# Agent guide

## Mission
Build a native original-PlayStation port/demake runtime for the user's exact LeafGreen USA Rev 1 target. Do not solve compatibility by embedding a GBA emulator.

## Non-negotiable boundaries
- Never commit ROMs, BIOS files or extracted commercial game assets.
- Keep portable game logic free of PS1 hardware calls.
- Use the exact target identity documented in README; reject unsupported ROM revisions until explicitly added.
- Treat upstream decompilation as a behavioural/reference source; do not casually copy third-party copyrighted/unlicensed source into this repository.
- GBA link/wireless networking is out of scope. Preserve the architecture needed for future memory-card trading.
- Host-test deterministic logic and conversion code whenever possible.

## Task discipline
Work from `docs/M0-PALLET-TOWN.md`. A checkbox is completed only when its acceptance behaviour exists, not when an interface/stub is created. Prefer small commits with tests. Document assumptions that need verification on PS1 hardware/toolchain.

## Architecture rule
Game modules call platform-neutral interfaces. PS1 GPU/SPU/pad/CD/memory-card details live under the PS1 platform backend. Asset extraction/conversion happens on the host and generated proprietary outputs stay ignored.

## Performance rule
Design for original hardware limits, not emulator convenience. Measure at milestone gates and use `docs/MEMORY-BUDGET.md` as an initial ceiling, revising it from real measurements.

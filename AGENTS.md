# AGENTS.md — LeafGreen-PSX Agent Guide

This file is the canonical entry point for AI coding agents contributing to LeafGreen-PSX.

## Mission

LeafGreen-PSX is a native PlayStation 1 port/demake of Pokemon LeafGreen. It must execute as PS1/MIPS software. It is **not** a GBA emulator and must never evolve into one.

The exact compatibility reference is Pokemon LeafGreen (USA) Rev 1:

- GBA game code: `BPGE`
- revision: `1`
- ROM size: 16 MiB
- SHA-1: `7862c67bdecbe21d1d69ce082ce34327e1c6ed5e`

The ROM is supplied locally by the developer and must never be committed or redistributed.

## Required reading before editing

Read these in order before making non-trivial changes:

1. `README.md`
2. `docs/MASTER-SPEC.md`
3. `docs/ARCHITECTURE.md`
4. `docs/ROADMAP.md`
5. `docs/M0-PALLET-TOWN.md`
6. `docs/PORTING-MATRIX.md`
7. `docs/BUILD-LOCAL.md`
8. `docs/TESTING.md`
9. `docs/WORK-HANDOFF.md`
10. `docs/AGENT-GUARDRAILS.md`
11. `docs/DECISIONS.md`

For save/trading work also read `docs/MEMORY-CARD-TRADING.md`.

## Non-negotiable architecture rules

- Do not emulate the ARM7TDMI, GBA PPU, GBA sound hardware, GBA cartridge bus, or GBA system as a way to run the game.
- Portable game logic must not directly touch PS1 hardware.
- PS1 GPU, SPU, controller, CD-ROM, timing and memory-card functionality belong behind the platform boundary.
- Prefer deterministic portable C for reusable game logic and host-side testing.
- Keep hardware-specific implementation under `src/platform/psx/` and corresponding interfaces under `include/`.
- Preserve LeafGreen Rev 1 behaviour by default. Any intentional PS1-specific gameplay divergence must be documented.
- Do not silently substitute guessed game data for exact data that can be extracted from the verified local ROM.
- Treat the PS1's memory constraints as first-class design constraints. Prefer area/resource streaming and bounded allocations over loading the game globally.

## Copyright/distribution boundary

Never commit or redistribute:

- Pokemon LeafGreen ROM images
- PlayStation BIOS images
- extracted proprietary Nintendo/Game Freak/Pokemon assets
- generated asset bundles containing copyrighted game resources
- generated BIN/CUE images containing extracted commercial assets

Tools, manifests, original port code, tests using synthetic data, documentation and extraction/conversion logic belong in Git. Generated local resources belong outside version control.

## Networking and trading

GBA link-cable functionality, Wireless Adapter networking, Union Room networking and network multiplayer are not targets.

Trading itself is **not removed permanently**. M10 reserves physical PS1 memory-card trading using the versioned `LGTR` transaction design. Do not couple Pokemon ownership, serialization, trade evolution or save identity to GBA link state in a way that would block M10.

## How to choose work

1. Inspect the roadmap and current handoff.
2. Select the earliest unblocked task that advances the active milestone.
3. Do not skip ahead to cosmetic features while an architectural dependency is unfinished unless the work is explicitly independent.
4. If a task requires unavailable ROM assets, PSn00bSDK, emulator/hardware interaction or visual validation, do not fake completion. Implement only independently verifiable prerequisites and leave a precise handoff.
5. Keep changes focused and reviewable.

## Definition of done

A contribution is not complete until applicable checks pass and documentation reflects the new state.

At minimum:

- compile changed portable C with the repository's strict host-test flags where applicable;
- run relevant Python and C host tests;
- add tests for new portable behaviour when practical;
- preserve the platform abstraction;
- update milestone/checklist documentation only for genuinely completed work;
- update `docs/WORK-HANDOFF.md` when the next agent needs new information;
- record significant architectural decisions in `docs/DECISIONS.md`;
- do not claim PS1 runtime correctness without PS1-compatible runtime validation.

See `docs/TESTING.md` and `docs/BUILD-LOCAL.md` for commands and environment-specific steps.

## Agent behaviour

- Inspect existing code before replacing it.
- Prefer extending existing abstractions to creating parallel systems.
- Never weaken validation just to make a test pass.
- Never remove a failing test without documenting why the requirement changed.
- Avoid speculative large rewrites.
- Avoid dependencies that are unnecessary on original PS1 hardware.
- Use fixed-width integer types where serialized/on-disc formats require stable widths.
- Bounds-check parsers, script execution and resource reads.
- Keep serialization versioned.
- Keep generated resources reproducible from the verified local ROM wherever practical.
- Clearly distinguish verified facts from assumptions/TODOs in documentation.

## Commit / PR expectations

Use small, descriptive commits. A PR should explain:

- what milestone/task it advances;
- what changed;
- how it was tested;
- what remains unverified on PS1 hardware/runtime;
- whether memory, disc layout, save compatibility or asset formats changed.

Do not bundle unrelated refactors into milestone work.

## Handoff rule

When stopping, leave the repository in a state where another unfamiliar agent can answer these questions without chat history:

1. What is the current milestone?
2. What was completed?
3. What is the next unblocked task?
4. What exact command/test should be run next?
5. What requires the user's local ROM or PS1 runtime?
6. What assumptions remain unverified?

`docs/WORK-HANDOFF.md` is the durable location for those answers.

## Prime directive

An agent should be able to clone this repository, read this file and the required documents, identify the next unblocked roadmap task, implement it without violating the native-port architecture, test what can be tested locally, document the result, and hand the project cleanly to the next contributor.
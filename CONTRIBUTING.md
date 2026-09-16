# Contributing to LeafGreen-PSX

LeafGreen-PSX welcomes focused contributions that advance the native PlayStation 1 port while preserving compatibility with Pokemon LeafGreen (USA) Rev 1.

AI agents must begin with `AGENTS.md`. Human contributors should read it as well because it contains the project's architectural and distribution constraints.

## Before starting

Read `README.md`, `AGENTS.md`, the active milestone document, `docs/ARCHITECTURE.md`, `docs/TESTING.md` and `docs/WORK-HANDOFF.md`.

Check the roadmap before selecting work. Prefer the earliest unblocked task in the active milestone. If you want to make a large architectural change, document the rationale before implementation.

## Development principles

- This is a native PS1 port/demake, not a GBA emulator.
- Keep portable game logic separate from PS1 hardware implementation.
- Target the exact LeafGreen USA Rev 1 reference documented by the project.
- Keep PS1 RAM/VRAM/CD constraints visible in design decisions.
- Prefer deterministic, testable portable C.
- Preserve original gameplay behaviour unless a divergence is intentional and documented.
- Do not commit commercial ROMs, BIOS files or extracted proprietary assets.

## Local assets

The build/tooling architecture expects the developer to provide their own verified LeafGreen Rev 1 ROM locally. The repository's verifier and extraction/conversion tools establish the boundary between distributable source code and locally generated resources.

Do not submit extracted game resources in a PR.

## Testing

Run all tests relevant to your change. Portable components should receive host-side tests whenever practical. Do not mark a PS1-facing task complete solely because host tests pass if its acceptance criteria require GPU, controller, CD-ROM, timing, audio or memory-card runtime behaviour.

See `docs/TESTING.md` and `docs/BUILD-LOCAL.md`.

## Pull requests

Keep PRs focused. Include:

1. milestone/task IDs advanced;
2. summary of implementation;
3. tests run and their results;
4. PS1 runtime validation performed, if any;
5. known limitations or remaining validation;
6. changes to serialized formats, RAM/VRAM budgets, CD layout or save compatibility.

If work changes an architectural decision, update `docs/DECISIONS.md`. If it changes what the next contributor should do, update `docs/WORK-HANDOFF.md`.

## Trading scope

Network/link-cable multiplayer is outside the target. Physical PS1 memory-card trading is a future M10 goal. Contributions to Pokemon/save architecture must preserve the ability to serialize Pokemon and perform ownership/trade transactions independently of GBA link state.

## Completion standard

A checklist item should only be marked complete when its documented acceptance criteria are satisfied. When hardware/runtime validation is unavailable, leave the item open and document exactly what remains to be tested.

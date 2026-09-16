# Porting method

## Principle
Port behaviour and data into a native PS1 architecture; do not recreate the GBA hardware environment simply to run ARM code.

## Workflow per subsystem
1. Identify exact LeafGreen Rev 1 behaviour/data dependencies.
2. Classify each dependency as portable, hardware-specific, data conversion, or intentionally removed/deferred.
3. Write/retain a portable interface and deterministic host tests where possible.
4. Generate required proprietary data locally from the verified input/reference.
5. Implement the PS1 backend/resource representation.
6. Validate in the PS1 runtime, then compare behaviour against the exact reference.
7. Record memory/performance costs and update budgets.

## Preserve where practical
Game rules, Pokémon/move/item/trainer data, progression, maps/events, dialogue semantics, encounter logic and battle calculations.

## Replace
ARM/Thumb or GBA-specific assembly where required, MMIO/register access, DMA/interrupt assumptions, PPU/sprite/background rendering, keypad, flash save, GBA audio hardware and cartridge resource access.

## Remove/redesign
GBA link/wireless networking is removed. Trading is redesigned later around PS1 memory cards. Enhanced presentation is deferred until compatibility milestones are stable.

## Upstream reference boundary
Use the exact-revision decompilation/reference to understand symbols, structures and behaviour, but keep this repository's redistribution boundary explicit. Prefer original portable implementations and locally generated data rather than copying large third-party source/assets into the public repository without a clear license basis.

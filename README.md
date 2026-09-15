# LeafGreen-PSX

> Shhh…. It’s exactly what you think it is…

LeafGreen-PSX is an experimental native PlayStation (PS1/PSX) port/demake of **Pokémon LeafGreen Version (USA) Rev 1**.

The project goal is to preserve LeafGreen's gameplay and data while replacing Game Boy Advance hardware dependencies with native PlayStation systems. It is **not** intended to ship as a GBA emulator wrapped in a PS1 executable.

## Target

- Original PlayStation / PS one hardware
- PSn00bSDK toolchain
- Native MIPS R3000A executable
- PS1 GPU rendering
- PS1 controller input
- PS1 SPU audio (later milestone)
- PS1 Memory Card saves
- CD-ROM resource bundles
- 320×240 baseline presentation

## Source ROM

Development targets a legally obtained LeafGreen USA Rev 1 ROM. The build tooling verifies the expected ROM before any local extraction step.

Expected SHA-1:

`7862c67bdecbe21d1d69ce082ce34327e1c6ed5e`

**Do not commit ROM files or extracted proprietary assets to this repository.**

## M0 — Pallet Town

The first major proof-of-concept milestone is:

PS1 boot → LeafGreen title → New Game → Oak introduction → player setup → bedroom → Pallet Town → movement/collision → building transitions.

The early foundation work (LGPSX-001–006) establishes the PS1 runtime, video, input, timing and platform boundary. ROM/resource integration begins at LGPSX-007.

## Multiplayer / trading scope

GBA link-cable code, Wireless Adapter support, Union Room networking and link battles are not being ported. The PS1 has no equivalent built-in network/link environment that justifies carrying those GBA subsystems into the compatibility core.

**Trading is a long-term goal via PlayStation memory cards.** The intended future design is asynchronous physical-card trading: serialize a Pokémon/trade transaction to a PS1 memory card, transfer/insert the card, and complete the exchange without networking. The architecture will preserve the seams required for this feature while implementation remains deferred until the core single-player port is stable. See [the memory-card trading design](docs/MEMORY-CARD-TRADING.md).

## References

- `pret/pokefirered` — behavioural/source reference for FireRed/LeafGreen
- `Lameguy64/PSn00bSDK` — open-source PlayStation development SDK

## Legal / repository policy

This repository must not distribute Nintendo/Game Freak ROM images or extracted copyrighted game assets. Tooling should operate locally on a user-supplied compatible ROM. LeafGreen-PSX is an unofficial fan engineering project and is not affiliated with Nintendo, Game Freak, Creatures Inc. or The Pokémon Company.

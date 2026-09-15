# LeafGreen-PSX

> Shhh… it’s exactly what you think it is.

**LeafGreen-PSX** is an experimental native PlayStation port/demake project targeting Pokémon LeafGreen (USA) Rev 1 as a behavioural/data reference. It is not a GBA emulator: the goal is to execute a PS1-native MIPS game runtime with PS1 GPU, SPU, controller, CD-ROM and memory-card backends.

## Target ROM

Development tooling supports a user-supplied Pokémon LeafGreen USA Rev 1 ROM with SHA-1 `7862c67bdecbe21d1d69ce082ce34327e1c6ed5e`, game code `BPGE`, revision `1`, size 16 MiB. The ROM is never distributed by this repository.

Verify locally:

```sh
python tools/romverify/verify_leafgreen.py /path/to/your/leafgreen.gba
```

## M0: Pallet Town

The first vertical slice is PS1 boot -> title -> New Game -> Oak intro -> player setup -> bedroom -> downstairs -> Pallet Town, with native movement, collision and building transitions. See `docs/M0-PALLET-TOWN.md`.

## Architecture

Portable game logic must not directly access PlayStation hardware. Platform-specific video, input, timing, CD, audio and save functionality lives behind the PS1 platform layer. Host-side conversion/testing tools are used wherever possible before hardware integration.

Generated commercial game assets, ROMs and disc images are excluded from version control. See `docs/ASSET-PIPELINE.md`.

## Trading

GBA link cable, Wireless Adapter and network multiplayer are outside scope. Trading itself is a long-term goal: LeafGreen-PSX is designed to eventually support asynchronous Pokémon trades through physical PS1 memory cards. See `docs/MEMORY-CARD-TRADING.md`.

## Current status

Foundation through LGPSX-009 is implemented on the M0 branch: PS1 project/platform skeleton, exact-ROM verification, versioned asset boundary and a deterministic GBA 4bpp/palette -> PS1 TIM prototype. Text rendering and overworld systems are now being built on top.

## Legal / project boundary

This repository does not provide Pokémon LeafGreen, Nintendo/Game Freak assets, BIOS files or extracted proprietary resources. Users/developers must provide their own legally obtained inputs where required. LeafGreen-PSX is an independent fan/technical project and is not affiliated with or endorsed by Nintendo, Game Freak, Creatures or The Pokémon Company.

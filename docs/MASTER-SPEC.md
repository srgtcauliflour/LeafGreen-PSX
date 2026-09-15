# LeafGreen-PSX Master Specification

## Mission
Create a native original-PlayStation port/demake of Pokemon LeafGreen (USA) Rev 1. The shipped PS1 program must execute native MIPS code; embedding a GBA CPU/system emulator is not an acceptable implementation of the game runtime.

## Reference target
- Game: Pokemon LeafGreen (USA)
- Revision: 1 / v1.1
- GBA game code: `BPGE`
- ROM size: 16 MiB
- SHA-1: `7862c67bdecbe21d1d69ce082ce34327e1c6ed5e`
- Source-level behavioural reference: pret/pokefirered `leafgreen_rev1`

The user's ROM remains local. ROM images and extracted copyrighted assets must never be committed.

## Platform target
- Original PlayStation / PS one compatible hardware
- PSn00bSDK toolchain
- 320x240 baseline framebuffer target
- Digital PS1 controller baseline; DualShock may be supported later
- PS1 GPU rendering
- SPU audio
- Memory Card persistence
- CD-ROM resource bundles

## Architectural rule
Portable LeafGreen game code must not directly access PlayStation hardware. Hardware access is routed through `include/lg/platform.h` and platform-specific implementations.

## Network/link scope
The original PlayStation has no standard built-in networking or GBA link interface. The following LeafGreen communication features are explicitly outside the compatibility target:
- GBA link cable
- Wireless Adapter
- Union Room
- trading
- multiplayer/link battles
- communication protocols and communication-error state machines
- network/link-dependent Mystery Gift functionality

The game must remain completable as a single-player title. Changes required to make trade evolutions or version-dependent Pokemon obtainable are deferred until the compatibility core is stable and will be documented as intentional PS1 adaptations.

## Compatibility policy
During initial porting, preserve LeafGreen Rev 1 behaviour wherever it does not depend on removed communications hardware. Do not mix optional gameplay enhancements into compatibility work.

## Resource strategy
Do not mirror cartridge memory assumptions. Convert game resources into PS1-friendly area/system bundles loaded from CD-ROM into main RAM, VRAM and SPU RAM. Avoid excessive small synchronous CD reads.

## M0: Pallet Town proof
M0 is accepted when a native PS1 build can progress through:

PS1 boot -> title -> New Game -> Oak introduction -> player setup -> bedroom -> downstairs -> Pallet Town -> player movement/collision -> basic building transitions.

No ARM/GBA CPU emulation may be used to satisfy M0.

## Engineering priorities
1. Correctness and reproducibility.
2. Real-hardware compatibility.
3. Strict memory/resource budgeting.
4. Behavioural comparison with LeafGreen Rev 1.
5. Clear platform abstraction.
6. Enhancements only after the compatibility baseline works.

# LeafGreen-PSX Master Specification

## Product
A native original-PlayStation port/demake runtime using Pokémon LeafGreen USA Rev 1 as the exact behavioural/data target. The shipped runtime executes MIPS PS1 code and uses PS1 GPU, SPU, controller, CD-ROM and memory-card services; it does not execute the game through an embedded GBA emulator.

## Compatibility target
Supported development input initially: 16 MiB LeafGreen USA Rev 1, game code BPGE, revision 1, SHA-1 `7862c67bdecbe21d1d69ce082ce34327e1c6ed5e`. Other revisions are rejected until explicitly mapped/tested.

## Distribution boundary
The repository contains original port/runtime code, tools, manifests/specifications and tests. It must not contain commercial ROMs, BIOS files or extracted proprietary game assets. Generated resources are local build products.

## Architecture
Game logic is portable. Hardware access is behind platform services. Host tooling verifies the ROM, extracts only locally required data, converts assets into PS1-friendly formats and produces deterministic metadata. CD resources are organized for context/area loading rather than cartridge-style arbitrary access.

## M0 definition
BIOS/boot -> title -> New Game -> Oak intro -> player setup -> bedroom -> downstairs -> Pallet Town -> movement/collision -> building transitions. See M0-PALLET-TOWN.md.

## Rendering direction
Correctness comes before enhancement. Indexed LeafGreen art maps naturally to PS1 indexed textures/CLUTs, but PS1 transparency semantics and VRAM placement are explicit. A later PS1 Enhanced milestone may expand presentation after compatibility is stable.

## Saving
GBA flash saving is replaced by a transactional PS1 memory-card design with recoverability. Save identity and Pokémon serialization must be stable enough to support the later trading milestone.

## Multiplayer/trading
Do not emulate GBA link cable, Wireless Adapter, Union Room networking or network battles. Trading is redesigned as M10: asynchronous transactions using physical PS1 memory cards, with Slot 1/Slot 2 and carry-card workflows. The `LGTR` envelope reserves versioning, transaction IDs, state and integrity checks now without blocking the single-player port.

## Engineering quality
Host-test portable logic, run strict warnings, validate exact target identity, document assumptions and measure on PS1 runtime/hardware at milestone gates. A stub does not satisfy a roadmap item whose acceptance criterion is visible/gameplay behaviour.

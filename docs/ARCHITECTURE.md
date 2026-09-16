# Architecture

## Layers

`game/overworld/script` contain portable state and behaviour. `platform/psx` owns PS1 hardware interaction. `tools` run on the development host and transform a verified local source into generated build resources.

## Resource flow
Verified LeafGreen Rev 1 input -> manifest/reference metadata -> local extraction -> conversion -> generated PS1 resource -> area bundle/CD image -> resource manager -> VRAM/RAM/SPU as needed.

The ROM is a build input, not a runtime cartridge abstraction. Game code should request logical resources rather than seek arbitrary ROM addresses.

## Data ownership
Portable game state owns player/progression/Pokémon state. Rendering consumes snapshots/commands rather than owning gameplay state. Save serialization is versioned and independent of memory-card transport. Future LGTR trades reuse Pokémon serialization but do not directly mutate storage until a transaction commit succeeds.

## Overworld
Maps expose compact logical cells/metatiles, collision/elevation and event/warp metadata. The PS1 renderer turns visible map data into textured primitives; movement/collision stays portable. Area bundles constrain working-set memory and CD latency.

## Scripts
The event VM is portable and yields to game services. The current tiny VM is scaffolding only; exact M0 commands must be mapped from the supported reference before LGPSX-019 can be completed.

## Testing boundary
Converters, serialization, battle/game math, scripts and movement should be host-testable. GPU/SPU/pad/CD/card backends require PS1 integration testing. Milestone completion requires both where relevant.

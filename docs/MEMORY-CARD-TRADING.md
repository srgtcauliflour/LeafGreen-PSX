# Memory Card Trading — Future Project Goal

## Status

Deferred. This feature is **not part of M0** and must not block the native single-player port.

## Goal

LeafGreen-PSX should eventually support asynchronous Pokémon trading between two players using physical PlayStation memory cards, requiring no network hardware or GBA link cable.

A likely user flow is:

1. Player A visits an in-game Trade Center.
2. The game exports a selected Pokémon into a dedicated trade record on Player A's memory card.
3. The card is physically moved to another PlayStation, or inserted into the second memory-card slot of the same console.
4. Player B imports/accepts the offer and deposits the return Pokémon.
5. Player A later imports the completed trade.

The final protocol must prevent accidental duplication, corruption and replay as far as practical on original PS1 hardware. The design should use transaction IDs, checksums, explicit states and recovery records rather than treating a trade as a simple Pokémon file copy.

## Hardware constraints

A standard PS1 memory card provides 128 KiB total storage arranged as 16 blocks of 8 KiB; block 0 contains card metadata/directory structures, leaving 15 data blocks for saves. Low-level transfers operate on 128-byte sectors. The eventual format should therefore be compact and should preferably coexist with the normal LeafGreen-PSX save rather than consuming excessive blocks.

## Architectural requirement now

Although implementation is deferred, the main save architecture must not make memory-card trading impossible later. Keep these concepts separable:

- persistent player/save identity
- Pokémon serialization/deserialization
- party and PC storage mutation
- trade-evolution trigger
- provenance/original-trainer metadata
- save transaction/commit layer
- memory-card I/O layer

Do not couple Pokémon ownership transfer directly to GBA link state.

## Future design work

The eventual specification should define:

- `LGTR` trade-record format and versioning
- offer / accepted / completed / cancelled transaction states
- unique trade and save identifiers
- Pokémon payload serialization
- trainer/OT metadata preservation
- checksums and corruption detection
- replay/duplication mitigation
- atomic/recoverable memory-card writes
- Slot 1 ↔ Slot 2 direct trading UX
- single-card "carry the trade" workflow between consoles
- trade-evolution behaviour
- compatibility rules between future LeafGreen-PSX versions

## Scope clarification

GBA link cable, Wireless Adapter, Union Room networking and network-based multiplayer remain outside the project scope. **Trading itself is not permanently removed**: memory-card-mediated trading is a long-term LeafGreen-PSX feature goal.
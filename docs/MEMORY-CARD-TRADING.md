# Memory Card Trading — Future Project Goal

## Status
Deferred. This feature is **not part of M0** and must not block the native single-player port.

## Goal
LeafGreen-PSX should eventually support asynchronous Pokémon trading between two players using physical PlayStation memory cards, requiring no network hardware or GBA link cable.

A likely flow is Player A creates an offer, the card is moved to another console or used in the second card slot, Player B accepts and deposits a return Pokémon, then Player A completes the transaction.

## Hardware-aware design
A standard PS1 memory card is 128 KiB, divided into 16 blocks of 8 KiB with block 0 used for directory/card metadata; low-level transfers use 128-byte sectors. The trade format must be compact and coexist with the normal save where practical.

## Architectural requirement now
Keep persistent save/player identity, Pokémon serialization, party/PC mutation, trade-evolution triggers, provenance/OT metadata, save transactions and memory-card I/O separable. Do not couple Pokémon ownership transfer to GBA link state.

## Reserved LGTR format
The repository now reserves a small versioned `LGTR` record API. Version 1 has transaction/save IDs, state, payload length, payload CRC32 and a bounded payload. This is intentionally only a durable envelope: the Pokémon payload and anti-duplication transaction protocol remain deferred until the save model exists.

States currently reserved: EMPTY, OFFERED, ACCEPTED, COMPLETED and CANCELLED. Future implementation must use recoverable/atomic writes and maintain enough history to reject replayed completed transactions as far as practical on untrusted removable media.

## Future design work
Define Pokémon payload serialization, trainer/OT preservation, trade-evolution semantics, Slot 1 ↔ Slot 2 UX, single-card carry workflow, transaction recovery, replay/duplication mitigation and compatibility rules between future LeafGreen-PSX versions.

## Scope clarification
GBA link cable, Wireless Adapter, Union Room networking and network-based multiplayer remain outside project scope. **Trading itself is not permanently removed**: memory-card-mediated trading is a long-term LeafGreen-PSX goal.

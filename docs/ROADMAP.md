# Roadmap

## M0 — Pallet Town vertical slice
Native boot, title/new-game path, bedroom/Pallet Town maps, player movement/collision, warps and minimum scripts. Host/tooling foundation through LGPSX-009 is in place; text/overworld/script scaffolds are underway.

## M1 — Overworld engine
General map loading, NPCs, object events, connections, scripts, flags, dialogue and stable area-bundle loading.

## M2 — Menus and inventory
Start menu, bag, items, party UI, PC/storage foundations and reusable window/text systems.

## M3 — Pokémon and battles
Species/move data, party model, encounters, trainers, turn battle engine, battle UI, status/experience/evolution and deterministic behavioural tests.

## M4 — Audio
SPU-native SFX/music pipeline, contextual loading/streaming and soundtrack conversion strategy.

## M5 — Saves
Robust PS1 memory-card saves, backup/recovery strategy, player/save identity and serialization boundaries needed by later trading.

## M6 — Kanto completion
Story progression, gyms, routes, caves, field moves, Elite Four and single-player accessibility policy for unavailable/trade-dependent Pokémon.

## M7 — Sevii/postgame
Sevii Islands, postgame progression and remaining LeafGreen content appropriate to the native port.

## M8 — Compatibility and accuracy
Regression suite against exact LeafGreen Rev 1 behaviour/data, original-hardware profiling and edge-case stabilization.

## M9 — PS1 Enhanced
Optional presentation/features that deliberately diverge from strict LeafGreen compatibility, including expanded viewport/presentation choices.

## M10 — Memory Card Trading
Implement the deferred `LGTR` transaction system: Slot 1/Slot 2 trading, carry-card asynchronous trades, trade evolutions, provenance preservation, atomic recovery and practical replay/duplication mitigation. No network hardware required.

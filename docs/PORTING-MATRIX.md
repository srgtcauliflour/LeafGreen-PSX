# Porting matrix

| System | Decision | PS1 strategy |
|---|---|---|
| core game state/progression | PORT | portable C/data model |
| Pokémon/species/moves/items/trainers | PORT | generated local data + portable logic |
| battle calculations | PORT | deterministic portable logic/tests |
| maps/events/scripts | PORT/ADAPT | converted data + portable event services |
| GBA PPU/background/sprite engine | REPLACE | PS1 GPU textured primitives/TIM/CLUT |
| GBA DMA/register/interrupt assumptions | REPLACE | PS1 platform/timing services |
| keypad | REPLACE | PS1 pad translation |
| cartridge/flash save | REPLACE | PS1 memory-card transaction layer |
| GBA audio/m4a hardware path | REPLACE | PS1 SPU-native pipeline |
| ROM random-access assumptions | REPLACE | CD area bundles/resource manager |
| link cable/Wireless Adapter | REMOVE | no PS1 network/link emulation |
| Union Room/link battles | REMOVE | no network multiplayer |
| trading | DEFER/REDESIGN | M10 physical memory-card LGTR transactions |
| trade evolutions/version accessibility | DEFER | compatibility policy after core stability |
| Mystery Gift requiring GBA comms | REMOVE/REDESIGN LATER | optional non-network replacement only if justified |
| enhanced viewport/presentation | DEFER | M9 optional PS1 Enhanced mode |

The exact LeafGreen Rev 1 target remains the behavioural reference. A subsystem is only considered ported when its relevant behaviour works on the native runtime; stubs do not count.

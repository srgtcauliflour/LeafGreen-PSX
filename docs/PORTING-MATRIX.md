# Porting Matrix

Each LeafGreen subsystem is classified as **PORT**, **REPLACE**, **REMOVE**, or **DEFER**.

| Subsystem | Action | PS1 direction |
|---|---|---|
| Pokemon/gameplay data | PORT | Preserve data/behaviour |
| Core battle calculations | PORT | Portable C where practical |
| Event/script logic | PORT | Preserve semantics |
| Overworld/maps/collision | PORT | PS1 renderer + resource format |
| Menus/text | PORT | PS1 GPU implementation |
| GBA PPU/register access | REPLACE | PS1 GPU |
| GBA keypad | REPLACE | PS1 controller |
| GBA sound hardware | REPLACE | PS1 SPU |
| Cartridge flash/SRAM | REPLACE | PS1 Memory Card |
| ROM/cartridge resource access | REPLACE | CD-ROM bundles/cache |
| GBA DMA/interrupt assumptions | REPLACE | PS1-safe scheduling/timing |
| Link cable | REMOVE | Not supported |
| Wireless Adapter | REMOVE | Not supported |
| Union Room | REMOVE | Not supported |
| Trading | REMOVE | Not supported |
| Link battles | REMOVE | Not supported |
| Link communication errors/protocols | REMOVE | Not supported |
| Trade-evolution replacement rules | DEFER | Single-player adaptation after compatibility baseline |
| Version-exclusive accessibility | DEFER | Optional single-player adaptation |
| PS1 Enhanced visual mode | DEFER | After compatibility baseline |

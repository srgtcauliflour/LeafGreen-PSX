# Project decisions

## D001 — Native runtime
LeafGreen-PSX is a native PS1 port/demake, not a packaged GBA emulator.

## D002 — Exact initial target
LeafGreen USA Rev 1 is the only supported input until additional revisions are deliberately mapped/tested.

## D003 — Public repository boundary
Do not commit ROM/BIOS/extracted proprietary assets. Generate required resources locally.

## D004 — Platform isolation
Portable game logic never directly calls PS1 hardware APIs.

## D005 — CD resource model
Use logical resources and area/context bundles rather than exposing cartridge-like ROM offsets to gameplay code.

## D006 — Multiplayer
GBA link/wireless networking is not ported.

## D007 — Trading
Trading remains a product goal but is redesigned for physical PS1 memory cards as M10. Reserve serialization/transaction boundaries early; do not block M0 on it.

## D008 — Accuracy before enhancement
Reach stable LeafGreen-compatible vertical slices before optional PS1 Enhanced presentation changes.

## D009 — Evidence-based milestone completion
Scaffolding is useful but does not complete a task whose acceptance criterion requires actual generated game data or PS1 runtime behaviour.

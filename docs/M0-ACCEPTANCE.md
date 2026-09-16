# M0 acceptance checklist

## Boot/platform
- PS1 BIOS hands off to the native executable.
- Stable 320x240 display output.
- Digital controller input maps to game actions.
- Frame pacing does not depend on GBA timing/registers.

## New game path
- Title screen can enter New Game.
- Oak intro progresses through required dialogue/choices.
- Player setup commits the required initial state.

## Overworld
- Bedroom renders from locally generated LeafGreen-derived resources.
- Player sprite renders/animates and moves on the grid.
- Collision prevents entry into blocked cells.
- Stairs/door warps transition correctly.
- Pallet Town renders and is walkable.
- Required M0 scripts/dialogue/events yield/resume correctly.

## Native proof
- No ARM/GBA CPU emulator executes LeafGreen code.
- Game logic reaches M0 through MIPS-native runtime and converted/local data.
- PS1 GPU/pad/CD services are used through the platform boundary.

## Engineering gate
Record PS-EXE size, peak RAM, VRAM allocation, frame time, CD reads/load latency and known compatibility deviations. Host CI must pass. No ROM/BIOS/extracted proprietary assets may be present in Git history.

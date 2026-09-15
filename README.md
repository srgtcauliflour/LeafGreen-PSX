# LeafGreen-PSX

> Shhh… it’s exactly what you think it is.

**LeafGreen-PSX** is an experimental native PlayStation/PS one port/demake project targeting **Pokemon LeafGreen (USA) Rev 1**.

The objective is not to package a Game Boy Advance emulator. The project is exploring how LeafGreen's reconstructed game logic/data can be adapted to native MIPS code and PlayStation hardware through a clean platform layer.

## Current milestone: M0 — Pallet Town

The first proof-of-concept target is:

`PS1 boot -> title -> New Game -> Oak intro -> player setup -> bedroom -> Pallet Town -> movement/collision/building transitions`

See [`docs/M0-PALLET-TOWN.md`](docs/M0-PALLET-TOWN.md).

## Supported reference ROM

Development targets a locally supplied Pokemon LeafGreen (USA) Rev 1 ROM:

- Game code: `BPGE`
- Revision: `1`
- Size: `16 MiB`
- SHA-1: `7862c67bdecbe21d1d69ce082ce34327e1c6ed5e`

**ROM files and extracted copyrighted game assets are not part of this repository.**

Verify a local dump with:

```bash
python tools/romverify/verify_leafgreen.py /path/to/leafgreen.gba
```

## Build foundation

The native PS1 target uses PSn00bSDK and CMake. Set `PSN00BSDK_LIBS` to the SDK's `lib/libpsn00b` directory, then:

```bash
cmake --preset default
cmake --build build
```

The CMake project defines a PS-EXE and a BIN/CUE CD image target.

## Scope note: multiplayer

GBA link cable, Wireless Adapter, Union Room, trading and link battles are intentionally outside the PS1 compatibility target. LeafGreen-PSX is designed as a single-player game. Single-player replacements for trade evolutions/version-exclusive accessibility are deferred until the compatibility core is stable.

## Documentation

- [`docs/MASTER-SPEC.md`](docs/MASTER-SPEC.md) — canonical project direction
- [`docs/PORTING-MATRIX.md`](docs/PORTING-MATRIX.md) — PORT / REPLACE / REMOVE / DEFER decisions
- [`docs/M0-PALLET-TOWN.md`](docs/M0-PALLET-TOWN.md) — first milestone

## Upstream references

LeafGreen-PSX is informed by the community reconstruction work in `pret/pokefirered` and uses PSn00bSDK for the PlayStation platform. Those projects are separate upstream projects and are not vendored here at this stage.

## Legal / project hygiene

Do not commit commercial ROM images, save dumps, extracted proprietary assets, generated disc images, or other copyrighted game content. Developers supply their own legally obtained source material locally.

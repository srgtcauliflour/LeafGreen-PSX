# gfxconv

Host-side graphics conversion for LeafGreen-PSX. Current prototype accepts raw GBA 4bpp tile bytes plus one 16-entry little-endian BGR555 palette and emits an indexed 4bpp PlayStation TIM.

Example after producing local generated inputs:

```sh
python tools/gfxconv/convert.py generated/input/tiles.4bpp generated/input/palette.bin generated/output/tiles.tim
```

The prototype lays multiple 8x8 tiles in a horizontal strip. Production work still needs texture-page packing, multiple CLUTs, flip/attribute handling, metatiles, animation and placement metadata. Never commit extracted input or converted proprietary output.

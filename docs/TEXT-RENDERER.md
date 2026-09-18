# Normal Latin text bring-up

LGPSX-010 now has a verified local font converter, a bounded portable layout
subset, and an optional native PS1 glyph-quad demo. Runtime acceptance is pending.

## Generate and build

```sh
python tools/font/extract_font.py /path/to/leafgreen.gba
cmake --preset default -DLGPSX_FONT_DEMO=ON
cmake --build build
```

`generated/font/` contains a 256-glyph TIM atlas, aligned C data and a provenance
manifest. These are proprietary local build products; never commit or publish
them or a disc image containing them. To return to the asset-free diagnostic
build, configure with `-DLGPSX_FONT_DEMO=OFF` and rebuild. The renderer is confined
to `src/platform/psx/text_psx.c`.

## Exact ROM evidence

Whole-ROM SHA-1 is checked before producing output. The complete 32,768-byte
font and 512-byte width table are also checked against SHA-256 descriptors in
the extraction tool. On the exact supported ROM, four identical font copies
occur at `0x1F314C`, `0x1FF34C`, `0x20F664`, `0x21F97C`, with identical width
tables immediately following each. The extractor uses the first copy; its bytes
are identical to the normal Latin reference, not a guessed offset.

Evidence was established by converting the indexed reference PNG to its
full-width Latin packing and matching the entire result to the verified ROM,
then matching the complete reference width table. Reference sources:

- [Font graphic](https://github.com/pret/pokefirered/blob/master/graphics/fonts/latin_normal.png)
- [Packing format](https://github.com/pret/pokefirered/blob/master/tools/gbagfx/font.c)
- [Widths and glyph semantics](https://github.com/pret/pokefirered/blob/master/src/text.c)
- [Character encoding](https://github.com/pret/pokefirered/blob/master/charmap.txt)

Each glyph occupies 64 source bytes: four 8x8 tiles, with two little-endian
bytes per row and the leftmost pixel in the high two bits. Conversion lays out
the first 256 glyphs in a 256x256, 4bpp PS1 atlas. Glyph zero is forced blank,
as in the reference renderer. Full reference extended/Japanese text is not yet
supported. The demo uses original diagnostic strings, not copied game dialogue.

## Layout contract

`lg_text_layout` accepts a byte span and an explicit 256-entry width table. It
reports termination, missing terminator, unsupported control, invalid input or
sink exhaustion. It supports ordinary glyphs, `FE` newline and `FF` end. Codes
`F7` through `FD` stop processing: placeholders, scrolling, pauses and formatting
need explicit game-service handling before they can be supported. Lines advance
16 pixels for this demo. There is no typewriter effect at this layer, but see
`LGWindowState`'s `reveal_per_step` below.
The ASCII helper supports letters, digits, spaces, newlines and `!?.-` only.

`LGDialogueState`/`LGWindowState` (see below) add an optional bounded
`wrap_width`: character wrapping only (it breaks before whichever glyph would
exceed the bound, not at word boundaries), and it never breaks the first
glyph on a line even if that glyph alone exceeds the bound. `lg_text_layout`
itself is unchanged and still lays out one unbounded line per `FE`.

`LGWindowState` also takes an optional `max_lines`: once a glyph would start
one line beyond that bound, it is withheld and `lg_window_step` reports
`LG_WINDOW_AWAIT_SCROLL` until the caller signals an advance, at which point
the box resets to its first line (`y` back to the box's starting `y`) and
continues from the withheld glyph. This gates the LeafGreen-style
scroll-a-full-box-then-continue behaviour behind an explicit per-frame
signal, the same way `LG_WINDOW_AWAIT_ADVANCE` gates a control byte.

`LGWindowState` also takes an optional `reveal_per_step`: a typewriter
effect that paces drawing to that many glyphs per `lg_window_step()` call
(one real frame, in `LGGameLoop`'s use of it). Unlike
`AWAIT_ADVANCE`/`AWAIT_SCROLL`, pausing for this needs no caller
acknowledgement -- it reports plain `LG_WINDOW_RUNNING` and simply
continues from the glyph it paused at on the next call. The actual reveal
speed (frames per glyph, or glyphs per frame) is entirely the caller's to
choose by how it drives `lg_window_step()`; nothing here claims a
LeafGreen-verified typing speed.

## PS1 backend and limits

- 4bpp texture at VRAM word coordinate `(640,0)`, size `64x256` words/rows.
- CLUT at `(640,256)`, 16 entries; no overlap with either framebuffer or SDK font.
- Transparent background and box indices, white ink, dark shadow. This is a
  diagnostic colour scheme, not a claim of exact LeafGreen window styling.
- Fixed 16x16 cells, 14-pixel visible height, widths from the exact ROM.
- Immediate textured quads for bring-up; queued HUD work is synchronized first.
- Stops at screen bounds; packet batching and clipping remain later work.

Host tests cover packing, quadrants, space transparency, rejection of the wrong
ROM, proportional placement, newlines, truncation, unsupported controls, sink
capacity and coordinate overflow. Both build modes compile. The native demo
has now run for 300 frames under PCSX-ReARMed HLE with readable glyphs and
correct observed transparency; see PS1-RUNTIME-VALIDATION.md. Retail BIOS and
hardware remain unverified. LGPSX-010 remains open for complete text behaviour.

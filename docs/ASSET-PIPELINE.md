# Asset pipeline

LeafGreen-PSX does not commit the commercial ROM or extracted proprietary assets. Host tools consume a locally supplied, verified LeafGreen USA Rev 1 ROM and emit generated build resources that remain ignored by Git.

## Boundary

`manifest.py` defines a versioned manifest contract. Each entry names an asset kind, source description and generated output. Offsets in the example manifest are deliberately synthetic placeholders; authoritative offsets/symbols must come from the locally built exact-revision reference or verified extraction metadata, not guesses.

## Graphics

The first converter supports GBA 4bpp indexed tiles and 16-colour BGR555 palettes and emits PS1 TIM data. GBA and PS1 both use 15-bit BGR colour components, but PS1 colour bit 15 has transparency/semi-transparency semantics; conversion is therefore explicit rather than blindly copying palette words.

The prototype arranges tiles into horizontal strips. Production map conversion will introduce texture-page packing, CLUT allocation, tile attributes/flips, animated tiles and deterministic manifests.

## Reproducibility

Generated files are build products. A clean checkout plus supported local ROM and toolchain must be sufficient to reproduce them.

# textconv

`charmap.py` is the Gen3 (RSE/FRLG) international text charmap -- a
byte<->character encoding table, reconstructed from the console's
well-documented, widely reverse-engineered Gen3 text format. It is a
functional mapping table, not creative/copyrighted expression (the same
category as an ASCII or Shift-JIS table), and contains no
LeafGreen-specific offsets or extracted strings. `decode_text()`/
`encode_text()` convert between raw charmap bytes and Python `str`,
handling the handful of layout control codes (`\n`, `\p`, `\l`) and the
0xFF end-of-string terminator; an unmapped byte becomes a `\xNN`
placeholder by default (or raises, with `on_unmapped='strict'`) since
this module only models the plain-glyph/control subset, not the many
named multi-byte placeholders (`PLAYER`, `STR_VAR_n`, etc.).

`extract_text.py` is `tools/assets/extract.py`'s manifest-driven,
SHA-1-gated shape, but for `"kind": "text"` manifest entries: instead of
copying raw bytes it decodes each range through `charmap.py` and writes
the result as UTF-8 text. It does not know or publish any real
LeafGreen text offsets itself -- those belong in a caller-supplied
manifest, generated/verified locally against a legally obtained ROM,
never committed.

Never commit extracted input or converted proprietary output (decoded
game text included).

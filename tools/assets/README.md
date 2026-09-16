# asset tools

`manifest.py` validates the versioned extraction/conversion contract. `extract.py` verifies the complete LeafGreen Rev 1 SHA-1 before reading any manifest ranges, preventing accidental use of another revision.

The committed example manifest contains synthetic placeholder offsets and is only for schema/tool testing. Real offsets/symbols must come from verified exact-revision metadata generated/inspected locally.

Generated outputs belong under ignored build/generated paths and must not be committed.

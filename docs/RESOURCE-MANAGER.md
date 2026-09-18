# Resource manager

Game code refers to logical resource IDs, not ROM addresses or CD sectors. A generated area-bundle index maps IDs to type/offset/size. The initial portable lookup structure is intentionally tiny; the PS1 backend will later resolve bundle offsets into asynchronous/scheduled CD reads and cache/upload resources as appropriate.

This boundary is important because LeafGreen's cartridge access model cannot simply be carried over to CD-ROM. It also keeps future packing/layout changes out of gameplay code.

## Byte-span resolution (`lg_resource_bytes`)

`lg_resource_bytes(index, id, expected_kind, buffer, buffer_size,
&out_size)` resolves an id to a byte span within a buffer the caller has
already loaded (whatever "area bundle" the index describes) -- it does no
I/O of its own, CD-scheduled or otherwise. It returns `NULL` if the id
isn't found, its kind doesn't match `expected_kind`, or its
`offset`/`size` doesn't fit entirely within `buffer_size`: an overflowing
or malformed entry is never trusted, not even partially. This is the
piece that turns the resource index from a lookup table into something a
caller can actually hand to, say, `LGGameLoop`'s `LGLoopTextLookupFn` or a
future script loader, once real bundle data and CD loading exist.

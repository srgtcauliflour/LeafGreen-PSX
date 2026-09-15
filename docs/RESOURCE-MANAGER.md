# Resource manager

Game code refers to logical resource IDs, not ROM addresses or CD sectors. A generated area-bundle index maps IDs to type/offset/size. The initial portable lookup structure is intentionally tiny; the PS1 backend will later resolve bundle offsets into asynchronous/scheduled CD reads and cache/upload resources as appropriate.

This boundary is important because LeafGreen's cartridge access model cannot simply be carried over to CD-ROM. It also keeps future packing/layout changes out of gameplay code.

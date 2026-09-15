# Text renderer design

LGPSX-010 is split into two layers: LeafGreen character decoding/layout and PS1 glyph rendering. Game code must not know about GPU packets, texture pages or CLUT coordinates.

The initial implementation will use a generated indexed font atlas. Host extraction/conversion creates the atlas from the user's verified local ROM/reference build; the repository contains no game font graphics. The PS1 backend uploads that atlas and emits textured glyph primitives.

The public game-facing API starts deliberately small and will expand for control codes, variable substitution, scrolling, choice menus and proportional metrics as the relevant LeafGreen behaviour is ported.

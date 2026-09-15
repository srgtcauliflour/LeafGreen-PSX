# Asset manifest schema v1

Top-level fields: `schema` (must be 1), `target` (currently `leafgreen-us-rev1`) and `assets` array. Each asset has a unique `id`, supported `kind`, a `source` description and generated `output` path.

Supported initial kinds: palette, tiles4, tiles8, tilemap, sprite4, sprite8, font, script, map and audio.

The schema intentionally does not require hard-coded ROM offsets in repository data. Source descriptors may later reference verified upstream symbols/extraction metadata produced locally. This prevents guessed offsets from becoming API and allows the same converter stages to consume exact-revision reference outputs.

Manifest validation is strict on identity/kinds but intentionally permissive inside `source` while extraction backends are being designed. A future schema version will lock source descriptor variants once real M0 extraction proves them.

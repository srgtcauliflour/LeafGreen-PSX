# Overworld core

The overworld model is intentionally platform-neutral. Maps expose dimensions plus compact cells containing a metatile ID, collision class, elevation and a warp id. Player movement asks the map whether a destination is enterable; rendering and controller polling remain outside this module.

The first implementation provides deterministic grid movement/collision primitives that can be host-tested before PS1 integration. LeafGreen behaviour such as ledges, doors, object events, elevation interactions, running, biking, surfing and scripted movement will extend this model without putting PS1 hardware calls into game logic.

## Map connections/warps (LGPSX-018, portable half)

`lg_map_warp_at(map, x, y, warps, warp_count)` looks up the cell at `(x, y)`
and, if its `warp` field is nonzero, finds the matching entry (by id) in a
caller-supplied `LGWarp` table. It returns `NULL` for out-of-bounds
coordinates, a cell with no warp, or an id missing from the table (an
incomplete table is not a crash). `LGWarp.dest_map` is a logical resource id
(see resource.h), not a pointer, so a warp table can be authored/serialized
independently of how maps are linked in memory. Warp ids are a portable
indirection, not real LeafGreen warp indices: no ROM evidence maps actual
door/stairs ids yet, and this does not decide when a warp fires (map edge,
a specific tile, an object event) — that policy, and the real per-map warp
data, belong to a later, evidence-based step.

This is preparatory work for LGPSX-012 through LGPSX-018; it is not marked complete until converted real M0 maps render and behave correctly.

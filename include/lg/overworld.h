#pragma once
#include <stddef.h>
#include <stdint.h>
/* warp: 0 means no warp; a nonzero id is looked up in a caller-supplied
   LGWarp table. Ids are a portable indirection, not LeafGreen warp
   indices: no ROM evidence maps real door/stairs ids yet. */
/* ledge: 0 means not a ledge; a nonzero value is (facing + 1) for the one
   direction a ledge can be jumped in (facing values match LGPlayer.facing
   -- 0=up,1=right,2=down,3=left, so ledge=1 means "jump only when facing
   up", etc). Entering a ledge cell from any other direction is blocked,
   same as a collision tile; entering it from the matching direction jumps
   straight over it onto the next cell beyond, same as LeafGreen's ledges
   -- see lg_player_step(). This is our own generic placeholder encoding,
   not verified LeafGreen ledge/metatile-behavior data. */
typedef struct { uint16_t metatile; uint8_t collision; uint8_t elevation; uint8_t warp; uint8_t ledge; } LGMapCell;
typedef struct { uint16_t width,height; const LGMapCell *cells; } LGMap;
/* moving: frames remaining of a tile-slide animation a renderer can use
   to interpolate sprite position, counted down by lg_player_animate_tick().
   x/y already hold the destination tile the instant a step succeeds --
   there is no multi-tile-slide delay to movement/collision itself, only
   to this presentation hint. */
typedef struct { int16_t x,y; int8_t facing; uint8_t moving; } LGPlayer;
/* Placeholder tile-slide duration in frames for LGPlayer.moving: no
   ROM/hardware-verified LeafGreen movement timing backs this number yet. */
#define LG_PLAYER_SLIDE_FRAMES 4
/* Placeholder running-speed tile-slide duration (used by lg_player_run_step()),
   half of LG_PLAYER_SLIDE_FRAMES -- again no ROM/hardware-verified
   LeafGreen running speed backs this number, only that OVERWORLD.md
   flagged "running, biking" as unfinished. */
#define LG_PLAYER_RUN_SLIDE_FRAMES (LG_PLAYER_SLIDE_FRAMES/2)
/* dest_map is a logical resource id (see resource.h), not a pointer, so a
   warp table can be authored/serialized without wiring maps together. */
typedef struct { uint8_t id; uint16_t dest_map; int16_t dest_x, dest_y; } LGWarp;
int lg_map_can_enter(const LGMap *map, int x, int y);
/* Elevation- and ledge-aware entry check: (to_x, to_y) must first pass
   lg_map_can_enter() (bounds + collision). If that cell is a ledge (see
   LGMapCell.ledge's own doc comment), the step from (from_x, from_y) to
   (to_x, to_y) must be moving in the ledge's one required direction --
   any other direction (including standing still) is refused, the same
   as a collision tile, regardless of elevation. Otherwise, its
   LGMapCell.elevation must be compatible with the cell stepped from:
   elevation 0 is a wildcard on either end (e.g. a bridge or stairs tile
   that connects any level), otherwise the two cells' elevation values
   must match exactly, so a player can't cross directly between two
   different nonzero elevations without a connecting tile. An
   out-of-bounds (from_x, from_y) (e.g. no map context yet) skips the
   elevation comparison and only checks lg_map_can_enter() on the
   destination (a ledge's direction is still enforced, since it's
   computed from the from/to delta regardless of bounds). This is our
   own generic placeholder rule, not verified LeafGreen elevation/ledge
   behaviour -- no ROM evidence backs these exact semantics yet, only
   that OVERWORLD.md already flagged "elevation interactions"/"ledges"
   as unfinished. */
int lg_map_can_enter_from(const LGMap *map, int from_x, int from_y,
                           int to_x, int to_y);
/* Sets p->moving to LG_PLAYER_SLIDE_FRAMES when the step actually moves
   the player (not when only turning to face a blocked direction), for a
   renderer to animate. This never blocks or paces the step itself --
   lg_player_step() always applies immediately regardless of any slide
   already in progress; call lg_player_animate_tick() once per frame to
   count it down. Uses lg_map_can_enter_from() (not just lg_map_can_enter())
   so a step is also refused when the destination's elevation is
   incompatible with the player's current cell.

   If the destination cell is a ledge (LGMapCell.ledge, see its own doc
   comment) whose required direction doesn't match this step, the step is
   blocked the same as a collision (turns to face it without moving). If
   it matches, the player jumps straight over the ledge cell onto the
   cell beyond it in the same direction (skipping the ledge tile itself)
   -- provided that landing cell passes a plain lg_map_can_enter()
   (elevation is not re-checked for the landing, since jumping down a
   ledge is itself an elevation change); if it doesn't, the whole step is
   blocked rather than landing partway. moving is set to twice
   LG_PLAYER_SLIDE_FRAMES for a ledge jump, since it covers two cells. */
void lg_player_step(LGPlayer *p, const LGMap *map, int dx, int dy);
/* Identical to lg_player_step() (same collision/elevation/ledge rules,
   same dx/dy shape), except a step that actually moves the player sets
   p->moving to LG_PLAYER_RUN_SLIDE_FRAMES (half of LG_PLAYER_SLIDE_FRAMES)
   instead, and a ledge jump to twice that -- a faster presentation hint
   for a "running" input state, not a different grid-step distance or
   pacing (this scaffolding has never paced steps by frames at all, only
   hinted a renderer's slide animation). No ROM/hardware-verified
   LeafGreen running speed backs this number. */
void lg_player_run_step(LGPlayer *p, const LGMap *map, int dx, int dy);
/* Counts down p->moving by one frame if it is nonzero. No-op at 0 or on
   a NULL player. */
void lg_player_animate_tick(LGPlayer *p);
/* Returns the matching warp for the cell at (x, y), or NULL when the
   coordinates are out of bounds, the cell has no warp, or no table entry
   matches its id (an incomplete table is not a crash). */
const LGWarp *lg_map_warp_at(const LGMap *map, int x, int y,
                              const LGWarp *warps, size_t warp_count);
/* A static NPC/sign/object placed at a tile. script_id is a portable
   indirection into a caller's own script table (the same idea as
   LGScriptTextFn's text_id), not a LeafGreen object-event id. */
typedef struct { int16_t x, y; uint8_t script_id; } LGObjectEvent;
/* Returns the event standing on the tile the player is facing (one step
   from player->x/y in the direction of player->facing -- 0=up, 1=right,
   2=down, 3=left, the same values lg_player_step() assigns), or NULL if
   no event is there or facing holds an unrecognised value. This is how a
   caller finds what an "interact" button press should trigger; it does
   not decide when to call it. */
const LGObjectEvent *lg_object_event_facing(const LGPlayer *player,
                                             const LGObjectEvent *events,
                                             size_t count);

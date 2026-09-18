#pragma once
#include <stddef.h>
#include <stdint.h>
/* warp: 0 means no warp; a nonzero id is looked up in a caller-supplied
   LGWarp table. Ids are a portable indirection, not LeafGreen warp
   indices: no ROM evidence maps real door/stairs ids yet. */
typedef struct { uint16_t metatile; uint8_t collision; uint8_t elevation; uint8_t warp; } LGMapCell;
typedef struct { uint16_t width,height; const LGMapCell *cells; } LGMap;
typedef struct { int16_t x,y; int8_t facing; uint8_t moving; } LGPlayer;
/* dest_map is a logical resource id (see resource.h), not a pointer, so a
   warp table can be authored/serialized without wiring maps together. */
typedef struct { uint8_t id; uint16_t dest_map; int16_t dest_x, dest_y; } LGWarp;
int lg_map_can_enter(const LGMap *map, int x, int y);
void lg_player_step(LGPlayer *p, const LGMap *map, int dx, int dy);
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

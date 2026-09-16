#pragma once
#include <stdint.h>
typedef struct { uint16_t metatile; uint8_t collision; uint8_t elevation; } LGMapCell;
typedef struct { uint16_t width,height; const LGMapCell *cells; } LGMap;
typedef struct { int16_t x,y; int8_t facing; uint8_t moving; } LGPlayer;
int lg_map_can_enter(const LGMap *map, int x, int y);
void lg_player_step(LGPlayer *p, const LGMap *map, int dx, int dy);

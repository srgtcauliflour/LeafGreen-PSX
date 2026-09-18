#include <assert.h>
#include "lg/overworld.h"
int main(void){
 LGMapCell c[9]={0}; c[5].collision=1; LGMap m={3,3,c}; LGPlayer p={1,1,0,0};
 lg_player_step(&p,&m,1,0); assert(p.x==1&&p.y==1);
 assert(p.moving==0); /* blocked: turned to face it, no slide to animate */
 lg_player_step(&p,&m,0,1); assert(p.x==1&&p.y==2);
 assert(p.moving==LG_PLAYER_SLIDE_FRAMES); /* moved: a renderer can animate this */
 lg_player_animate_tick(&p); assert(p.moving==LG_PLAYER_SLIDE_FRAMES-1);
 for (int i=0;i<LG_PLAYER_SLIDE_FRAMES;i++) lg_player_animate_tick(&p);
 assert(p.moving==0); /* never underflows past 0 */
 lg_player_animate_tick(0); /* no-op, doesn't crash */
 assert(!lg_map_can_enter(&m,-1,0));

 c[7].warp=1;
 LGWarp warps[]={{1,42,3,4},{2,0,0,0}};
 const LGWarp *w=lg_map_warp_at(&m,1,2,warps,2);
 assert(w && w->dest_map==42 && w->dest_x==3 && w->dest_y==4);
 assert(!lg_map_warp_at(&m,0,0,warps,2)); /* cell has no warp */
 assert(!lg_map_warp_at(&m,-1,2,warps,2)); /* out of bounds */
 c[8].warp=9; /* id with no matching table entry */
 assert(!lg_map_warp_at(&m,2,2,warps,2));
 assert(!lg_map_warp_at(&m,1,2,0,0)); /* no table at all */

 /* Player at (1,1); an NPC sits one tile below at (1,2). */
 LGPlayer q={1,1,0,0};
 LGObjectEvent events[]={{1,2,5},{2,2,9}};
 q.facing=2; /* down */
 const LGObjectEvent *e=lg_object_event_facing(&q,events,2);
 assert(e && e->script_id==5);
 q.facing=0; /* up: nothing at (1,0) */
 assert(!lg_object_event_facing(&q,events,2));
 q.facing=1; /* right: (2,1) is empty, the NPC is at (2,2) */
 assert(!lg_object_event_facing(&q,events,2));
 q.facing=(int8_t)99; /* unrecognised facing value */
 assert(!lg_object_event_facing(&q,events,2));
 assert(!lg_object_event_facing(0,events,2));
 assert(!lg_object_event_facing(&q,0,2));

 /* Elevation gating: nonzero elevations must match exactly to move
    between them; elevation 0 on either side is a wildcard that always
    connects. Grid indices are y*width+x on a 3x3 map. */
 LGMapCell ec[9]={0};
 ec[4].elevation=1; /* (1,1): elevation 1 */
 ec[5].elevation=2; /* (2,1): elevation 2, mismatched neighbor */
 ec[7].elevation=1; /* (1,2): elevation 1, matching neighbor */
 LGMap em={3,3,ec};
 assert(lg_map_can_enter_from(&em,1,1,2,1)==0); /* 1 -> 2: mismatched */
 assert(lg_map_can_enter_from(&em,1,1,1,2)==1); /* 1 -> 1: matches */
 assert(lg_map_can_enter_from(&em,1,1,0,1)==1); /* 1 -> 0: wildcard dest */
 assert(lg_map_can_enter_from(&em,0,0,1,1)==1); /* 0 -> 1: wildcard source */
 assert(lg_map_can_enter_from(&em,-1,-1,1,1)==1); /* no source context: only collision checked */
 assert(lg_map_can_enter_from(&em,1,1,-1,0)==0); /* destination out of bounds */
 ec[5].collision=1; /* (2,1) also blocked -- elevation match can't override collision */
 ec[5].elevation=1;
 assert(lg_map_can_enter_from(&em,1,1,2,1)==0);

 /* lg_player_step() itself refuses an elevation-incompatible step, same
    as a collision-blocked one: it turns to face it without moving. */
 LGMapCell sc[9]={0};
 sc[4].elevation=1; /* (1,1) */
 sc[5].elevation=2; /* (2,1): mismatched */
 LGMap sm={3,3,sc};
 LGPlayer r={1,1,0,0};
 lg_player_step(&r,&sm,1,0);
 assert(r.x==1 && r.y==1 && r.facing==1 && r.moving==0);

 return 0;
}

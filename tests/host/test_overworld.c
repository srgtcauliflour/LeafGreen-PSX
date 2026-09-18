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

 /* Ledges: a nonzero LGMapCell.ledge only lets a step through when moving
    in its one required direction (ledge - 1, matching LGPlayer.facing
    values), jumping straight over the ledge tile onto the cell beyond
    it. 4x1 row: player at x=0, ledge at x=1 (jumpable only moving right),
    landing at x=2. */
 LGMapCell lc[4]={0};
 lc[1].ledge=2; /* facing 1 (right) + 1 */
 LGMap lm={4,1,lc};
 LGPlayer lp={0,0,0,0};
 lg_player_step(&lp,&lm,1,0); /* step right onto the ledge tile */
 assert(lp.x==2 && lp.y==0 && lp.facing==1);
 assert(lp.moving==(uint8_t)(LG_PLAYER_SLIDE_FRAMES*2)); /* double duration: two cells */

 /* Approaching the same ledge from the wrong direction is blocked, same
    as a collision tile -- it only turns to face it. */
 LGPlayer lp2={2,0,0,0};
 lg_player_step(&lp2,&lm,-1,0); /* step left back onto the ledge tile */
 assert(lp2.x==2 && lp2.y==0 && lp2.facing==3 && lp2.moving==0);

 /* A blocked landing cell refuses the whole jump, not a partial one. */
 LGMapCell lc2[4]={0};
 lc2[1].ledge=2;
 lc2[2].collision=1; /* landing cell blocked */
 LGMap lm2={4,1,lc2};
 LGPlayer lp3={0,0,0,0};
 lg_player_step(&lp3,&lm2,1,0);
 assert(lp3.x==0 && lp3.y==0 && lp3.facing==1 && lp3.moving==0);

 /* lg_map_can_enter_from() itself reflects the same rule directly. */
 assert(lg_map_can_enter_from(&lm,0,0,1,0)==1); /* rightward: matches ledge */
 assert(lg_map_can_enter_from(&lm,2,0,1,0)==0); /* leftward: wrong direction */

 return 0;
}

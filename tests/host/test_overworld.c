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

 return 0;
}

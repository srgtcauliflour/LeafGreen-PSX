#include <assert.h>
#include "lg/overworld.h"
int main(void){
 LGMapCell c[9]={0}; c[5].collision=1; LGMap m={3,3,c}; LGPlayer p={1,1,0,0};
 lg_player_step(&p,&m,1,0); assert(p.x==1&&p.y==1);
 lg_player_step(&p,&m,0,1); assert(p.x==1&&p.y==2);
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
 return 0;
}

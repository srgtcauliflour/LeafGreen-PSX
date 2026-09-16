#include <assert.h>
#include "lg/overworld.h"
int main(void){
 LGMapCell c[9]={0}; c[5].collision=1; LGMap m={3,3,c}; LGPlayer p={1,1,0,0};
 lg_player_step(&p,&m,1,0); assert(p.x==1&&p.y==1);
 lg_player_step(&p,&m,0,1); assert(p.x==1&&p.y==2);
 assert(!lg_map_can_enter(&m,-1,0)); return 0;
}

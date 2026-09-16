#include "lg/overworld.h"
int lg_map_can_enter(const LGMap *m,int x,int y){
 if(!m||x<0||y<0||x>=m->width||y>=m->height) return 0;
 return m->cells[y*m->width+x].collision==0;
}
void lg_player_step(LGPlayer *p,const LGMap *m,int dx,int dy){
 if(!p||!m) return;
 if(dx>1||dx<-1||dy>1||dy<-1||(dx&&dy)) return;
 if(dx>0)p->facing=1; else if(dx<0)p->facing=3; else if(dy>0)p->facing=2; else if(dy<0)p->facing=0;
 int nx=p->x+dx, ny=p->y+dy;
 if(lg_map_can_enter(m,nx,ny)){p->x=nx;p->y=ny;}
}

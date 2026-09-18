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
const LGWarp *lg_map_warp_at(const LGMap *m,int x,int y,const LGWarp *warps,size_t count){
 if(!m||x<0||y<0||x>=m->width||y>=m->height) return 0;
 uint8_t id=m->cells[y*m->width+x].warp;
 if(id==0||!warps) return 0;
 for(size_t i=0;i<count;i++) if(warps[i].id==id) return &warps[i];
 return 0;
}
const LGObjectEvent *lg_object_event_facing(const LGPlayer *p,const LGObjectEvent *events,size_t count){
 if(!p||!events) return 0;
 int dx=0,dy=0;
 switch(p->facing){
 case 0:dy=-1;break;
 case 1:dx=1;break;
 case 2:dy=1;break;
 case 3:dx=-1;break;
 default:return 0;
 }
 int16_t tx=(int16_t)(p->x+dx), ty=(int16_t)(p->y+dy);
 for(size_t i=0;i<count;i++) if(events[i].x==tx&&events[i].y==ty) return &events[i];
 return 0;
}

#include "lg/script.h"
enum { OP_END=0x00, OP_SET=0x01, OP_ADD=0x02, OP_WAIT=0x03 };
void lg_script_init(LGScriptVM *v,const uint8_t*c,size_t n){if(!v)return;v->code=c;v->size=n;v->pc=0;for(int i=0;i<32;i++)v->vars[i]=0;v->status=LG_SCRIPT_RUNNING;}
LGScriptStatus lg_script_step(LGScriptVM *v){
 if(!v||v->status==LG_SCRIPT_DONE||v->status==LG_SCRIPT_ERROR)return v?v->status:LG_SCRIPT_ERROR;
 if(v->status==LG_SCRIPT_WAITING){v->status=LG_SCRIPT_RUNNING;return v->status;}
 if(!v->code||v->pc>=v->size){v->status=LG_SCRIPT_ERROR;return v->status;}
 uint8_t op=v->code[v->pc++];
 switch(op){
 case OP_END:v->status=LG_SCRIPT_DONE;break;
 case OP_WAIT:v->status=LG_SCRIPT_WAITING;break;
 case OP_SET:case OP_ADD:{if(v->size-v->pc<3){v->status=LG_SCRIPT_ERROR;break;}uint8_t i=v->code[v->pc++];uint16_t x=v->code[v->pc];x|=(uint16_t)((uint16_t)v->code[v->pc+1]<<8);v->pc+=2;if(i>=32){v->status=LG_SCRIPT_ERROR;break;}if(op==OP_SET)v->vars[i]=x;else v->vars[i]+=x;break;}
 default:v->status=LG_SCRIPT_ERROR;break;
 }return v->status;
}

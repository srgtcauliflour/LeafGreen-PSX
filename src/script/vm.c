#include "lg/script.h"
/* OP_FLAG_SET: id(1), value(1, must be 0 or 1) -- sets/clears flag bit id.
   OP_JUMP_IF_FLAG: id(1), value(1, must be 0 or 1), addr_lo(1), addr_hi(1)
   -- jumps to the little-endian absolute address (must be < size) when
   flag id currently equals value, otherwise falls through. Both are pure
   VM state, unlike the callback-driven service opcodes below. */
enum { OP_END=0x00, OP_SET=0x01, OP_ADD=0x02, OP_WAIT=0x03, OP_TEXT=0x04,
       OP_MOVE=0x05, OP_WARP=0x06, OP_FLAG_SET=0x07, OP_JUMP_IF_FLAG=0x08,
       OP_ITEM=0x09 };
static bool flag_get(const LGScriptVM *v,uint8_t id){return (v->flags[id/8]>>(id%8))&1;}
static void flag_put(LGScriptVM *v,uint8_t id,uint8_t value){if(value)v->flags[id/8]|=(uint8_t)(1u<<(id%8));else v->flags[id/8]&=(uint8_t)~(1u<<(id%8));}
void lg_script_init(LGScriptVM *v,const uint8_t*c,size_t n){if(!v)return;v->code=c;v->size=n;v->pc=0;for(int i=0;i<32;i++)v->vars[i]=0;for(int i=0;i<32;i++)v->flags[i]=0;v->status=LG_SCRIPT_RUNNING;v->text_fn=0;v->text_context=0;v->move_fn=0;v->move_context=0;v->warp_fn=0;v->warp_context=0;v->item_fn=0;v->item_context=0;}
void lg_script_set_text_fn(LGScriptVM *v,LGScriptTextFn fn,void *context){if(!v)return;v->text_fn=fn;v->text_context=context;}
void lg_script_set_move_fn(LGScriptVM *v,LGScriptMoveFn fn,void *context){if(!v)return;v->move_fn=fn;v->move_context=context;}
void lg_script_set_warp_fn(LGScriptVM *v,LGScriptWarpFn fn,void *context){if(!v)return;v->warp_fn=fn;v->warp_context=context;}
void lg_script_set_item_fn(LGScriptVM *v,LGScriptItemFn fn,void *context){if(!v)return;v->item_fn=fn;v->item_context=context;}
LGScriptStatus lg_script_step(LGScriptVM *v){
 if(!v||v->status==LG_SCRIPT_DONE||v->status==LG_SCRIPT_ERROR||v->status==LG_SCRIPT_BLOCKED)return v?v->status:LG_SCRIPT_ERROR;
 if(v->status==LG_SCRIPT_WAITING){v->status=LG_SCRIPT_RUNNING;return v->status;}
 if(!v->code||v->pc>=v->size){v->status=LG_SCRIPT_ERROR;return v->status;}
 uint8_t op=v->code[v->pc++];
 switch(op){
 case OP_END:v->status=LG_SCRIPT_DONE;break;
 case OP_WAIT:v->status=LG_SCRIPT_WAITING;break;
 case OP_TEXT:{if(v->pc>=v->size){v->status=LG_SCRIPT_ERROR;break;}uint8_t id=v->code[v->pc++];if(!v->text_fn||!v->text_fn(v->text_context,id)){v->status=LG_SCRIPT_ERROR;break;}v->status=LG_SCRIPT_BLOCKED;break;}
 case OP_MOVE:{if(v->size-v->pc<2){v->status=LG_SCRIPT_ERROR;break;}int8_t dx=(int8_t)v->code[v->pc++];int8_t dy=(int8_t)v->code[v->pc++];if(dx<-1||dx>1||dy<-1||dy>1||(dx&&dy)){v->status=LG_SCRIPT_ERROR;break;}if(!v->move_fn||!v->move_fn(v->move_context,dx,dy)){v->status=LG_SCRIPT_ERROR;break;}v->status=LG_SCRIPT_BLOCKED;break;}
 case OP_WARP:{if(v->pc>=v->size){v->status=LG_SCRIPT_ERROR;break;}uint8_t id=v->code[v->pc++];if(!v->warp_fn||!v->warp_fn(v->warp_context,id)){v->status=LG_SCRIPT_ERROR;break;}v->status=LG_SCRIPT_BLOCKED;break;}
 case OP_ITEM:{if(v->size-v->pc<3){v->status=LG_SCRIPT_ERROR;break;}uint8_t id=v->code[v->pc++];uint16_t qty=v->code[v->pc];qty|=(uint16_t)((uint16_t)v->code[v->pc+1]<<8);v->pc+=2;if(!v->item_fn||!v->item_fn(v->item_context,id,qty)){v->status=LG_SCRIPT_ERROR;break;}v->status=LG_SCRIPT_BLOCKED;break;}
 case OP_FLAG_SET:{if(v->size-v->pc<2){v->status=LG_SCRIPT_ERROR;break;}uint8_t id=v->code[v->pc++];uint8_t value=v->code[v->pc++];if(value>1){v->status=LG_SCRIPT_ERROR;break;}flag_put(v,id,value);break;}
 case OP_JUMP_IF_FLAG:{if(v->size-v->pc<4){v->status=LG_SCRIPT_ERROR;break;}uint8_t id=v->code[v->pc++];uint8_t value=v->code[v->pc++];uint16_t addr=v->code[v->pc];addr|=(uint16_t)((uint16_t)v->code[v->pc+1]<<8);v->pc+=2;if(value>1||addr>=v->size){v->status=LG_SCRIPT_ERROR;break;}if(flag_get(v,id)==value)v->pc=addr;break;}
 case OP_SET:case OP_ADD:{if(v->size-v->pc<3){v->status=LG_SCRIPT_ERROR;break;}uint8_t i=v->code[v->pc++];uint16_t x=v->code[v->pc];x|=(uint16_t)((uint16_t)v->code[v->pc+1]<<8);v->pc+=2;if(i>=32){v->status=LG_SCRIPT_ERROR;break;}if(op==OP_SET)v->vars[i]=x;else v->vars[i]+=x;break;}
 default:v->status=LG_SCRIPT_ERROR;break;
 }return v->status;
}
void lg_script_unblock(LGScriptVM *v){if(!v||v->status!=LG_SCRIPT_BLOCKED)return;v->status=LG_SCRIPT_RUNNING;}

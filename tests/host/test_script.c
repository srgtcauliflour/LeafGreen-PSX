#include <assert.h>
#include "lg/script.h"
int main(void){const unsigned char code[]={1,2,5,0,2,2,3,0,3,0};LGScriptVM v;lg_script_init(&v,code,sizeof code);assert(lg_script_step(&v)==LG_SCRIPT_RUNNING);assert(v.vars[2]==5);assert(lg_script_step(&v)==LG_SCRIPT_RUNNING);assert(v.vars[2]==8);assert(lg_script_step(&v)==LG_SCRIPT_WAITING);assert(lg_script_step(&v)==LG_SCRIPT_RUNNING);assert(lg_script_step(&v)==LG_SCRIPT_DONE);return 0;}

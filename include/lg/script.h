#pragma once
#include <stddef.h>
#include <stdint.h>
typedef enum { LG_SCRIPT_RUNNING=0, LG_SCRIPT_WAITING, LG_SCRIPT_DONE, LG_SCRIPT_ERROR } LGScriptStatus;
typedef struct { const uint8_t *code; size_t size, pc; uint16_t vars[32]; LGScriptStatus status; } LGScriptVM;
void lg_script_init(LGScriptVM *vm,const uint8_t *code,size_t size);
LGScriptStatus lg_script_step(LGScriptVM *vm);

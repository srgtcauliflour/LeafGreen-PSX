#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
typedef enum { LG_SCRIPT_RUNNING=0, LG_SCRIPT_WAITING, LG_SCRIPT_BLOCKED, LG_SCRIPT_DONE, LG_SCRIPT_ERROR } LGScriptStatus;
/* Called for OP_TEXT with the raw 1-byte operand as a portable text id (not
   a LeafGreen text index). Returns false to reject an id the game service
   doesn't recognise, which the VM treats as a script error. On true, the VM
   blocks until the caller reports the text/dialogue finished via
   lg_script_unblock(); unlike OP_WAIT this never resumes on its own,
   because the game service's own timing decides when it's done, not one
   fixed VM step. */
typedef bool (*LGScriptTextFn)(void *context, uint8_t text_id);
/* Called for OP_MOVE with a single grid step (dx, dy each in {-1,0,1}, not
   both nonzero -- the same shape lg_player_step() accepts). Returns false
   to reject a step the game service can't take (e.g. blocked/out of
   bounds), which the VM treats as a script error. On true, the VM blocks
   the same way OP_TEXT does: the caller calls lg_script_unblock() once the
   movement (e.g. a multi-frame slide into the next tile) has finished. */
typedef bool (*LGScriptMoveFn)(void *context, int8_t dx, int8_t dy);
typedef struct {
    const uint8_t *code; size_t size, pc; uint16_t vars[32];
    LGScriptStatus status;
    LGScriptTextFn text_fn; void *text_context;
    LGScriptMoveFn move_fn; void *move_context;
} LGScriptVM;
void lg_script_init(LGScriptVM *vm,const uint8_t *code,size_t size);
/* Registers the text/dialogue service callback; pass fn=0 to leave OP_TEXT
   unsupported (it errors instead of blocking forever with nothing able to
   resolve it). Never wires PS1 GPU/controller/CD calls in here directly:
   fn belongs to the platform-neutral game-service layer. */
void lg_script_set_text_fn(LGScriptVM *vm, LGScriptTextFn fn, void *context);
/* Registers the movement service callback; pass fn=0 to leave OP_MOVE
   unsupported, for the same reason as the text callback above. */
void lg_script_set_move_fn(LGScriptVM *vm, LGScriptMoveFn fn, void *context);
LGScriptStatus lg_script_step(LGScriptVM *vm);
/* Resolves a pending LG_SCRIPT_BLOCKED wait (e.g. the dialogue/window the
   last OP_TEXT opened has finished). No-op unless the VM is blocked. */
void lg_script_unblock(LGScriptVM *vm);

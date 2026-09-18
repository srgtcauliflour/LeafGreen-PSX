#include <assert.h>
#include "lg/script.h"

typedef struct { uint8_t last_id; unsigned calls; } TextService;
static bool accept_under_9(void *context, uint8_t id) {
    TextService *s = context;
    s->last_id = id;
    ++s->calls;
    return id < 9;
}

typedef struct { int8_t last_dx, last_dy; unsigned calls; bool accept; } MoveService;
static bool move_service(void *context, int8_t dx, int8_t dy) {
    MoveService *s = context;
    s->last_dx = dx;
    s->last_dy = dy;
    ++s->calls;
    return s->accept;
}

typedef struct { uint8_t last_id; unsigned calls; bool accept; } WarpService;
static bool warp_service(void *context, uint8_t id) {
    WarpService *s = context;
    s->last_id = id;
    ++s->calls;
    return s->accept;
}

typedef struct { uint8_t last_id; uint16_t last_qty; unsigned calls; bool accept; } ItemService;
static bool item_service(void *context, uint8_t id, uint16_t qty) {
    ItemService *s = context;
    s->last_id = id;
    s->last_qty = qty;
    ++s->calls;
    return s->accept;
}

typedef struct { uint8_t last_id, last_var; unsigned calls; bool accept; } ChoiceService;
static bool choice_service(void *context, uint8_t id, uint8_t var_index) {
    ChoiceService *s = context;
    s->last_id = id;
    s->last_var = var_index;
    ++s->calls;
    return s->accept;
}

int main(void) {
    const uint8_t code[] = {1,2,5,0,2,2,3,0,3,0};
    const uint8_t wide[] = {1,31,0x34,0x12,2,31,0xff,0xff,0};
    const uint8_t invalid[] = {1,32,0,0};
    LGScriptVM v;
    lg_script_init(&v, code, sizeof code);
    assert(lg_script_step(&v) == LG_SCRIPT_RUNNING);
    assert(v.vars[2] == 5);
    assert(lg_script_step(&v) == LG_SCRIPT_RUNNING);
    assert(v.vars[2] == 8);
    assert(lg_script_step(&v) == LG_SCRIPT_WAITING);
    assert(lg_script_step(&v) == LG_SCRIPT_RUNNING);
    assert(lg_script_step(&v) == LG_SCRIPT_DONE);
    lg_script_init(&v, wide, sizeof wide);
    assert(lg_script_step(&v) == LG_SCRIPT_RUNNING);
    assert(v.vars[31] == 0x1234);
    assert(lg_script_step(&v) == LG_SCRIPT_RUNNING);
    assert(v.vars[31] == 0x1233);
    for (size_t length = 1; length < 4; ++length) {
        lg_script_init(&v, wide, length);
        assert(lg_script_step(&v) == LG_SCRIPT_ERROR);
        assert(v.vars[31] == 0);
    }
    lg_script_init(&v, invalid, sizeof invalid);
    assert(lg_script_step(&v) == LG_SCRIPT_ERROR);

    /* OP_TEXT blocks on an accepted id until lg_script_unblock() is
       called; unlike OP_WAIT it never resumes on its own. */
    const uint8_t text[] = {4,7,4,9,0};
    TextService svc = {0, 0};
    lg_script_init(&v, text, sizeof text);
    lg_script_set_text_fn(&v, accept_under_9, &svc);
    assert(lg_script_step(&v) == LG_SCRIPT_BLOCKED);
    assert(svc.last_id == 7 && svc.calls == 1);
    /* Stepping while blocked is a no-op, unlike WAITING. */
    assert(lg_script_step(&v) == LG_SCRIPT_BLOCKED);
    assert(svc.calls == 1);
    lg_script_unblock(&v);
    assert(lg_script_step(&v) == LG_SCRIPT_ERROR); /* id 9 rejected */
    assert(svc.calls == 2);

    /* Missing operand and a missing/rejecting callback are both errors. */
    const uint8_t truncated_text[] = {4};
    lg_script_init(&v, truncated_text, sizeof truncated_text);
    lg_script_set_text_fn(&v, accept_under_9, &svc);
    assert(lg_script_step(&v) == LG_SCRIPT_ERROR);
    lg_script_init(&v, text, sizeof text);
    assert(lg_script_step(&v) == LG_SCRIPT_ERROR); /* no text_fn registered */

    /* OP_MOVE blocks the same way, using the same dx/dy shape as
       lg_player_step(): each in {-1,0,1}, never both nonzero. */
    const uint8_t move[] = {5,(uint8_t)-1,0,0};
    MoveService msvc = {0, 0, 0, true};
    lg_script_init(&v, move, sizeof move);
    lg_script_set_move_fn(&v, move_service, &msvc);
    assert(lg_script_step(&v) == LG_SCRIPT_BLOCKED);
    assert(msvc.last_dx == -1 && msvc.last_dy == 0 && msvc.calls == 1);
    assert(lg_script_step(&v) == LG_SCRIPT_BLOCKED); /* no-op while blocked */
    assert(msvc.calls == 1);
    lg_script_unblock(&v);
    assert(lg_script_step(&v) == LG_SCRIPT_DONE);

    /* A rejecting callback, a missing callback, an invalid dx/dy shape and
       a truncated operand are all errors. */
    msvc.accept = false;
    lg_script_init(&v, move, sizeof move);
    lg_script_set_move_fn(&v, move_service, &msvc);
    assert(lg_script_step(&v) == LG_SCRIPT_ERROR);
    lg_script_init(&v, move, sizeof move);
    assert(lg_script_step(&v) == LG_SCRIPT_ERROR); /* no move_fn registered */
    const uint8_t diagonal[] = {5,1,1};
    lg_script_init(&v, diagonal, sizeof diagonal);
    lg_script_set_move_fn(&v, move_service, &msvc);
    assert(lg_script_step(&v) == LG_SCRIPT_ERROR);
    const uint8_t truncated_move[] = {5,1};
    lg_script_init(&v, truncated_move, sizeof truncated_move);
    lg_script_set_move_fn(&v, move_service, &msvc);
    assert(lg_script_step(&v) == LG_SCRIPT_ERROR);

    /* OP_WARP blocks the same way as OP_TEXT/OP_MOVE. */
    const uint8_t warp[] = {6,3,0};
    WarpService wsvc = {0, 0, true};
    lg_script_init(&v, warp, sizeof warp);
    lg_script_set_warp_fn(&v, warp_service, &wsvc);
    assert(lg_script_step(&v) == LG_SCRIPT_BLOCKED);
    assert(wsvc.last_id == 3 && wsvc.calls == 1);
    lg_script_unblock(&v);
    assert(lg_script_step(&v) == LG_SCRIPT_DONE);
    wsvc.accept = false;
    lg_script_init(&v, warp, sizeof warp);
    lg_script_set_warp_fn(&v, warp_service, &wsvc);
    assert(lg_script_step(&v) == LG_SCRIPT_ERROR);
    lg_script_init(&v, warp, sizeof warp);
    assert(lg_script_step(&v) == LG_SCRIPT_ERROR); /* no warp_fn registered */
    const uint8_t truncated_warp[] = {6};
    lg_script_init(&v, truncated_warp, sizeof truncated_warp);
    lg_script_set_warp_fn(&v, warp_service, &wsvc);
    assert(lg_script_step(&v) == LG_SCRIPT_ERROR);

    /* OP_ITEM blocks the same way as OP_TEXT/OP_MOVE/OP_WARP, passing
       through a 16-bit little-endian quantity. */
    const uint8_t item[] = {9,4,0x34,0x12,0}; /* item 4, qty 0x1234 */
    ItemService isvc = {0, 0, 0, true};
    lg_script_init(&v, item, sizeof item);
    lg_script_set_item_fn(&v, item_service, &isvc);
    assert(lg_script_step(&v) == LG_SCRIPT_BLOCKED);
    assert(isvc.last_id == 4 && isvc.last_qty == 0x1234 && isvc.calls == 1);
    assert(lg_script_step(&v) == LG_SCRIPT_BLOCKED); /* no-op while blocked */
    assert(isvc.calls == 1);
    lg_script_unblock(&v);
    assert(lg_script_step(&v) == LG_SCRIPT_DONE);
    isvc.accept = false;
    lg_script_init(&v, item, sizeof item);
    lg_script_set_item_fn(&v, item_service, &isvc);
    assert(lg_script_step(&v) == LG_SCRIPT_ERROR);
    lg_script_init(&v, item, sizeof item);
    assert(lg_script_step(&v) == LG_SCRIPT_ERROR); /* no item_fn registered */
    const uint8_t truncated_item[] = {9,4,0};
    lg_script_init(&v, truncated_item, sizeof truncated_item);
    lg_script_set_item_fn(&v, item_service, &isvc);
    assert(lg_script_step(&v) == LG_SCRIPT_ERROR);

    /* OP_ITEM_TAKE blocks the same way, using a separate callback/context
       from OP_ITEM even though the signature matches. */
    const uint8_t item_take[] = {10,4,0x34,0x12,0}; /* item 4, qty 0x1234 */
    ItemService tsvc = {0, 0, 0, true};
    lg_script_init(&v, item_take, sizeof item_take);
    lg_script_set_item_take_fn(&v, item_service, &tsvc);
    assert(lg_script_step(&v) == LG_SCRIPT_BLOCKED);
    assert(tsvc.last_id == 4 && tsvc.last_qty == 0x1234 && tsvc.calls == 1);
    assert(lg_script_step(&v) == LG_SCRIPT_BLOCKED); /* no-op while blocked */
    assert(tsvc.calls == 1);
    lg_script_unblock(&v);
    assert(lg_script_step(&v) == LG_SCRIPT_DONE);
    tsvc.accept = false;
    lg_script_init(&v, item_take, sizeof item_take);
    lg_script_set_item_take_fn(&v, item_service, &tsvc);
    assert(lg_script_step(&v) == LG_SCRIPT_ERROR);
    lg_script_init(&v, item_take, sizeof item_take);
    assert(lg_script_step(&v) == LG_SCRIPT_ERROR); /* no item_take_fn registered */
    const uint8_t truncated_item_take[] = {10,4,0};
    lg_script_init(&v, truncated_item_take, sizeof truncated_item_take);
    lg_script_set_item_take_fn(&v, item_service, &tsvc);
    assert(lg_script_step(&v) == LG_SCRIPT_ERROR);
    /* Registering item_fn alone does not satisfy OP_ITEM_TAKE. */
    lg_script_init(&v, item_take, sizeof item_take);
    lg_script_set_item_fn(&v, item_service, &isvc);
    assert(lg_script_step(&v) == LG_SCRIPT_ERROR);

    /* OP_CHOICE blocks the same way, passing a choice-prompt id and a
       var index (already bounds-checked by the VM) the caller will
       write the selection into before unblocking. */
    const uint8_t choice[] = {11,2,5,0}; /* OP_CHOICE id=2 var=5 */
    ChoiceService csvc = {0, 0, 0, true};
    lg_script_init(&v, choice, sizeof choice);
    lg_script_set_choice_fn(&v, choice_service, &csvc);
    assert(lg_script_step(&v) == LG_SCRIPT_BLOCKED);
    assert(csvc.last_id == 2 && csvc.last_var == 5 && csvc.calls == 1);
    assert(lg_script_step(&v) == LG_SCRIPT_BLOCKED); /* no-op while blocked */
    assert(csvc.calls == 1);
    v.vars[5] = 1; /* the caller writes the selection before unblocking */
    lg_script_unblock(&v);
    assert(lg_script_step(&v) == LG_SCRIPT_DONE);
    assert(v.vars[5] == 1);
    csvc.accept = false;
    lg_script_init(&v, choice, sizeof choice);
    lg_script_set_choice_fn(&v, choice_service, &csvc);
    assert(lg_script_step(&v) == LG_SCRIPT_ERROR);
    lg_script_init(&v, choice, sizeof choice);
    assert(lg_script_step(&v) == LG_SCRIPT_ERROR); /* no choice_fn registered */
    const uint8_t truncated_choice[] = {11,2};
    lg_script_init(&v, truncated_choice, sizeof truncated_choice);
    lg_script_set_choice_fn(&v, choice_service, &csvc);
    assert(lg_script_step(&v) == LG_SCRIPT_ERROR);
    const uint8_t bad_choice_var[] = {11,2,32,0}; /* var index 32 is out of range */
    lg_script_init(&v, bad_choice_var, sizeof bad_choice_var);
    csvc.accept = true;
    lg_script_set_choice_fn(&v, choice_service, &csvc);
    unsigned calls_before = csvc.calls;
    assert(lg_script_step(&v) == LG_SCRIPT_ERROR);
    assert(csvc.calls == calls_before); /* rejected before the callback ran */

    /* OP_FLAG_SET sets/clears a bit; OP_JUMP_IF_FLAG jumps only when the
       flag currently equals the operand, and only moves pc -- the target
       instruction executes on the following step, not the same one. */
    const uint8_t flagset[] = {7,5,1, 8,5,1,8,0, 0}; /* set flag5=1, jump-if-1 to pc 8 (OP_END) */
    lg_script_init(&v, flagset, sizeof flagset);
    assert(lg_script_step(&v) == LG_SCRIPT_RUNNING); /* OP_FLAG_SET 5=1 */
    assert(lg_script_step(&v) == LG_SCRIPT_RUNNING); /* jump taken, pc moves to 8 */
    assert(v.pc == 8);
    assert(lg_script_step(&v) == LG_SCRIPT_DONE); /* OP_END at pc 8 */

    const uint8_t nojump[] = {8,5,1,5,0, 0}; /* flag 5 is 0, expects 1: falls through */
    lg_script_init(&v, nojump, sizeof nojump);
    assert(lg_script_step(&v) == LG_SCRIPT_RUNNING); /* no jump taken */
    assert(v.pc == 5);
    assert(lg_script_step(&v) == LG_SCRIPT_DONE);

    const uint8_t bad_value[] = {7,0,2};
    lg_script_init(&v, bad_value, sizeof bad_value);
    assert(lg_script_step(&v) == LG_SCRIPT_ERROR);
    const uint8_t bad_jump_target[] = {8,0,0,99,0};
    lg_script_init(&v, bad_jump_target, sizeof bad_jump_target);
    assert(lg_script_step(&v) == LG_SCRIPT_ERROR); /* target 99 >= size */
    const uint8_t truncated_flag[] = {7,0};
    lg_script_init(&v, truncated_flag, sizeof truncated_flag);
    assert(lg_script_step(&v) == LG_SCRIPT_ERROR);
    const uint8_t truncated_jump[] = {8,0,0,0};
    lg_script_init(&v, truncated_jump, sizeof truncated_jump);
    assert(lg_script_step(&v) == LG_SCRIPT_ERROR);

    /* OP_JUMP_IF_VAR is the same idea as OP_JUMP_IF_FLAG, but compares a
       full 16-bit var value instead of a single flag bit -- this is what
       lets a script branch on OP_CHOICE's result. */
    const uint8_t varset[] = {1,5,1,0, 12,5,1,0,10,0, 0}; /* OP_SET var5=1, jump-if-var5==1 to pc 10 (OP_END) */
    lg_script_init(&v, varset, sizeof varset);
    assert(lg_script_step(&v) == LG_SCRIPT_RUNNING); /* OP_SET var5=1 */
    assert(lg_script_step(&v) == LG_SCRIPT_RUNNING); /* jump taken, pc moves to 10 */
    assert(v.pc == 10);
    assert(lg_script_step(&v) == LG_SCRIPT_DONE); /* OP_END at pc 10 */

    const uint8_t novarjump[] = {12,5,2,0,6,0, 0}; /* var 5 is 0, expects 2: falls through */
    lg_script_init(&v, novarjump, sizeof novarjump);
    assert(lg_script_step(&v) == LG_SCRIPT_RUNNING); /* no jump taken */
    assert(v.pc == 6);
    assert(lg_script_step(&v) == LG_SCRIPT_DONE);

    const uint8_t bad_var_index[] = {12,32,0,0,0,0}; /* var index 32 is out of range */
    lg_script_init(&v, bad_var_index, sizeof bad_var_index);
    assert(lg_script_step(&v) == LG_SCRIPT_ERROR);
    const uint8_t bad_var_jump_target[] = {12,0,0,0,99,0}; /* target 99 >= size */
    lg_script_init(&v, bad_var_jump_target, sizeof bad_var_jump_target);
    assert(lg_script_step(&v) == LG_SCRIPT_ERROR);
    const uint8_t truncated_var_jump[] = {12,0,0,0,0}; /* missing addr_hi */
    lg_script_init(&v, truncated_var_jump, sizeof truncated_var_jump);
    assert(lg_script_step(&v) == LG_SCRIPT_ERROR);

    /* Unblocking anything but a blocked VM is a no-op. */
    lg_script_init(&v, code, sizeof code);
    lg_script_unblock(&v);
    assert(v.status == LG_SCRIPT_RUNNING);
    lg_script_unblock(0);

    return 0;
}

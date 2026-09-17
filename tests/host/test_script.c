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

    /* Unblocking anything but a blocked VM is a no-op. */
    lg_script_init(&v, code, sizeof code);
    lg_script_unblock(&v);
    assert(v.status == LG_SCRIPT_RUNNING);
    lg_script_unblock(0);

    return 0;
}

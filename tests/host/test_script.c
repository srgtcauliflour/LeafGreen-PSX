#include <assert.h>
#include "lg/script.h"

typedef struct { uint8_t last_id; unsigned calls; } TextService;
static bool accept_under_9(void *context, uint8_t id) {
    TextService *s = context;
    s->last_id = id;
    ++s->calls;
    return id < 9;
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

    /* Unblocking anything but a blocked VM is a no-op. */
    lg_script_init(&v, code, sizeof code);
    lg_script_unblock(&v);
    assert(v.status == LG_SCRIPT_RUNNING);
    lg_script_unblock(0);

    return 0;
}

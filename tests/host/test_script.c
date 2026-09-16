#include <assert.h>
#include "lg/script.h"
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
    return 0;
}

#include "lg/interact.h"
#include <assert.h>

static bool lookup_known(void *context, uint8_t script_id, const uint8_t **code, size_t *size) {
    (void)context;
    static const uint8_t script[] = {0}; /* OP_END */
    if (script_id != 5) return false;
    *code = script;
    *size = sizeof script;
    return true;
}

int main(void) {
    LGObjectEvent events[] = {{1, 2, 5}}; /* NPC at (1,2), script id 5 */
    LGPlayer player = {1, 1, 2, 0}; /* facing down (2), one step from (1,2) */
    LGGameService svc;
    LGScriptVM vm;
    LGMapCell cells[9] = {0};
    LGMap map = {3, 3, cells};
    lg_game_service_init(&svc, &player, &map, 0, 0);

    /* No button pressed: nothing happens. */
    LgInputState input = {0, 0, 0};
    assert(lg_overworld_try_interact(&input, &player, events, 1, lookup_known, 0, &vm, &svc)
           == LG_INTERACT_NONE);

    /* A pressed while facing the event with a known script: starts it. */
    input.pressed = LG_BUTTON_A;
    assert(lg_overworld_try_interact(&input, &player, events, 1, lookup_known, 0, &vm, &svc)
           == LG_INTERACT_STARTED);
    assert(vm.status == LG_SCRIPT_RUNNING);
    assert(lg_script_step(&vm) == LG_SCRIPT_DONE); /* the looked-up script really is bound */

    /* A pressed but facing an empty tile: nothing to interact with. */
    LGPlayer elsewhere = {0, 0, 2, 0};
    assert(lg_overworld_try_interact(&input, &elsewhere, events, 1, lookup_known, 0, &vm, &svc)
           == LG_INTERACT_NONE);

    /* A pressed, facing an event, but its script id is unknown to the
       lookup: not an error, just nothing to do. */
    LGObjectEvent decorative[] = {{1, 2, 9}};
    assert(lg_overworld_try_interact(&input, &player, decorative, 1, lookup_known, 0, &vm, &svc)
           == LG_INTERACT_NONE);

    /* Invalid arguments are rejected as errors, not silently ignored. */
    assert(lg_overworld_try_interact(0, &player, events, 1, lookup_known, 0, &vm, &svc)
           == LG_INTERACT_ERROR);
    assert(lg_overworld_try_interact(&input, 0, events, 1, lookup_known, 0, &vm, &svc)
           == LG_INTERACT_ERROR);
    assert(lg_overworld_try_interact(&input, &player, events, 1, 0, 0, &vm, &svc)
           == LG_INTERACT_ERROR);
    assert(lg_overworld_try_interact(&input, &player, events, 1, lookup_known, 0, 0, &svc)
           == LG_INTERACT_ERROR);
    assert(lg_overworld_try_interact(&input, &player, events, 1, lookup_known, 0, &vm, 0)
           == LG_INTERACT_ERROR);

    return 0;
}

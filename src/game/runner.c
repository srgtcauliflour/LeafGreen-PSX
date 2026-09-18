#include "lg/runner.h"

/* A warp resolved inside LGGameService may have switched service->map to
   a different one (see lg_game_service_set_map_table()). If that new map
   has a matching LGMapEntry, follow its events/event_count too, so the
   runner doesn't keep offering to interact with the previous map's NPCs.
   If no entry matches (e.g. no map table was registered, or the new map
   isn't in it), events/event_count are left as they were -- the same
   "leave unresolved rather than guess" choice LGGameService itself makes
   for map/warps in that situation. */
static void sync_events_to_active_map(LGOverworldRunner *r) {
    for (size_t i = 0; i < r->service->map_table_count; ++i) {
        if (r->service->map_table[i].map != r->service->map) continue;
        r->events = r->service->map_table[i].events;
        r->event_count = r->service->map_table[i].event_count;
        return;
    }
}

void lg_overworld_runner_init(LGOverworldRunner *runner, LGPlayer *player,
                               LGGameService *service, LGScriptVM *vm, LGGameLoop *loop,
                               const LGObjectEvent *events, size_t event_count,
                               LGInteractScriptLookupFn script_lookup_fn,
                               void *script_lookup_context) {
    if (!runner) return;
    runner->player = player;
    runner->service = service;
    runner->vm = vm;
    runner->loop = loop;
    runner->events = events;
    runner->event_count = event_count;
    runner->script_lookup_fn = script_lookup_fn;
    runner->script_lookup_context = script_lookup_context;
    runner->mode = LG_RUNNER_MODE_IDLE;
}

LGRunnerStepResult lg_overworld_runner_step(LGOverworldRunner *runner,
                                             const LgInputState *input,
                                             bool advance_pressed) {
    if (!runner || !runner->player || !runner->service || !runner->vm || !runner->loop || !input)
        return LG_RUNNER_STEP_ERROR;

    if (runner->mode == LG_RUNNER_MODE_SCRIPT) {
        LGLoopStatus status = lg_game_loop_step(runner->loop, advance_pressed);
        if (status == LG_LOOP_DONE) {
            runner->mode = LG_RUNNER_MODE_IDLE;
            sync_events_to_active_map(runner);
            return LG_RUNNER_STEP_SCRIPT_DONE;
        }
        if (status == LG_LOOP_ERROR) {
            runner->mode = LG_RUNNER_MODE_IDLE;
            sync_events_to_active_map(runner);
            return LG_RUNNER_STEP_ERROR;
        }
        return LG_RUNNER_STEP_SCRIPT_RUNNING;
    }

    /* Skip the interact check entirely when there are no events to face --
       this is also what lets a runner for an event-free map omit
       script_lookup_fn, since lg_overworld_try_interact() otherwise
       requires it unconditionally. */
    if (runner->event_count > 0) {
        LGInteractResult interact = lg_overworld_try_interact(
            input, runner->player, runner->events, runner->event_count,
            runner->script_lookup_fn, runner->script_lookup_context, runner->vm, runner->service);
        if (interact == LG_INTERACT_ERROR) return LG_RUNNER_STEP_ERROR;
        if (interact == LG_INTERACT_STARTED) {
            runner->mode = LG_RUNNER_MODE_SCRIPT;
            return LG_RUNNER_STEP_SCRIPT_STARTED;
        }
    }

    lg_overworld_input_step(runner->player, runner->service->map, input);
    return LG_RUNNER_STEP_IDLE;
}

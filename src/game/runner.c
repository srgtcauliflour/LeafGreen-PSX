#include "lg/runner.h"

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
            return LG_RUNNER_STEP_SCRIPT_DONE;
        }
        if (status == LG_LOOP_ERROR) {
            runner->mode = LG_RUNNER_MODE_IDLE;
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

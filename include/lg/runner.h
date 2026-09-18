#ifndef LG_RUNNER_H
#define LG_RUNNER_H
#include "lg/input_control.h"
#include "lg/interact.h"
#include "lg/loop.h"

/* Free-roam walking (input_control.h), facing/interacting with an object
   (interact.h) and driving a running script frame by frame
   (loop.h/LGGameService) all exist, but nothing switches between them --
   a real game loop needs exactly one function to call every frame.
   LGOverworldRunner is that: while idle it takes free-roam input and
   checks for an interact; once a script starts, it drives that script
   with LGGameLoop until done, then returns to idle. If the script warped
   to a map registered in the service's LGMapEntry table (see
   lg_game_service_set_map_table()), the runner's own events/event_count
   follow that entry's on the way back to idle, so interacting stays
   correct for whichever map is now active -- not the one the player left. */
typedef enum { LG_RUNNER_MODE_IDLE, LG_RUNNER_MODE_SCRIPT } LGRunnerMode;

typedef enum {
    LG_RUNNER_STEP_IDLE,           /* free-roam frame; input may have moved the player */
    LG_RUNNER_STEP_SCRIPT_STARTED, /* an interact just started a script this frame */
    LG_RUNNER_STEP_SCRIPT_RUNNING, /* the active script advanced (or is waiting) this frame */
    LG_RUNNER_STEP_SCRIPT_DONE,    /* the active script just finished; idle again next frame */
    LG_RUNNER_STEP_ERROR           /* invalid arguments, or the script errored (now back to idle) */
} LGRunnerStepResult;

typedef struct {
    LGPlayer *player;
    LGGameService *service; /* service->map is always the live active map */
    LGScriptVM *vm;
    LGGameLoop *loop; /* caller-initialized (lg_game_loop_init), bound to the same vm/service */
    const LGObjectEvent *events;
    size_t event_count;
    LGInteractScriptLookupFn script_lookup_fn;
    void *script_lookup_context;
    LGRunnerMode mode;
} LGOverworldRunner;

void lg_overworld_runner_init(LGOverworldRunner *runner, LGPlayer *player,
                               LGGameService *service, LGScriptVM *vm, LGGameLoop *loop,
                               const LGObjectEvent *events, size_t event_count,
                               LGInteractScriptLookupFn script_lookup_fn,
                               void *script_lookup_context);
/* Advances by one frame. While idle: tries an interact first (a newly
   pressed A facing a scripted object starts it, LG_RUNNER_STEP_SCRIPT_STARTED),
   otherwise takes free-roam movement input (LG_RUNNER_STEP_IDLE). The
   interact check (and therefore the requirement that script_lookup_fn be
   non-NULL) is skipped entirely when event_count is 0, so a runner for an
   event-free map can leave script_lookup_fn NULL. While a script is
   running: steps it via LGGameLoop, returning to idle on LG_LOOP_DONE
   (LG_RUNNER_STEP_SCRIPT_DONE) or LG_LOOP_ERROR (LG_RUNNER_STEP_ERROR, so
   one broken script doesn't wedge the whole overworld), otherwise
   LG_RUNNER_STEP_SCRIPT_RUNNING. */
LGRunnerStepResult lg_overworld_runner_step(LGOverworldRunner *runner,
                                             const LgInputState *input,
                                             bool advance_pressed);
#endif

#ifndef LG_INTERACT_H
#define LG_INTERACT_H
#include "lg/platform.h"
#include "lg/service.h"

/* Looks up the script bytecode for a portable script id (see
   LGObjectEvent.script_id). Returns false for an id with no known
   script, which lg_overworld_try_interact() then treats as "nothing to
   do" rather than an error -- a decorative object with no script yet is
   not a bug. */
typedef bool (*LGInteractScriptLookupFn)(void *context, uint8_t script_id,
                                          const uint8_t **code, size_t *size);

typedef enum { LG_INTERACT_NONE, LG_INTERACT_STARTED, LG_INTERACT_ERROR } LGInteractResult;

/* The last piece connecting free-roam input to scripted events: on a
   newly pressed LG_BUTTON_A, if the player is facing an LGObjectEvent
   (lg_object_event_facing()) and lookup_fn resolves its script_id to
   bytecode, starts that script on vm (lg_script_init()) and binds
   service to it (lg_game_service_bind()) -- the caller then drives it
   with LGGameLoop from the next frame on, the same as any other script.

   Returns LG_INTERACT_STARTED on that path, LG_INTERACT_NONE if nothing
   happened (no press, no facing event, or lookup_fn found no script for
   it -- none of these are errors), or LG_INTERACT_ERROR only for invalid
   arguments (any of input/player/lookup_fn/vm/service is NULL). Does not
   itself run any of the script -- call lg_game_loop_step() (or
   lg_script_step()) afterward as normal. */
LGInteractResult lg_overworld_try_interact(const LgInputState *input, const LGPlayer *player,
                                            const LGObjectEvent *events, size_t event_count,
                                            LGInteractScriptLookupFn lookup_fn, void *lookup_context,
                                            LGScriptVM *vm, LGGameService *service);
#endif

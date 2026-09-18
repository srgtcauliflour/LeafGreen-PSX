#include "lg/interact.h"

LGInteractResult lg_overworld_try_interact(const LgInputState *input, const LGPlayer *player,
                                            const LGObjectEvent *events, size_t event_count,
                                            LGInteractScriptLookupFn lookup_fn, void *lookup_context,
                                            LGScriptVM *vm, LGGameService *service) {
    if (!input || !player || !lookup_fn || !vm || !service) return LG_INTERACT_ERROR;
    if (!(input->pressed & LG_BUTTON_A)) return LG_INTERACT_NONE;

    const LGObjectEvent *event = lg_object_event_facing(player, events, event_count);
    if (!event) return LG_INTERACT_NONE;

    const uint8_t *code;
    size_t size;
    if (!lookup_fn(lookup_context, event->script_id, &code, &size)) return LG_INTERACT_NONE;

    lg_script_init(vm, code, size);
    lg_game_service_bind(service, vm);
    return LG_INTERACT_STARTED;
}

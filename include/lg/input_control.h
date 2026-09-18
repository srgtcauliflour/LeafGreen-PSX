#ifndef LG_INPUT_CONTROL_H
#define LG_INPUT_CONTROL_H
#include "lg/overworld.h"
#include "lg/platform.h"

/* Bridges LgInputState (platform.h's portable controller abstraction) to
   LGPlayer movement (overworld.h), so free-roam walking has somewhere to
   live: previously only a script's OP_MOVE could move the player at all.
   On a newly pressed direction (input->pressed, not held -- so holding
   the button doesn't repeat a step every single frame with no timing
   model to pace it) it takes exactly one grid step via lg_player_step(),
   turning to face a blocked direction without moving, same as that
   function's own contract. When more than one direction is pressed in
   the same frame, up/down/left/right is the priority order used to pick
   one -- a simple placeholder policy, not verified LeafGreen input
   handling (e.g. no turn-then-walk on first press); real semantics need
   ROM evidence. While LG_BUTTON_B is held (LeafGreen's Running Shoes
   button, not a bike), the step uses lg_player_run_step() instead of
   lg_player_step() -- same grid step, just a faster LGPlayer.moving
   presentation hint; there is still no biking, and no ROM-verified
   running speed backs this number. Returns true if a direction was
   pressed and lg_player_step() was called (whether or not the step was
   actually allowed), false if no directional button was newly pressed or
   any argument is NULL. */
bool lg_overworld_input_step(LGPlayer *player, const LGMap *map,
                              const LgInputState *input);
#endif
